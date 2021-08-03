/*
 **************************************************************************
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <linux/tc_act/tc_nss_mirred.h>
#include <net/netfilter/nf_conntrack_core.h>
#include "nss_mirred.h"
#include "nss_igs.h"
#include "nss_ifb.h"

static LIST_HEAD(nss_mirred_list);		/* List for all nss mirred actions */
static DEFINE_SPINLOCK(nss_mirred_list_lock);	/* Lock for the nss mirred list */

/*
 * nss_mirred_release()
 *	Cleanup the resources for nss mirred action.
 */
static void nss_mirred_release(struct tc_action *tc_act, int bind)
{
	struct nss_mirred_tcf *act = nss_mirred_get(tc_act);
	struct net_device *dev = rcu_dereference_protected(act->tcfm_dev, 1);
	struct nss_ifb_info *ifb_info = nss_ifb_find_dev(dev);

	if (!ifb_info) {
		nss_igs_error("IFB device %s not found in the linked list \n", dev->name);
		return;
	}

	/*
	 * Send IFB RESET NEXTHOP configure message to the mapped interface.
	 */
	if (!nss_ifb_reset_nexthop(ifb_info)) {
		nss_igs_error("Error in sending IFB RESET NEXTHOP configure message\n");
	}

	if (!nss_ifb_clear_igs_node(ifb_info)) {
		nss_igs_error("Error in sending IFB CLEAR configure message\n");
	}

	/*
	 * Delete the nss mirred action list.
	 */
	spin_lock_bh(&nss_mirred_list_lock);
	list_del(&act->tcfm_list);
	spin_unlock_bh(&nss_mirred_list_lock);
	if (dev) {
		dev_put(dev);
	}
}

/*
 * nss_mirred_policy
 *	nss mirred policy structure.
 */
static const struct nla_policy nss_mirred_policy[TC_NSS_MIRRED_MAX + 1] = {
	[TC_NSS_MIRRED_PARMS] = { .len = sizeof(struct tc_nss_mirred) },
};

/*
 * nss_mirred_init()
 *	Initialize the nss mirred action.
 */
static int nss_mirred_init(struct net *net, struct nlattr *nla,
			   struct nlattr *est, struct tc_action *tc_act, int ovr,
			   int bind)
{
	struct nlattr *arr[TC_NSS_MIRRED_MAX + 1];
	struct tc_nss_mirred *parm;
	struct nss_mirred_tcf *act;
	struct net_device *to_dev, *from_dev;
	struct nss_ifb_info *ifb_info;
	int32_t ret, ifb_num;

	if (!nla) {
		return -EINVAL;
	}

	/*
	 * Parse and validate the user configurations.
	 */
	ret = nla_parse_nested(arr, TC_NSS_MIRRED_MAX, nla, nss_mirred_policy);
	if (ret < 0) {
		return ret;
	}
	if (!arr[TC_NSS_MIRRED_PARMS]) {
		return -EINVAL;
	}

	parm = nla_data(arr[TC_NSS_MIRRED_PARMS]);
	if (!parm->from_ifindex || !parm->to_ifindex) {
		nss_igs_error("Invalid ifindex: from_ifindex: %u, to_ifindex: %u\n",
				parm->from_ifindex, parm->to_ifindex);
		return -EINVAL;
	}

	from_dev = __dev_get_by_index(net, parm->from_ifindex);
	if (!from_dev) {
		nss_igs_error("No device found for %u ifindex\n", parm->from_ifindex);
		return -ENODEV;
	}

	if (nss_cmn_get_interface_number_by_dev(from_dev) < 0) {
		nss_igs_error("No NSS FW device found for %s\n", from_dev->name);
		return -ENODEV;
	}

	if (netif_is_ifb_dev(from_dev)) {
		nss_igs_error("IFB device %s as from_dev\n", from_dev->name);
		return -ENODEV;
	}

	to_dev = __dev_get_by_index(net, parm->to_ifindex);
	if (!to_dev) {
		nss_igs_error("No device found for %u ifindex\n", parm->to_ifindex);
		return -ENODEV;
	}

	if (!netif_is_ifb_dev(to_dev)) {
		nss_igs_error("%s is not an IFB device\n", to_dev->name);
		return -ENODEV;
	}

	ifb_info = nss_ifb_find_map_dev(from_dev);
	if (ifb_info) {
		if (nss_ifb_is_mapped(ifb_info)) {
			nss_igs_error("%s device is already mapped to the other IFB device\n",
					from_dev->name);
			return -EEXIST;
		}
	}

	ifb_info = nss_ifb_find_dev(to_dev);
	if (ifb_info) {
		if (nss_ifb_is_mapped(ifb_info)) {
			nss_igs_error("%s IFB device is already mapped to the other device\n",
					to_dev->name);
			return -EEXIST;
		}
	}

	ifb_num = nss_cmn_get_interface_number_by_dev_and_type(to_dev, NSS_DYNAMIC_INTERFACE_TYPE_IGS);
	if (ifb_num < 0) {
		/*
		 * Create the IFB instance in the NSS firmware.
		 */
		ifb_num = nss_ifb_create_if(to_dev);
		if (ifb_num < 0) {
			nss_igs_error("failure in IFB creation\n");
			return -EINVAL;
		}
	}

	/*
	 * Send config message to the interface attached to an IFB interface.
	 */
	if (nss_ifb_config_msg_tx_sync(from_dev, ifb_num, NSS_IFB_SET_IGS_NODE, NULL) < 0) {
		nss_igs_error("Sending config to %s dev failed\n", from_dev->name);
		return -EINVAL;
	}

	/*
	 * Send next hop config message to the interface attached to an IFB interface.
	 */
	if (nss_ifb_config_msg_tx_sync(from_dev, ifb_num, NSS_IFB_SET_NEXTHOP, NULL) < 0) {
		nss_igs_error("Sending next hop config to %s dev failed\n", from_dev->name);
		return -EINVAL;
	}

	/*
	 * Bind an IFB device with its requested mapped interface.
	 */
	ret = nss_ifb_bind(ifb_info, from_dev, to_dev);
	if (ret < 0) {
		nss_igs_error(" Binding an IFB device failed\n");
		nss_ifb_delete_if(ifb_num);
		return ret;
	}

	/*
	 * Return error if nss mirred action index is present in the hash.
	 */
	if (tcf_hash_check(parm->index, tc_act, bind)) {
		return -EEXIST;
	}

	ret = tcf_hash_create(parm->index, est, tc_act, sizeof(*act),
			bind, true);
	if (ret) {
		return ret;
	}

	act = nss_mirred_get(tc_act);

	/*
	 * Fill up the nss mirred tc parameters to
	 * its local action structure.
	 */
	ASSERT_RTNL();
	act->tcf_action = parm->action;
	act->tcfm_from_ifindex = parm->from_ifindex;
	act->tcfm_to_ifindex = parm->to_ifindex;
	dev_hold(to_dev);
	rcu_assign_pointer(act->tcfm_dev, to_dev);

	/*
	 * Add the new action instance to the nss mirred action list.
	 */
	spin_lock_bh(&nss_mirred_list_lock);
	list_add(&act->tcfm_list, &nss_mirred_list);
	spin_unlock_bh(&nss_mirred_list_lock);
	tcf_hash_insert(tc_act);

	return ACT_P_CREATED;
}

/*
 * nss_mirred_act()
 *	nss mirred action handler.
 */
static int nss_mirred_act(struct sk_buff *skb, const struct tc_action *tc_act,
		      struct tcf_result *res)
{
	struct nss_mirred_tcf *act = tc_act->priv;
	struct net_device *dev;
	struct sk_buff *skb_new;
	int retval, err;
	u32 skb_tc_at = G_TC_AT(skb->tc_verd);

	/*
	 * Return if skb is not at ingress.
	 */
	if (!(skb_tc_at & AT_INGRESS)) {
		return TC_ACT_UNSPEC;
	}

	/*
	 * Update the last use of action.
	 */
	tcf_lastuse_update(&act->tcf_tm);
	bstats_cpu_update(this_cpu_ptr(act->common.cpu_bstats), skb);

	rcu_read_lock();
	retval = READ_ONCE(act->tcf_action);
	dev = rcu_dereference(act->tcfm_dev);
	if (unlikely(!dev)) {
		nss_igs_error("tc nssmirred: target device is gone\n");
		goto out;
	}

	if (unlikely(!(dev->flags & IFF_UP))) {
		nss_igs_error("tc nssmirred: device %s is down\n", dev->name);
		goto out;
	}

	/*
	 * Redirect the packet to the attached IFB interface
	 */
	skb_new = skb_clone(skb, GFP_ATOMIC);
	if (!skb_new) {
		goto out;
	}

	skb_new->skb_iif = skb->dev->ifindex;
	skb_new->dev = dev;
	skb_new->tc_verd = SET_TC_FROM(skb_new->tc_verd, skb_tc_at);
	skb_push_rcsum(skb_new, skb->mac_len);
	skb_sender_cpu_clear(skb_new);

	err = dev_queue_xmit(skb_new);
	if (!err) {
		rcu_read_unlock();
		return retval;
	}
out:
	qstats_overlimit_inc(this_cpu_ptr(act->common.cpu_qstats));
	retval = TC_ACT_SHOT;
	rcu_read_unlock();
	return retval;
}

/*
 * nss_mirred_dump()
 *	Dump the nssmirred action configurations.
 */
static int nss_mirred_dump(struct sk_buff *skb, struct tc_action *tc_act, int bind, int ref)
{
	struct tcf_t filter;
	unsigned char *tail = skb_tail_pointer(skb);
	struct nss_mirred_tcf *act = tc_act->priv;
	struct tc_nss_mirred opt = {
		.index   = act->tcf_index,
		.action  = act->tcf_action,
		.refcnt  = act->tcf_refcnt - ref,
		.bindcnt = act->tcf_bindcnt - bind,
		.from_ifindex = act->tcfm_from_ifindex,
		.to_ifindex = act->tcfm_to_ifindex,
	};

	if (nla_put(skb, TC_NSS_MIRRED_PARMS, sizeof(opt), &opt)) {
		goto out;
	}
	filter.install = jiffies_to_clock_t(jiffies - act->tcf_tm.install);
	filter.lastuse = jiffies_to_clock_t(jiffies - act->tcf_tm.lastuse);
	filter.expires = jiffies_to_clock_t(act->tcf_tm.expires);
	if (nla_put(skb, TC_NSS_MIRRED_TM, sizeof(filter), &filter)) {
		goto out;
	}
	return skb->len;

out:
	nlmsg_trim(skb, tail);
	return -1;
}

/*
 * nss_mirred_unregister_event_handler()
 *	nss mirred un-register event handler.
 */
static void nss_mirred_unregister_event_handler(struct net_device *dev)
{
	struct nss_ifb_info *ifb_info;

	/*
	 * IFB interface.
	 */
	if (netif_is_ifb_dev(dev)) {
		ifb_info = nss_ifb_find_dev(dev);
	} else {
		/*
		 * Check if the device is an IFB mapped device.
		 */
		ifb_info = nss_ifb_find_map_dev(dev);
	}

	/*
	 * Device not present in ifb list.
	 */
	if (!ifb_info) {
		return;
	}

	/*
	 * Send IFB RESET NEXTHOP configure message to the mapped interface.
	 */
	if (!nss_ifb_reset_nexthop(ifb_info)) {
		nss_igs_error("Error in sending IFB RESET NEXTHOP configure message\n");
	}

	/*
	 * Send IFB CLEAR configure message to the mapped interface.
	 */
	if (!nss_ifb_clear_igs_node(ifb_info)) {
		nss_igs_error("Error in sending IFB CLEAR configure message\n");
	}
	if (netif_is_ifb_dev(dev)) {
		int32_t ifb_num = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_IGS);

		if (ifb_num < 0) {
			nss_igs_error("Invalid %s IFB device\n", dev->name);
			return;
		}
		nss_ifb_delete_if(ifb_num);
		nss_ifb_list_del(ifb_info);
	}
}

/*
 * nss_mirred_down_event_handler()
 *	nss mirred interface's down event handler.
 */
static void nss_mirred_down_event_handler(struct net_device *dev)
{
	struct nss_ifb_info *ifb_info;

	/*
	 * IFB interface.
	 */
	if (!netif_is_ifb_dev(dev)) {
		return;
	}

	ifb_info = nss_ifb_find_dev(dev);

	if (!ifb_info) {
		return;
	}

	if (!nss_ifb_down(ifb_info)) {
		nss_igs_error("Error in sending IFB DOWN configure message\n");
	}
}

/*
 * nss_mirred_up_event_handler()
 *	nss mirred interface's up event handler.
 */
static void nss_mirred_up_event_handler(struct net_device *dev)
{
	struct nss_ifb_info *ifb_info;

	/*
	 * IFB interface.
	 */
	if (!netif_is_ifb_dev(dev)) {
		return;
	}

	ifb_info = nss_ifb_find_dev(dev);

	if (!ifb_info) {
		return;
	}

	if (!nss_ifb_up(ifb_info)) {
		nss_igs_error("Error in sending IFB UP configure message\n");
	}
}

/*
 * nss_mirred_device_event()
 *	nssmirred device event callback.
 */
static int nss_mirred_device_event(struct notifier_block *unused,
		unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct nss_mirred_tcf *act;

	switch (event) {
	case NETDEV_UNREGISTER:
		nss_mirred_unregister_event_handler(dev);

		ASSERT_RTNL();

		/*
		 * Free up the actions instance present in
		 * the nss mirred list.
		 */
		spin_lock_bh(&nss_mirred_list_lock);
		list_for_each_entry(act, &nss_mirred_list, tcfm_list) {
			if (rcu_access_pointer(act->tcfm_dev) == dev) {
				dev_put(dev);
				RCU_INIT_POINTER(act->tcfm_dev, NULL);
			}
		}
		spin_unlock_bh(&nss_mirred_list_lock);
		break;
	case NETDEV_UP:
		nss_mirred_up_event_handler(dev);
		break;
	case NETDEV_DOWN:
		nss_mirred_down_event_handler(dev);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * nss_mirred_device_notifier
 *	nss mirred device notifier structure.
 */
static struct notifier_block nss_mirred_device_notifier = {
	.notifier_call = nss_mirred_device_event,
};

/*
 * nss_mirred_act_ops
 *	Registration structure for nss mirred action.
 */
struct tc_action_ops nss_mirred_act_ops = {
	.kind		=	"nssmirred",
	.type		=	TCA_ACT_MIRRED_NSS,
	.owner		=	THIS_MODULE,
	.act		=	nss_mirred_act,
	.dump		=	nss_mirred_dump,
	.cleanup	=	nss_mirred_release,
	.init		=	nss_mirred_init,
};

/*
 * nss_mirred_igs_nf_ops
 *	Pre-routing hooks into netfilter packet monitoring point.
 */
struct nf_hook_ops nss_mirred_igs_nf_ops[] __read_mostly = {
	/*
	 * Pre routing hook is used to copy class-id to the ECM rule.
	 */
	{
		.hook		=	nss_ifb_igs_ip_pre_routing_hook,
		.pf		=	NFPROTO_IPV4,
		.hooknum	=	NF_INET_PRE_ROUTING,
		.priority	=	NF_IP_PRI_CONNTRACK + 1,
	},
	{
		.hook		=	nss_ifb_igs_ip_pre_routing_hook,
		.pf		=	NFPROTO_IPV6,
		.hooknum	=	NF_INET_PRE_ROUTING,
		.priority	=	NF_IP_PRI_CONNTRACK + 1,
	},
};

/*
 * nss_mirred_init_module()
 *	nssmirred init function.
 */
static int __init nss_mirred_init_module(void)
{
	int err = register_netdevice_notifier(&nss_mirred_device_notifier);
	if (err) {
		return err;
	}

	err = tcf_register_action(&nss_mirred_act_ops, NSS_MIRRED_TAB_MASK);
	if (err) {
		unregister_netdevice_notifier(&nss_mirred_device_notifier);
		return err;
	}

	err = nf_register_hooks(nss_mirred_igs_nf_ops, ARRAY_SIZE(nss_mirred_igs_nf_ops));
	if (err < 0) {
		nss_igs_error("Registering ingress nf hooks failed, ret: %d\n", err);
		tcf_unregister_action(&nss_mirred_act_ops);
		unregister_netdevice_notifier(&nss_mirred_device_notifier);
		return err;
	}

	nss_ifb_init();
	return 0;
}

/*
 * nss_mirred_cleanup_module()
 *	nssmirred exit function.
 */
static void __exit nss_mirred_cleanup_module(void)
{
	nf_unregister_hooks(nss_mirred_igs_nf_ops, ARRAY_SIZE(nss_mirred_igs_nf_ops));

	/*
	 * Un-register nss mirred action.
	 */
	tcf_unregister_action(&nss_mirred_act_ops);
	unregister_netdevice_notifier(&nss_mirred_device_notifier);
}

module_init(nss_mirred_init_module);
module_exit(nss_mirred_cleanup_module);

MODULE_LICENSE("Dual BSD/GPL");
