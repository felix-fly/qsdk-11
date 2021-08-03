/*
 ***************************************************************************
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
 ***************************************************************************
 */

#include <linux/version.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <net/genetlink.h>
#include <nss_api_if.h>
#include <nss_nl_if.h>
#include "nss_nlcmn_if.h"
#include "nss_nl.h"
#include "nss_nlgre_redir_if.h"

#define DRIVER_NAME "nss_gre_redir"

static struct nss_nlgre_redir_pvt_data pvt_data[NSS_NLGRE_REDIR_MAX_TUNNELS];
static const struct net_device_ops gre_netdev_ops;

/*
 * nss_nlgre_redir_mgr_get_pvt_data_index()
 *	Returns Index in array of private data.
 */
static bool nss_nlgre_redir_mgr_get_pvt_data_index(struct net_device *dev, uint32_t *index)
{
	uint32_t i;
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	*index = -1;
	if (!dev) {
		nss_nl_error("%p: Dev is NULL\n", nss_ctx);
		return false;
	}

	for (i = 0; i < NSS_NLGRE_REDIR_MAX_TUNNELS; i++) {
		if (pvt_data[i].dev != dev) {
			continue;
		}

		*index = i;
		return true;
	}

	return false;
}

/*
 * nss_nlgre_redir_mgr_xmit()
 *	Used when the interface is used for transmit data.
 */
static netdev_tx_t nss_nlgre_redir_mgr_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct nss_gre_redir_ndev_priv *priv;
	struct nss_gre_redir_encap_per_pkt_metadata *gre_encap = NULL;
	uint32_t ifnum, ret = 0;
	uint32_t *mdata_ptr = (uint32_t *)&skb->head[0];
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	priv = netdev_priv(dev);
	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_HOST_INNER);
	*mdata_ptr = NSS_GRE_REDIR_PER_PACKET_METADATA_OFFSET;

	/*
	 * configuring gre meta data
	 */
	gre_encap =  (struct nss_gre_redir_encap_per_pkt_metadata *)(skb->head + NSS_GRE_REDIR_PER_PACKET_METADATA_OFFSET);
	if (gre_encap == NULL) {
		nss_nl_error("%p: Not a valid skb to transmit\n", nss_ctx);
		dev_kfree_skb_any(skb);
		return -EINVAL;
	}

	memset(gre_encap, 0, sizeof(struct nss_gre_redir_encap_per_pkt_metadata));
	gre_encap->gre_flags = 0;
	gre_encap->gre_prio = 0;
	gre_encap->gre_seq = ++priv->gre_seq;
	gre_encap->gre_tunnel_id = 10;
	gre_encap->ip_dscp = 0;
	gre_encap->ip_df_override = 0;
	gre_encap->ipsec_pattern = 0;

	nss_ctx = nss_gre_redir_get_context();
	ret = nss_gre_redir_tx_buf(nss_ctx, skb, ifnum);
	if (ret != NSS_TX_SUCCESS) {
		nss_nl_error("%p: Transmit failed and returned with %d\n", nss_ctx, ret);
		dev_kfree_skb_any(skb);
	}

	return ret;
}

/*
 * nss_nlgre_redir_mgr_host_data_cb()
 *	Data callback for host offload inner node.
 */
static void nss_nlgre_redir_mgr_host_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_gre_redir_decap_per_pkt_metadata *gre_decap = NULL;
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	if (!skb) {
		nss_nl_error("%p: SKB in NULL\n", nss_ctx);
		return;
	}

	gre_decap = (struct nss_gre_redir_decap_per_pkt_metadata *)(skb->head +  NSS_GRE_REDIR_PER_PACKET_METADATA_OFFSET);
	if (!gre_decap) {
		nss_nl_error("%p: Not a valid skb\n", nss_ctx);
		dev_kfree_skb_any(skb);
		return;
	}

	skb->protocol = eth_type_trans(skb, netdev);
	netif_receive_skb(skb);
}

/*
 * nss_nlgre_redir_mgr_print_skb()
 * 	Prints the skb data
 */
static void nss_nlgre_redir_mgr_print_skb(struct sk_buff *skb)
{
	int length;
	int iter;

	/*
	 * Check if length is less than 12 bytes
	 * Else bring down to minimum multiple of 12 bytes.
	 */
	if (skb->len < 16)
		nss_nl_trace("Skb too small to print min size: 16 bytes\n");
	else
		length = (skb->len / 16);
	/*
	 * Print first 48 bytes of sk_buff
	 */
	for (iter = 0; iter < length && length <= 3; iter++) {
		nss_nl_trace("%04xx: %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x\n", iter,
				skb->data[iter*8], skb->data[iter*8 + 1], skb->data[iter*8 + 2], skb->data[iter*8 + 3],
				skb->data[iter*8 + 4], skb->data[iter*8 + 5], skb->data[iter*8 + 6], skb->data[iter*8 + 7],
				skb->data[iter*8 + 8], skb->data[iter*8 + 9], skb->data[iter*8 + 10], skb->data[iter*8 + 11],
				skb->data[iter*8 + 10], skb->data[iter*8 + 13], skb->data[iter*8 + 14], skb->data[iter*8 + 15]);
	}
}

/*
 * nss_nlgre_redir_mgr_wifi_offl_data_cb()
 *	Data callback for wifi offload inner node.
 */
static void nss_nlgre_redir_mgr_wifi_offl_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	nss_nl_trace("Exception packet on wifi offld inner printing skb:\n");
	nss_nlgre_redir_mgr_print_skb(skb);
	dev_kfree_skb(skb);
}

/*
 * nss_nlgre_redir_mgr_sjack_data_cb
 *	Data callback for sjack inner node.
 */
static void nss_nlgre_redir_mgr_sjack_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	nss_nl_trace("Exception packet on sjack inner node printing skb:\n");
	nss_nlgre_redir_mgr_print_skb(skb);
	dev_kfree_skb(skb);
}

/*
 * nss_nlgre_redir_mgr_outer_data_cb()
 *	Data callback for outer node.
 */
static void nss_nlgre_redir_mgr_outer_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	nss_nl_trace("Exception packet on outer node printing skb:\n");
	nss_nlgre_redir_mgr_print_skb(skb);
	dev_kfree_skb(skb);
}

/*
 * nss_nlgre_redir_mgr_cb_gre_msg()
 *	HLOS->NSS message completion callback.
 */
static void nss_nlgre_redir_mgr_cb_gre_msg(void)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();
	nss_nl_info("%p: callback gre tunnel msg from NSS\n", nss_ctx);
}

/*
 * nss_nlgre_redir_mgr_deinit_pvt_data()
 *	Deinitialize private data for the given index.
 */
static bool nss_nlgre_redir_mgr_deinit_pvt_data(uint32_t index)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	if (index == NSS_NLGRE_REDIR_MAX_TUNNELS) {
		nss_nl_error("%p: Index is out of range\n", nss_ctx);
		return false;
	}

	pvt_data[index].dev = NULL;
	pvt_data[index].enable = false;
	pvt_data[index].host_inner_ifnum = -1;
	pvt_data[index].wifi_offl_inner_ifnum = -1;
	pvt_data[index].sjack_inner_ifnum = -1;
	pvt_data[index].outer_ifnum = -1;
	return true;
}

/*
 * nss_nlgre_redir_mgr_tunnel_type
 * 	Returns tunnel type
 */
enum nss_gre_redir_tunnel_types nss_nlgre_redir_mgr_tunnel_type(char *tun_type)
{
	if (tun_type == NULL)
		return NSS_GRE_REDIR_TUNNEL_TYPE_UNKNOWN;
	if (!strncmp(tun_type, "tun", NSS_NLGRE_REDIR_TUN_TYPE_MAX_SZ))
		return NSS_GRE_REDIR_TUNNEL_TYPE_TUN;
	if (!strncmp(tun_type, "dtun", NSS_NLGRE_REDIR_TUN_TYPE_MAX_SZ))
		return NSS_GRE_REDIR_TUNNEL_TYPE_DTUN;
	if (!strncmp(tun_type, "split", NSS_NLGRE_REDIR_TUN_TYPE_MAX_SZ))
		return NSS_GRE_REDIR_TUNNEL_TYPE_SPLIT;

	return NSS_GRE_REDIR_TUNNEL_TYPE_UNKNOWN;
}

/*
 * nss_gre_redir_unregister_and_deallocate()
 *	Unregisters and deallocates a node.
 */
bool nss_nlgre_redir_mgr_unregister_and_deallocate(struct net_device *dev, uint32_t type)
{
	int ifnum;
	bool ret;
	nss_tx_status_t status;

	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, type);
	if (ifnum == -1) {
		nss_nl_error("%p: unable to get NSS interface for net device %s of type %d\n", dev, dev->name, type);
		return false;
	}

	ret = nss_gre_redir_unregister_if(ifnum);
	if (!ret) {
		nss_nl_error("%p: Unable to unregister interface %d\n", dev, ifnum);
		return false;
	}

	status = nss_dynamic_interface_dealloc_node(ifnum, type);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_error("%p: Unable to deallocate node %d\n", dev, ifnum);
		return false;
	}

	return true;
}

/*
 * nss_nlgre_redir_mgr_interfaces_unregister_and_dealloc
 * 	Find out the interfaces to be deallocated
 */
void nss_nlgre_redir_mgr_interfaces_unregister_and_dealloc(struct net_device *dev, int tun)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();
	if (pvt_data[tun].sjack_inner_ifnum != -1) {
		if(!nss_nlgre_redir_mgr_unregister_and_deallocate(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_HOST_INNER)) {
		nss_nl_error("%p: Unable to unregister and deallocate node of type %d\n", nss_ctx,
				NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_HOST_INNER);
		}
	}

	if (pvt_data[tun].wifi_offl_inner_ifnum != -1) {
		if (!nss_nlgre_redir_mgr_unregister_and_deallocate(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_OFFL_INNER)) {
			nss_nl_error("%p: Unable to unregister and deallocate node of type %d\n", nss_ctx,
				NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_OFFL_INNER);
		}
	}

	if (pvt_data[tun].host_inner_ifnum != -1) {
		if (!nss_nlgre_redir_mgr_unregister_and_deallocate(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_SJACK_INNER)) {
			nss_nl_error("%p: Unable to unregister and deallocate node of type %d\n", nss_ctx,
					NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_SJACK_INNER);
		}
	}

	if (pvt_data[tun].outer_ifnum != -1) {
		if (!nss_nlgre_redir_mgr_unregister_and_deallocate(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER)) {
			nss_nl_error("%p: Unable to unregister and deallocate node of type %d\n", nss_ctx,
					NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER);
		}
	}
}

/*
 * nss_nlgre_redir_mgr_ether_setup()
 * 	Used to setup the ethernet functionality
 */
void nss_nlgre_redir_mgr_ether_setup(struct net_device *dev)
{
	eth_hw_addr_random(dev);
}

/*
 * nss_nlgre_redir_mgr_destroy_tun()
 *	Unregisters and deallocs dynamic interfaces.
 */
int nss_nlgre_redir_mgr_destroy_tun(struct net_device *dev)
{
	int index;
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	if (!dev) {
		nss_nl_error("%p: Dev is NULL\n", nss_ctx);
		return -1;
	}

	if (!nss_nlgre_redir_mgr_get_pvt_data_index(dev, &index)) {
		nss_nl_error("%p: Unable to find tunnel associated with net dev\n", dev);
		return -1;
	}

	nss_nlgre_redir_mgr_interfaces_unregister_and_dealloc(dev, index);
	nss_nlgre_redir_mgr_deinit_pvt_data(index);
	dev_put(dev);
	unregister_netdev(dev);
	free_netdev(dev);

	return index;
}

/*
 * nss_nlgre_redir_mgr_interface_map()
 *	Interface map message.
 */
bool nss_nlgre_redir_mgr_interface_map(uint32_t vap_nss_if, uint32_t tun_type, struct nss_nlgre_redir_map *if_map_params)
{
	struct nss_gre_redir_msg ngrm;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t ret;
	uint32_t len = sizeof(struct nss_gre_redir_msg) - sizeof(struct nss_cmn_msg);
	nss_ctx = nss_gre_redir_get_context();

	if ((vap_nss_if >= NSS_DYNAMIC_IF_START+NSS_MAX_DYNAMIC_INTERFACES) || (vap_nss_if < NSS_DYNAMIC_IF_START)) {
		nss_nl_error("%p: vap_nss_if is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return false;
	}

	if (if_map_params->rid >= NSS_NLGRE_REDIR_RADIO_ID_MAX) {
		nss_nl_error("%p: radio_id is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return false;
	}

	if (if_map_params->vid >= NSS_NLGRE_REDIR_VAP_ID_MAX) {
		nss_nl_error("%p: vap_id is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return false;
	}

	if ((tun_type <= NSS_GRE_REDIR_TUNNEL_TYPE_UNKNOWN) || (tun_type >= NSS_GRE_REDIR_TUNNEL_TYPE_MAX)) {
		nss_nl_error("%p: not a valid tunnel_type\n", nss_ctx);
		return false;
	}

	nss_cmn_msg_init(&ngrm.cm, NSS_GRE_REDIR_INTERFACE, NSS_GRE_REDIR_TX_INTERFACE_MAP_MSG,
			len, nss_nlgre_redir_mgr_cb_gre_msg, NULL);

	ngrm.msg.interface_map.vap_nssif = vap_nss_if;
	ngrm.msg.interface_map.radio_id = if_map_params->rid;
	ngrm.msg.interface_map.vap_id = if_map_params->vid;
	ngrm.msg.interface_map.tunnel_type = tun_type;
	ngrm.msg.interface_map.ipsec_pattern = if_map_params->ipsec_sa_pattern;

	if (tun_type == NSS_GRE_REDIR_TUNNEL_TYPE_SPLIT) {
		ngrm.msg.interface_map.nexthop_nssif = NSS_ETH_RX_INTERFACE;
	} else {
		ngrm.msg.interface_map.nexthop_nssif = vap_nss_if;
	}

	ngrm.msg.interface_map.lag_en = 0;
	ret = nss_gre_redir_tx_msg_sync(nss_ctx, &ngrm);
	if (ret != NSS_TX_SUCCESS) {
		nss_nl_error("%p: Tx to firmware failed\n", nss_ctx);
		return false;
	}

	nss_nl_info("%p: Successfully transmitted msg to firmware\n", nss_ctx);
	return true;
}

/*
 * nss_nlgre_redir_mgr_interface_unmap()
 *	Interface unmap message.
 */
bool nss_nlgre_redir_mgr_interface_unmap(uint32_t vap_nss_if, struct nss_nlgre_redir_unmap *if_unmap_params)
{
	struct nss_gre_redir_msg ngrm;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t ret;
	uint32_t len = sizeof(struct nss_gre_redir_msg) - sizeof(struct nss_cmn_msg);
	nss_ctx = nss_gre_redir_get_context();

	if ((vap_nss_if >= NSS_DYNAMIC_IF_START+NSS_MAX_DYNAMIC_INTERFACES) || (vap_nss_if < NSS_DYNAMIC_IF_START)) {
		nss_nl_error("%p: vap_nss_if is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return false;
	}

	if (if_unmap_params->rid >= NSS_NLGRE_REDIR_RADIO_ID_MAX) {
		nss_nl_error("%p: radio_id is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return false;
	}

	if (if_unmap_params->vid >= NSS_NLGRE_REDIR_VAP_ID_MAX) {
		nss_nl_error("%p: vap_id is out of valid range for vap: %d\n", nss_ctx, vap_nss_if);
		return false;
	}

	nss_cmn_msg_init(&ngrm.cm, NSS_GRE_REDIR_INTERFACE, NSS_GRE_REDIR_TX_INTERFACE_UNMAP_MSG,
			len, nss_nlgre_redir_mgr_cb_gre_msg, NULL);
	ngrm.msg.interface_unmap.vap_nssif = vap_nss_if;
	ngrm.msg.interface_unmap.radio_id = if_unmap_params->rid;
	ngrm.msg.interface_unmap.vap_id = if_unmap_params->vid;

	ret = nss_gre_redir_tx_msg_sync(nss_ctx, &ngrm);
	if (ret != NSS_TX_SUCCESS) {
		nss_nl_error("%p: Tx to firmware failed\n", nss_ctx);
		return false;
	}

	nss_nl_info("%p: Successfully transmitted msg to firmware\n", nss_ctx);
	return true;
}

/*
 * nss_nlgre_redir_mgr_set_next_hop()
 *	Sets next hop as gre-redir for wifi.
 */
bool nss_nlgre_redir_mgr_set_next_hop(struct nss_nlgre_redir_set_next *setnext_params)
{
	void *ctx;
	int ifnumber, next_dev_ifnum, index;
	struct net_device *dev, *tundev;
	nss_tx_status_t ret;
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	dev = dev_get_by_name(&init_net, setnext_params->dev_name);
	if (!dev) {
		nss_nl_error("%p: Unable to find net device corresponding to device %s\n", nss_ctx, setnext_params->dev_name);
		return false;
	}

	ifnumber = nss_cmn_get_interface_number_by_dev(dev);
	dev_put(dev);
	if (ifnumber == -1) {
		nss_nl_error("%p: Unable to find NSS interface for net device %s\n", nss_ctx, setnext_params->dev_name);
		return false;
	}

	if (!strncmp(setnext_params->mode, "split", NSS_NLGRE_REDIR_TUN_TYPE_MAX_SZ)) {
		next_dev_ifnum = NSS_ETH_RX_INTERFACE;
	} else {
		tundev = dev_get_by_name(&init_net, setnext_params->next_dev_name);
		if (!tundev) {
			nss_nl_error("%p: Unable to find net device corresponding to device %s\n", nss_ctx, setnext_params->next_dev_name);
			return false;
		}

		if (!nss_nlgre_redir_mgr_get_pvt_data_index(tundev, &index)) {
			nss_nl_error("%p: Unable to find tunnel associated with device\n", tundev);
			dev_put(tundev);
			return false;
		}

		dev_put(tundev);
		next_dev_ifnum = pvt_data[index].wifi_offl_inner_ifnum;
	}

	nss_nl_info("%p: next hop interface number is %d\n", nss_ctx, next_dev_ifnum);
	ctx = nss_wifi_get_context();
	ret = nss_wifi_vdev_set_next_hop(ctx, ifnumber, next_dev_ifnum);

	if (ret != NSS_TX_SUCCESS) {
		nss_nl_error("%p: wifi drv api failed to set next hop\n", nss_ctx);
		return false;
	}

	return true;
}

/*
 * nss_nlgre_redir_mgr_create_tun()
 *	Allocates net_dev and configures tunnel.
 */
int nss_nlgre_redir_mgr_create_tun(struct nss_nlgre_redir_create_tun *create_params)
{
	int i, tun_idx = -1;
	struct net_device *dev;
	struct nss_gre_redir_ndev_priv *priv;
	struct nss_gre_redir_inner_configure_msg ngrm;
	struct nss_gre_redir_outer_configure_msg ngrocm;
	nss_tx_status_t status;
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();

	dev = alloc_netdev(sizeof(*priv), "gretun%d", NET_NAME_UNKNOWN, ether_setup);
	if (!dev) {
		nss_nl_error("Unable to allocate netdev\n");
		return tun_idx;
	}

	dev->needed_headroom = NSS_NLGRE_REDIR_NEEDED_HEADROOM;
	dev->netdev_ops = &gre_netdev_ops;
	for (i = 0; i < NSS_NLGRE_REDIR_MAX_TUNNELS ; i++) {
		if (!pvt_data[i].enable) {
			tun_idx = i;
			break;
		}
	}

	if (tun_idx == -1) {
		nss_nl_error("Unable to allocate any tunnel\n");
		return tun_idx;
	}

	/*
	 * Dynamic interface allocation.
	 */
	pvt_data[tun_idx].host_inner_ifnum = nss_gre_redir_alloc_and_register_node(dev,
			nss_nlgre_redir_mgr_host_data_cb,
			NULL, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_HOST_INNER, dev);
	nss_nl_info("%p: host_inner = %d\n", nss_ctx, pvt_data[tun_idx].host_inner_ifnum);
	if (pvt_data[tun_idx].host_inner_ifnum == -1) {
		nss_nl_error("%p: Unable to allocate and register wifi host inner interface\n", nss_ctx);
		goto fail;
	}

	pvt_data[tun_idx].wifi_offl_inner_ifnum = nss_gre_redir_alloc_and_register_node(dev,
			nss_nlgre_redir_mgr_wifi_offl_data_cb,
			NULL, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_OFFL_INNER, dev);
	nss_nl_info("%p: wifi_inner = %d\n", nss_ctx, pvt_data[tun_idx].wifi_offl_inner_ifnum);
	if (pvt_data[tun_idx].wifi_offl_inner_ifnum == -1) {
		nss_nl_error("%p: Unable to allocate and register wifi offload inner interface\n", nss_ctx);
		goto fail;
	}

	pvt_data[tun_idx].sjack_inner_ifnum = nss_gre_redir_alloc_and_register_node(dev,
			nss_nlgre_redir_mgr_sjack_data_cb,
			NULL, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_SJACK_INNER, dev);
	nss_nl_info("%p: sjack_inner = %d\n", nss_ctx, pvt_data[tun_idx].sjack_inner_ifnum);
	if (pvt_data[tun_idx].sjack_inner_ifnum == -1) {
		nss_nl_error("%p: Unable to allocate and register sjack inner interface\n", nss_ctx);
		goto fail;
	}

	pvt_data[tun_idx].outer_ifnum = nss_gre_redir_alloc_and_register_node(dev,
			nss_nlgre_redir_mgr_outer_data_cb,
			NULL, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER, dev);
	nss_nl_info("%p: outer = %d\n", nss_ctx, pvt_data[tun_idx].outer_ifnum);
	if (pvt_data[tun_idx].outer_ifnum == -1) {
		nss_nl_error("%p: Unable to allocate and register outer interface\n", nss_ctx);
		goto fail;
	}

	memset(&ngrm, 0, sizeof(struct nss_gre_redir_inner_configure_msg));
	memset(&ngrocm, 0, sizeof(struct nss_gre_redir_outer_configure_msg));
	ngrm.ip_hdr_type = create_params->iptype;
	ngrocm.ip_hdr_type = create_params->iptype;

	memcpy(ngrm.ip_src_addr, create_params->sip, sizeof(ngrm.ip_src_addr));
	memcpy(ngrm.ip_dest_addr, create_params->dip, sizeof(ngrm.ip_dest_addr));

	/*
	 * TODO: Dynamic assignment of values from userspace
	 * ip_df_policy value currently hard coded. This needs to be supplied from userspace.
	 */
	ngrm.ip_df_policy = 0;
	ngrm.gre_version = 0;
	ngrm.ip_ttl = 128;
	ngrm.except_outerif = pvt_data[tun_idx].outer_ifnum;

	/*
	 * TODO: Dynamic assignment of values from userspace
	 * rps_hint value currently hard coded. This needs to be supplied from userspace.
	 */
	ngrocm.rps_hint = 0;
	ngrocm.except_hostif = pvt_data[tun_idx].host_inner_ifnum;
	ngrocm.except_offlif = pvt_data[tun_idx].wifi_offl_inner_ifnum;
	ngrocm.except_sjackif = pvt_data[tun_idx].sjack_inner_ifnum;

	status = nss_gre_redir_configure_inner_node(pvt_data[tun_idx].host_inner_ifnum, &ngrm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_warn("%p: unable to configure host inner node %d\n", nss_ctx, pvt_data[tun_idx].host_inner_ifnum);
		goto fail;
	}

	status = nss_gre_redir_configure_inner_node(pvt_data[tun_idx].wifi_offl_inner_ifnum, &ngrm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_warn("%p: unable to configure wifi offload inner node %d\n", nss_ctx, pvt_data[tun_idx].host_inner_ifnum);
		goto fail;
	}

	status = nss_gre_redir_configure_inner_node(pvt_data[tun_idx].sjack_inner_ifnum, &ngrm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_warn("%p: unable to configure sjack inner node %d\n", nss_ctx, pvt_data[tun_idx].sjack_inner_ifnum);
		goto fail;
	}

	status = nss_gre_redir_configure_outer_node(pvt_data[tun_idx].outer_ifnum, &ngrocm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_warn("%p: unable to configure outer node %d\n", nss_ctx, pvt_data[tun_idx].host_inner_ifnum);
		goto fail;
	}

	nss_nlgre_redir_mgr_ether_setup(dev);
	if (register_netdev(dev)) {
		nss_nl_warn("%p: Unable to register netdev\n", nss_ctx);
		nss_nlgre_redir_mgr_destroy_tun(dev);
	}

	pvt_data[tun_idx].enable = true;
	pvt_data[tun_idx].dev = dev;

	return tun_idx;
fail:
	nss_nlgre_redir_mgr_interfaces_unregister_and_dealloc(dev, tun_idx);
	nss_nlgre_redir_mgr_deinit_pvt_data(tun_idx);
	free_netdev(dev);
	return -1;
}

/*
 * nss_nlgre_redir_mgr_open()
 *	Used when the interface is opened for use.
 */
static int nss_nlgre_redir_mgr_open(struct net_device *dev)
{
	struct nss_gre_redir_ndev_priv *priv;
	priv = netdev_priv(dev);
	priv->gre_seq = 0;
	netif_start_queue(dev);
	netif_carrier_on(dev);
	return 0;
}

/*
 * nss_nlgre_redir_mgr_close()
 *	Used when the interface is closed.
 */
static int nss_nlgre_redir_mgr_close(struct net_device *dev)
{
	netif_stop_queue (dev);
	netif_carrier_off(dev);
	return 0;
}

/*
 * nss_nlgre_redir_mgr_get_stats64()
 *	Used when to get link statistics.
 */
static struct rtnl_link_stats64 *nss_nlgre_redir_mgr_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	bool ret = false;
	int i;
	struct nss_gre_redir_tunnel_stats get_stats;

	for (i = 0; i < NSS_GRE_REDIR_MAX_INTERFACES; i++) {
		ret = nss_gre_redir_get_stats(i, &get_stats);
		if (ret == true && get_stats.dev == dev) {
			break;
		}
	}
	if (ret == false)
		return stats;

	stats->tx_bytes   = get_stats.node_stats.tx_bytes;
	stats->tx_packets = get_stats.node_stats.tx_packets;
	stats->rx_bytes   = get_stats.node_stats.rx_bytes;
	stats->rx_packets = get_stats.node_stats.rx_packets;
	for (i = 0;i < ARRAY_SIZE(get_stats.node_stats.rx_dropped); i++) {
		stats->rx_dropped += get_stats.node_stats.rx_dropped[i];
	}

	stats->tx_dropped = get_stats.tx_dropped;

	return stats;
}

/*
 * nss_nlgre_redir_mgr_set_mac_address()
 *	Sets the MAC address.
 */
static int nss_nlgre_redir_mgr_set_mac_address(struct net_device *dev, void *p)
{
	struct sockaddr *addr = (struct sockaddr *)p;

	if (!is_valid_ether_addr(addr->sa_data)) {
		nss_nl_error("MAC address validation failed \n");
		return -EINVAL;
	}

	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	return 0;
}

/*
 * net_device_ops
 *	Netdevice operations.
 */
static const struct net_device_ops gre_netdev_ops = {
	.ndo_open = nss_nlgre_redir_mgr_open,
	.ndo_stop = nss_nlgre_redir_mgr_close,
	.ndo_start_xmit = nss_nlgre_redir_mgr_xmit,
	.ndo_get_stats64 = nss_nlgre_redir_mgr_get_stats64,
	.ndo_set_mac_address = nss_nlgre_redir_mgr_set_mac_address,
};

MODULE_DESCRIPTION("NSS GRE_REDIR module");
