/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2019, The Linux Foundation. All rights reserved.
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

/*
 * nss_nlgre_redir.c
 * 	NSS Netlink gre_redir Handler
 */
#include <linux/version.h>
#include <net/genetlink.h>
#include <nss_api_if.h>
#include <nss_nlcmn_if.h>
#include <nss_nl_if.h>
#include <nss_nlgre_redir_if.h>
#include "nss_nl.h"
#include "nss_nlgre_redir.h"
#include "nss_nlgre_redir_mgr.h"

/*
 * Get max size of the array
 */
#define NSS_NLGRE_REDIR_OPS_SZ ARRAY_SIZE(nss_nlgre_redir_ops)

/*
 * nss_nlgre_redir_family
 * 	Gre_redir family definition
 */
static struct genl_family nss_nlgre_redir_family = {
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
	.name = NSS_NLGRE_REDIR_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nlgre_redir_rule),	/* NSS NETLINK gre_redir rule */
	.version = NSS_NL_VER,				/* Set it to NSS_NLGRE_REDIR version */
	.maxattr = NSS_NLGRE_REDIR_CMD_TYPE_MAX,	/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
};

/*
 * nss_nlgre_redir_mcgrp
 * 	Multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nlgre_redir_mcgrp[] = {
	{.name = NSS_NLGRE_REDIR_MCAST_GRP},
};

/*
 * nss_nlgre_redir_ops_create_tun()
 * 	Handler for tunnel create
 */
static int nss_nlgre_redir_ops_create_tun(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_family, info, NSS_NLGRE_REDIR_CMD_TYPE_CREATE_TUN);
	if (!nl_cm) {
		nss_nl_error("Unable to extract create tunnel data\n");
		ret = -EINVAL;
		goto fail;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Create a gre tunnel
	 */
	ret = nss_nlgre_redir_mgr_create_tun(&nl_rule->msg.create);
	if(ret == -1) {
		nss_nl_error("Unable to create tunnel\n");
		ret = -EAGAIN;
		goto fail;
	}

	nss_nl_info("Successfully created gretun%d tunnel\n", ret);
fail:
	return ret;
}

/*
 * nss_nlgre_redir_ops_destroy_tun()
 * 	Handler destroy tunnel
 */
static int nss_nlgre_redir_ops_destroy_tun(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct net_device *dev;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_family, info, NSS_NLGRE_REDIR_CMD_TYPE_DESTROY_TUN);
	if (!nl_cm) {
		nss_nl_error("Unable to extract destroy tunnel data\n");
		ret = -EINVAL;
		goto fail;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Get the dev reference
	 */
	dev = dev_get_by_name(&init_net, nl_rule->msg.destroy.netdev);
	if (!dev) {
		nss_nl_error("Invalid parameter: %s\n", nl_rule->msg.destroy.netdev);
		ret = -ENODEV;
		goto fail;
	}

	/*
	 * Destroy the tunnel
	 */
	ret = nss_nlgre_redir_mgr_destroy_tun(dev);
	if (ret == -1) {
		nss_nl_error("Unable to destroy tunnel: %s\n", nl_rule->msg.destroy.netdev);
		ret = -EAGAIN;
		dev_put(dev);
		goto fail;
	}

	nss_nl_info("Successfully destroyed gretun = %s tunnel\n", nl_rule->msg.destroy.netdev);
fail:
	return ret;
}

/*
 * nss_nlgre_redir_ops_map()
 * 	Handler for map command
 */
static int nss_nlgre_redir_ops_map(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	uint32_t vap_nss_if, tun_type;
	struct net_device *dev;
	int ret = 0;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_family, info, NSS_NLGRE_REDIR_CMD_TYPE_MAP);
	if (!nl_cm) {
		nss_nl_error("Unable to extract map interface data\n");
		ret = -EINVAL;
		goto fail;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Get the dev reference
	 */
	dev = dev_get_by_name(&init_net, nl_rule->msg.map.vap_nss_if);
	if (!dev) {
		nss_nl_error("Invalid parameter: vap_nss_if\n");
		ret = -ENODEV;
		goto fail;
	}

	vap_nss_if = nss_cmn_get_interface_number_by_dev(dev);
	dev_put(dev);
	tun_type = nss_nlgre_redir_mgr_tunnel_type(nl_rule->msg.map.tun_type);

	/*
	 * map the interface
	 */
	ret = nss_nlgre_redir_mgr_interface_map(vap_nss_if, tun_type, &nl_rule->msg.map);
	if(!ret) {
		nss_nl_error("Unable to map nss interface\n");
		ret = -EAGAIN;
		goto fail;
	}

	nss_nl_info("Successfully mapped nss interface.\n");
fail:
	return ret;
}

/*
 * nss_nlgre_redir_ops_unmap()
 * 	Handler for unmap command
 */
static int nss_nlgre_redir_ops_unmap(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct net_device *dev;
	int32_t vap_nss_if;
	int ret = 0;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_family, info, NSS_NLGRE_REDIR_CMD_TYPE_UNMAP);
	if (!nl_cm) {
		nss_nl_error("Unable to extract unmap data\n");
		ret = -EINVAL;
		goto fail;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Get the dev reference
	 */
	dev = dev_get_by_name(&init_net, nl_rule->msg.unmap.vap_nss_if);
	if (!dev) {
		nss_nl_error("Invalid parameter: dev_name\n");
		ret = -ENODEV;
		goto fail;
	}

	vap_nss_if = nss_cmn_get_interface_number_by_dev(dev);
	dev_put(dev);

	/*
	 * Unmap the interface
	 */
	ret = nss_nlgre_redir_mgr_interface_unmap(vap_nss_if, &nl_rule->msg.unmap);
	if(!ret) {
		nss_nl_error("Unable to unmap nss interface\n");
		ret = -EAGAIN;
		goto fail;
	}

	nss_nl_info("Successfully unmapped the nss interface.\n");
fail:
	return ret;
}

/*
 * nss_nlgre_redir_ops_set_next()
 * 	Handler for set_next command
 */
static int nss_nlgre_redir_ops_set_next(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_family, info, NSS_NLGRE_REDIR_CMD_TYPE_SET_NEXT_HOP);
	if (!nl_cm) {
		nss_nl_error("Unable to extract set_next_hop data\n");
		ret = -EINVAL;
		goto fail;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	ret = nss_nlgre_redir_mgr_set_next_hop(&nl_rule->msg.snext);
	if (!ret) {
		nss_nl_error("Unable to set next hop\n");
		ret = -EAGAIN;
		goto fail;
	}

	nss_nl_info("Successfully set the next hop\n");
fail:
	return ret;
}

/*
 * nss_nlgre_redir_get_ifnum()
 * 	Get the interface number corresponding to netdev
 */
int nss_nlgre_redir_get_ifnum(struct net_device* dev, enum nss_dynamic_interface_type type)
{
	int ifnum;

	/*
	 * Get the interface number depending upon the dev and type
	 */
	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, type);
	if (ifnum < 0) {
		nss_nl_error("%p: Failed to find interface number (dev:%s, type:%d)\n", dev, dev->name, type);
		return -1;
	}

	return ifnum;
}

/*
 * nss_nlgre_redir_ops
 * 	Operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlgre_redir_ops[] = {
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_CREATE_TUN, .doit = nss_nlgre_redir_ops_create_tun,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_DESTROY_TUN, .doit = nss_nlgre_redir_ops_destroy_tun,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_MAP, .doit = nss_nlgre_redir_ops_map,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_UNMAP, .doit = nss_nlgre_redir_ops_unmap,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_SET_NEXT_HOP, .doit = nss_nlgre_redir_ops_set_next,},
};

/*
 * nss_nlgre_redir_init()
 * 	handler init
 */
bool nss_nlgre_redir_init(void)
{
	int err;
	nss_nl_info_always("Init NSS netlink gre_redir handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	err = genl_register_family_with_ops_groups(&nss_nlgre_redir_family, nss_nlgre_redir_ops, nss_nlgre_redir_mcgrp);
	if (err) {
		nss_nl_info_always("Error: %d unable to register gre_redir family\n", err);
		return false;
	}

	return true;
}

/*
 * nss_nlgre_redir_exit()
 *	handler exit
 */
bool nss_nlgre_redir_exit(void)
{
	int err;
	nss_nl_info_always("Exit NSS netlink gre_redir handler\n");

	/*
	 * unregister the ops family
	 */
	err = genl_unregister_family(&nss_nlgre_redir_family);
	if (err) {
		nss_nl_info_always("Error: %d unable to unregister gre_redir NETLINK family\n", err);
		return false;
	}

	return true;
}
