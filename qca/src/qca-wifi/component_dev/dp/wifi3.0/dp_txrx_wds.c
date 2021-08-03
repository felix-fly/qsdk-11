/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include "../../../cmn_dev/fw_hdr/fw/htt.h"
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_nbuf.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_tx.h"
#include "enet.h"
#include "dp_txrx_wds.h"

/* Generic AST entry aging timer value */
#define DP_AST_AGING_TIMER_DEFAULT_MS	1000
#define DP_VLAN_UNTAGGED 0
#define DP_VLAN_TAGGED_MULTICAST 1
#define DP_VLAN_TAGGED_UNICAST 2
#define DP_MAX_VLAN_IDS 4096

static void dp_ast_aging_timer_fn(void *soc_hdl)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev;
	struct dp_peer *peer;
	struct dp_ast_entry *ase, *temp_ase;
	int i;
	bool check_wds_ase = false;

	if (soc->wds_ast_aging_timer_cnt++ >= DP_WDS_AST_AGING_TIMER_CNT) {
		soc->wds_ast_aging_timer_cnt = 0;
		check_wds_ase = true;
	}

	 /* Peer list access lock */
	qdf_spin_lock_bh(&soc->peer_ref_mutex);

	/* AST list access lock */
	qdf_spin_lock_bh(&soc->ast_lock);

	for (i = 0; i < MAX_PDEV_CNT && soc->pdev_list[i]; i++) {
		pdev = soc->pdev_list[i];
		qdf_spin_lock_bh(&pdev->vdev_list_lock);
		DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev) {
			DP_VDEV_ITERATE_PEER_LIST(vdev, peer) {
				DP_PEER_ITERATE_ASE_LIST(peer, ase, temp_ase) {
					/*
					 * Do not expire static ast entries
					 * and HM WDS entries
					 */
					if (ase->type !=
					    CDP_TXRX_AST_TYPE_WDS &&
					    ase->type !=
					    CDP_TXRX_AST_TYPE_MEC &&
					    ase->type !=
					    CDP_TXRX_AST_TYPE_DA)
						continue;

					/* Expire MEC entry every n sec.
					 * This needs to be expired in
					 * case if STA backbone is made as
					 * AP backbone, In this case it needs
					 * to be re-added as a WDS entry.
					 */
					if (ase->is_active && ase->type ==
					    CDP_TXRX_AST_TYPE_MEC) {
						ase->is_active = FALSE;
						continue;
					} else if (ase->is_active &&
						   check_wds_ase) {
						ase->is_active = FALSE;
						continue;
					}

					if (ase->type ==
					    CDP_TXRX_AST_TYPE_MEC) {
						DP_STATS_INC(soc,
							     ast.aged_out, 1);
						dp_peer_del_ast(soc, ase);
					} else if (check_wds_ase) {
						DP_STATS_INC(soc,
							     ast.aged_out, 1);
						dp_peer_del_ast(soc, ase);
					}
				}
			}
		}
		qdf_spin_unlock_bh(&pdev->vdev_list_lock);
	}

	qdf_spin_unlock_bh(&soc->ast_lock);
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	if (qdf_atomic_read(&soc->cmn_init_done))
		qdf_timer_mod(&soc->ast_aging_timer,
			      DP_AST_AGING_TIMER_DEFAULT_MS);
}

/*
 * dp_soc_wds_attach() - Setup WDS timer and AST table
 * @soc:		Datapath SOC handle
 *
 * Return: None
 */
void dp_soc_wds_attach(struct dp_soc *soc)
{
	soc->wds_ast_aging_timer_cnt = 0;
	qdf_timer_init(soc->osdev, &soc->ast_aging_timer,
		       dp_ast_aging_timer_fn, (void *)soc,
		       QDF_TIMER_TYPE_WAKE_APPS);

	qdf_timer_mod(&soc->ast_aging_timer, DP_AST_AGING_TIMER_DEFAULT_MS);
}

/*
 * dp_soc_wds_detach() - Detach WDS data structures and timers
 * @txrx_soc: DP SOC handle
 *
 * Return: None
 */
void dp_soc_wds_detach(struct dp_soc *soc)
{
	qdf_timer_stop(&soc->ast_aging_timer);
	qdf_timer_free(&soc->ast_aging_timer);
}

/**
 * dp_rx_da_learn() - Add AST entry based on DA lookup
 *			This is a WAR for HK 1.0 and will
 *			be removed in HK 2.0
 *
 * @soc: core txrx main context
 * @rx_tlv_hdr	: start address of rx tlvs
 * @ta_peer	: Transmitter peer entry
 * @nbuf	: nbuf to retrieve destination mac for which AST will be added
 *
 */
void
dp_rx_da_learn(struct dp_soc *soc,
	       uint8_t *rx_tlv_hdr,
	       struct dp_peer *ta_peer,
	       qdf_nbuf_t nbuf)
{
	/* For HKv2 DA port learing is not needed */
	if (qdf_likely(soc->ast_override_support))
		return;

	if (qdf_unlikely(!ta_peer))
		return;

	if (qdf_unlikely(ta_peer->vdev->opmode != wlan_op_mode_ap))
		return;

	if (!soc->da_war_enabled)
		return;

	if (qdf_unlikely(!qdf_nbuf_is_da_valid(nbuf) &&
			 !qdf_nbuf_is_da_mcbc(nbuf))) {
		dp_peer_add_ast(soc,
				ta_peer,
				qdf_nbuf_data(nbuf),
				CDP_TXRX_AST_TYPE_DA,
				IEEE80211_NODE_F_WDS_HM);
	}
}

/**
 * dp_tx_mec_handler() - Tx  MEC Notify Handler
 * @vdev: pointer to dp dev handler
 * @status : Tx completion status from HTT descriptor
 *
 * Handles MEC notify event sent from fw to Host
 *
 * Return: none
 */
void dp_tx_mec_handler(struct dp_vdev *vdev, uint8_t *status)
{
	struct dp_soc *soc;
	uint32_t flags = IEEE80211_NODE_F_WDS_HM;
	struct dp_peer *peer;
	uint8_t mac_addr[QDF_MAC_ADDR_SIZE], i;

	if (!vdev->mec_enabled)
		return;

	/* MEC required only in STA mode */
	if (vdev->opmode != wlan_op_mode_sta)
		return;

	soc = vdev->pdev->soc;
	peer = vdev->vap_bss_peer;

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  FL("peer is NULL"));
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s Tx MEC Handler",
		  __func__);

	for (i = 0; i < QDF_MAC_ADDR_SIZE; i++)
		mac_addr[(QDF_MAC_ADDR_SIZE - 1) - i] =
					status[(QDF_MAC_ADDR_SIZE - 2) + i];

	if (qdf_mem_cmp(mac_addr, vdev->mac_addr.raw, QDF_MAC_ADDR_SIZE))
		dp_peer_add_ast(soc,
				peer,
				mac_addr,
				CDP_TXRX_AST_TYPE_MEC,
				flags);
}

/**
 * dp_txrx_set_wds_rx_policy() - API to store datapath
 *                            config parameters
 * @vdev_handle - datapath vdev handle
 * @cfg: ini parameter handle
 *
 * Return: status
 */
#ifdef WDS_VENDOR_EXTENSION
void
dp_txrx_set_wds_rx_policy(struct cdp_vdev *vdev_handle,	u_int32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_peer *peer;

	if (vdev->opmode == wlan_op_mode_ap) {
		/* for ap, set it on bss_peer */
		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			if (peer->bss_peer) {
				peer->wds_ecm.wds_rx_filter = 1;
				peer->wds_ecm.wds_rx_ucast_4addr =
					(val & WDS_POLICY_RX_UCAST_4ADDR) ?
					1 : 0;
				peer->wds_ecm.wds_rx_mcast_4addr =
					(val & WDS_POLICY_RX_MCAST_4ADDR) ?
					1 : 0;
				break;
			}
		}
	} else if (vdev->opmode == wlan_op_mode_sta) {
		peer = TAILQ_FIRST(&vdev->peer_list);
		peer->wds_ecm.wds_rx_filter = 1;
		peer->wds_ecm.wds_rx_ucast_4addr =
			(val & WDS_POLICY_RX_UCAST_4ADDR) ? 1 : 0;
		peer->wds_ecm.wds_rx_mcast_4addr =
			(val & WDS_POLICY_RX_MCAST_4ADDR) ? 1 : 0;
	}
}

/**
 * dp_txrx_peer_wds_tx_policy_update() - API to set tx wds policy
 *
 * @peer_handle - datapath peer handle
 * @wds_tx_ucast: policy for unicast transmission
 * @wds_tx_mcast: policy for multicast transmission
 *
 * Return: void
 */
void
dp_txrx_peer_wds_tx_policy_update(struct cdp_peer *peer_handle,
				  int wds_tx_ucast, int wds_tx_mcast)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;

	if (wds_tx_ucast || wds_tx_mcast) {
		peer->wds_enabled = 1;
		peer->wds_ecm.wds_tx_ucast_4addr = wds_tx_ucast;
		peer->wds_ecm.wds_tx_mcast_4addr = wds_tx_mcast;
	} else {
		peer->wds_enabled = 0;
		peer->wds_ecm.wds_tx_ucast_4addr = 0;
		peer->wds_ecm.wds_tx_mcast_4addr = 0;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "Policy Update set to :\n");
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "peer->wds_enabled %d\n", peer->wds_enabled);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "peer->wds_ecm.wds_tx_ucast_4addr %d\n",
		  peer->wds_ecm.wds_tx_ucast_4addr);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "peer->wds_ecm.wds_tx_mcast_4addr %d\n",
		  peer->wds_ecm.wds_tx_mcast_4addr);
}

int dp_wds_rx_policy_check(uint8_t *rx_tlv_hdr,
			   struct dp_vdev *vdev,
			   struct dp_peer *peer)
{
	struct dp_peer *bss_peer;
	int fr_ds, to_ds, rx_3addr, rx_4addr;
	int rx_policy_ucast, rx_policy_mcast;
	int rx_mcast = hal_rx_msdu_end_da_is_mcbc_get(rx_tlv_hdr);

	if (vdev->opmode == wlan_op_mode_ap) {
		TAILQ_FOREACH(bss_peer, &vdev->peer_list, peer_list_elem) {
			if (bss_peer->bss_peer) {
				/* if wds policy check is not enabled on this vdev, accept all frames */
				if (!bss_peer->wds_ecm.wds_rx_filter) {
					return 1;
				}
				break;
			}
		}
		rx_policy_ucast = bss_peer->wds_ecm.wds_rx_ucast_4addr;
		rx_policy_mcast = bss_peer->wds_ecm.wds_rx_mcast_4addr;
	} else {             /* sta mode */
		if (!peer->wds_ecm.wds_rx_filter) {
			return 1;
		}
		rx_policy_ucast = peer->wds_ecm.wds_rx_ucast_4addr;
		rx_policy_mcast = peer->wds_ecm.wds_rx_mcast_4addr;
	}

	/* ------------------------------------------------
	 *                       self
	 * peer-             rx  rx-
	 * wds  ucast mcast dir policy accept note
	 * ------------------------------------------------
	 * 1     1     0     11  x1     1      AP configured to accept ds-to-ds Rx ucast from wds peers, constraint met; so, accept
	 * 1     1     0     01  x1     0      AP configured to accept ds-to-ds Rx ucast from wds peers, constraint not met; so, drop
	 * 1     1     0     10  x1     0      AP configured to accept ds-to-ds Rx ucast from wds peers, constraint not met; so, drop
	 * 1     1     0     00  x1     0      bad frame, won't see it
	 * 1     0     1     11  1x     1      AP configured to accept ds-to-ds Rx mcast from wds peers, constraint met; so, accept
	 * 1     0     1     01  1x     0      AP configured to accept ds-to-ds Rx mcast from wds peers, constraint not met; so, drop
	 * 1     0     1     10  1x     0      AP configured to accept ds-to-ds Rx mcast from wds peers, constraint not met; so, drop
	 * 1     0     1     00  1x     0      bad frame, won't see it
	 * 1     1     0     11  x0     0      AP configured to accept from-ds Rx ucast from wds peers, constraint not met; so, drop
	 * 1     1     0     01  x0     0      AP configured to accept from-ds Rx ucast from wds peers, constraint not met; so, drop
	 * 1     1     0     10  x0     1      AP configured to accept from-ds Rx ucast from wds peers, constraint met; so, accept
	 * 1     1     0     00  x0     0      bad frame, won't see it
	 * 1     0     1     11  0x     0      AP configured to accept from-ds Rx mcast from wds peers, constraint not met; so, drop
	 * 1     0     1     01  0x     0      AP configured to accept from-ds Rx mcast from wds peers, constraint not met; so, drop
	 * 1     0     1     10  0x     1      AP configured to accept from-ds Rx mcast from wds peers, constraint met; so, accept
	 * 1     0     1     00  0x     0      bad frame, won't see it
	 *
	 * 0     x     x     11  xx     0      we only accept td-ds Rx frames from non-wds peers in mode.
	 * 0     x     x     01  xx     1
	 * 0     x     x     10  xx     0
	 * 0     x     x     00  xx     0      bad frame, won't see it
	 * ------------------------------------------------
	 */

	fr_ds = hal_rx_mpdu_get_fr_ds(rx_tlv_hdr);
	to_ds = hal_rx_mpdu_get_to_ds(rx_tlv_hdr);
	rx_3addr = fr_ds ^ to_ds;
	rx_4addr = fr_ds & to_ds;

	if (vdev->opmode == wlan_op_mode_ap) {
		if ((!peer->wds_enabled && rx_3addr && to_ds) ||
				(peer->wds_enabled && !rx_mcast && (rx_4addr == rx_policy_ucast)) ||
				(peer->wds_enabled && rx_mcast && (rx_4addr == rx_policy_mcast))) {
			return 1;
		}
	} else {           /* sta mode */
		if ((!rx_mcast && (rx_4addr == rx_policy_ucast)) ||
				(rx_mcast && (rx_4addr == rx_policy_mcast))) {
			return 1;
		}
	}
	return 0;
}
#endif

/**
 * dp_tx_add_groupkey_metadata - Add group key in metadata
 * @vdev: DP vdev handle
 * @msdu_info: MSDU info to be setup in MSDU descriptor
 * @group_key: Group key index programmed in metadata
 *
 * Return: void
 */
#ifdef QCA_MULTIPASS_SUPPORT
static
void dp_tx_add_groupkey_metadata(struct dp_vdev *vdev,
		struct dp_tx_msdu_info_s *msdu_info, uint16_t group_key)
{
	struct htt_tx_msdu_desc_ext2_t *meta_data =
		(struct htt_tx_msdu_desc_ext2_t *)&msdu_info->meta_data[0];

	qdf_mem_zero(meta_data, sizeof(struct htt_tx_msdu_desc_ext2_t));

	/*
	 * When attempting to send a multicast packet with multi-passphrase,
	 * host shall add HTT EXT meta data "struct htt_tx_msdu_desc_ext2_t"
	 * ref htt.h indicating the group_id field in "key_flags" also having
	 * "valid_key_flags" as 1. Assign “key_flags = group_key_ix”.
	 */
	HTT_TX_MSDU_EXT2_DESC_FLAG_VALID_KEY_FLAGS_SET(msdu_info->meta_data[0], 1);
	HTT_TX_MSDU_EXT2_DESC_KEY_FLAGS_SET(msdu_info->meta_data[2], group_key);
}

/**
 * dp_tx_remove_vlan_tag - Remove 4 bytes of vlan tag
 * @vdev: DP vdev handle
 * @tx_desc: Tx Descriptor Handle
 *
 * Return: void
 */
static
void dp_tx_remove_vlan_tag(struct dp_vdev *vdev, qdf_nbuf_t nbuf)
{
	struct vlan_ethhdr veth_hdr;
	struct vlan_ethhdr *veh = (struct vlan_ethhdr *)nbuf->data;

	/*
	 * Extract VLAN header of 4 bytes:
	 * Frame Format : {dst_addr[6], src_addr[6], 802.1Q header[4], EtherType[2], Payload}
	 * Before Removal : xx xx xx xx xx xx xx xx xx xx xx xx 81 00 00 02 08 00 45 00 00...
	 * After Removal  : xx xx xx xx xx xx xx xx xx xx xx xx 08 00 45 00 00...
	 */
	qdf_mem_copy(&veth_hdr, veh, sizeof(veth_hdr));
	qdf_nbuf_pull_head(nbuf, ETHERTYPE_VLAN_LEN);
	veh = (struct vlan_ethhdr *)nbuf->data;
	qdf_mem_copy(veh, &veth_hdr, 2 * QDF_MAC_ADDR_SIZE);
	return;
}

/**
 * dp_tx_need_multipass_process - If frame needs multipass phrase processing
 * @vdev: DP vdev handle
 * @tx_desc: Tx Descriptor Handle
 * @vlan_id: vlan id of frame
 *
 * Return: whether peer is special or classic
 */
static
uint8_t dp_tx_need_multipass_process(struct dp_soc *soc, struct dp_vdev *vdev,
			   qdf_nbuf_t buf, uint16_t *vlan_id)
{
	struct dp_peer *peer = NULL;
	qdf_ether_header_t *eh = (qdf_ether_header_t *)qdf_nbuf_data(buf);
	struct vlan_ethhdr *veh = NULL;
	bool not_vlan = ((vdev->tx_encap_type == htt_cmn_pkt_type_raw) ||
			(htons(eh->ether_type) != ETH_P_8021Q));

	if (qdf_unlikely(not_vlan))
		return DP_VLAN_UNTAGGED;

	veh = (struct vlan_ethhdr *)eh;
	*vlan_id = (ntohs(veh->h_vlan_TCI) & VLAN_VID_MASK);

	if (qdf_unlikely(DP_FRAME_IS_MULTICAST((eh)->ether_dhost))) {
		qdf_spin_lock_bh(&vdev->mpass_peer_mutex);
		TAILQ_FOREACH(peer, &vdev->mpass_peer_list,
			      mpass_peer_list_elem) {
			if (*vlan_id == peer->vlan_id) {
				qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);
				return DP_VLAN_TAGGED_MULTICAST;
			}
		}
		qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);
		return DP_VLAN_UNTAGGED;
	}

	peer = dp_peer_find_hash_find(soc, eh->ether_dhost, 0, DP_VDEV_ALL);

	if (qdf_unlikely(peer == NULL))
		return DP_VLAN_UNTAGGED;

	/*
	 * Do not drop the frame when vlan_id doesn't match.
	 * Send the frame as it is.
	 */
	if (*vlan_id == peer->vlan_id) {
		dp_peer_unref_delete(peer);
		return DP_VLAN_TAGGED_UNICAST;
	}

	return DP_VLAN_UNTAGGED;
}

/**
 * dp_tx_multipass_process - Process vlan frames in tx path
 * @soc: dp soc handle
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @msdu_info: msdu descriptor
 *
 * Return: status whether frame needs to be dropped or transmitted
 */
bool dp_tx_multipass_process(struct dp_soc *soc, struct dp_vdev *vdev,
			     qdf_nbuf_t nbuf,
			     struct dp_tx_msdu_info_s *msdu_info)
{
	uint16_t vlan_id = 0;
	uint16_t group_key = 0;
	uint8_t is_spcl_peer = DP_VLAN_UNTAGGED;
	qdf_nbuf_t nbuf_copy = NULL;

	if (HTT_TX_MSDU_EXT2_DESC_FLAG_VALID_KEY_FLAGS_GET(msdu_info->meta_data[0])) {
		return true;
	}

	is_spcl_peer = dp_tx_need_multipass_process(soc, vdev, nbuf, &vlan_id);

	if ((is_spcl_peer != DP_VLAN_TAGGED_MULTICAST) &&
	    (is_spcl_peer != DP_VLAN_TAGGED_UNICAST))
		return true;

	if (is_spcl_peer == DP_VLAN_TAGGED_UNICAST) {
		dp_tx_remove_vlan_tag(vdev, nbuf);
		return true;
	}

	/* AP can have classic clients, special clients &
	 * classic repeaters.
	 * 1. Classic clients & special client:
	 *	Remove vlan header, find corresponding group key
	 *	index, fill in metaheader and enqueue multicast
	 *	frame to TCL.
	 * 2. Classic repeater:
	 *	Pass through to classic repeater with vlan tag
	 *	intact without any group key index. Hardware
	 *	will know which key to use to send frame to
	 *	repeater.
	 */
	nbuf_copy = qdf_nbuf_copy(nbuf);

	/*
	 * Send multicast frame to special peers even
	 * if pass through to classic repeater fails.
	 */
	if (nbuf_copy) {
		struct dp_tx_msdu_info_s msdu_info_copy;
		qdf_mem_zero(&msdu_info_copy, sizeof(msdu_info_copy));
		msdu_info_copy.tid = HTT_TX_EXT_TID_INVALID;
		HTT_TX_MSDU_EXT2_DESC_FLAG_VALID_KEY_FLAGS_SET(msdu_info_copy.meta_data[0], 1);
		nbuf_copy = dp_tx_send_msdu_single(vdev, nbuf_copy, &msdu_info_copy, HTT_INVALID_PEER, NULL);
		if (nbuf_copy) {
			qdf_nbuf_free(nbuf_copy);
			qdf_err("nbuf_copy send failed");
		}
	}

	group_key = vdev->iv_vlan_map[vlan_id];

	/*
	 * If group key is not installed, drop the frame.
	 */
	if (!group_key)
		return false;

	dp_tx_remove_vlan_tag(vdev, nbuf);
	dp_tx_add_groupkey_metadata(vdev, msdu_info, group_key);
	msdu_info->exception_fw = 1;
	return true;
}

/**
 * dp_rx_multipass_process - insert vlan tag on frames for traffic separation
 * @vdev: DP vdev handle
 * @nbuf: skb
 * @tid: traffic priority
 *
 * Return: bool: true if tag is inserted else false
 */
bool dp_rx_multipass_process(struct dp_peer *peer, qdf_nbuf_t nbuf, uint8_t tid)
{
	qdf_ether_header_t *eh = (qdf_ether_header_t *)qdf_nbuf_data(nbuf);
	struct vlan_ethhdr vethhdr;

	if (qdf_unlikely(!peer->vlan_id))
	       return false;

	if (qdf_unlikely(qdf_nbuf_headroom(nbuf) < ETHERTYPE_VLAN_LEN))
		return false;

	/*
	 * Form the VLAN header and insert in nbuf
	 */
	qdf_mem_copy(vethhdr.h_dest, eh->ether_dhost, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(vethhdr.h_source, eh->ether_shost, QDF_MAC_ADDR_SIZE);
	vethhdr.h_vlan_proto = htons(QDF_ETH_TYPE_8021Q);
	vethhdr.h_vlan_TCI = htons(((tid & 0x7) << VLAN_PRIO_SHIFT) |
			      (peer->vlan_id & VLAN_VID_MASK));

	/*
	 * Packet format : DSTMAC | SRCMAC | <VLAN HEADERS TO BE INSERTED> | ETHERTYPE | IP HEADER
	 * DSTMAC: 6 BYTES
	 * SRCMAC: 6 BYTES
	 * VLAN HEADER: 4 BYTES ( TPID | PCP | VLAN ID)
	 * ETHERTYPE: 2 BYTES
	 */
	qdf_nbuf_push_head(nbuf, sizeof(struct vlan_hdr));
	qdf_mem_copy(qdf_nbuf_data(nbuf), &vethhdr,
		     sizeof(struct vlan_ethhdr)- ETHERNET_TYPE_LEN);

	return true;
}

/**
 * dp_peer_multipass_list_remove: remove peer from list
 * @peer: pointer to peer
 *
 * return: void
 */
void dp_peer_multipass_list_remove(struct dp_peer *peer)
{
	struct dp_vdev *vdev = peer->vdev;
	struct dp_peer *tpeer = NULL;
	bool found = 0;

	qdf_spin_lock_bh(&vdev->mpass_peer_mutex);
	TAILQ_FOREACH(tpeer, &vdev->mpass_peer_list, mpass_peer_list_elem) {
		if (tpeer == peer) {
			found = 1;
			TAILQ_REMOVE(&vdev->mpass_peer_list, peer, mpass_peer_list_elem);
			break;
		}
	}

	qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);

	if (found)
		dp_peer_unref_delete(peer);
}

/**
 * dp_peer_multipass_list_add: add to new multipass list
 * @dp_soc: soc handle
 * @dp_vdev: vdev handle
 * @peer_mac: mac address
 *
 * return: void
 */
static void dp_peer_multipass_list_add(struct dp_soc *soc, struct dp_vdev *vdev,
					uint8_t *peer_mac)
{
	struct dp_peer *peer = dp_peer_find_hash_find(soc, peer_mac, 0,
						      vdev->vdev_id);

	if (!peer) {
		return;
	}

	/*
	 * Ref_cnt is incremented inside dp_peer_find_hash_find().
	 * Decrement it when element is deleted from the list.
	 */
	qdf_spin_lock_bh(&vdev->mpass_peer_mutex);
	TAILQ_INSERT_HEAD(&vdev->mpass_peer_list, peer, mpass_peer_list_elem);
	qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);
}

/**
 * dp_peer_set_vlan_id: set vlan_id for this peer
 * @cdp_soc: soc handle
 * @peer_mac: mac address
 * @vlan_id: vlan id for peer
 *
 * return: void
 */
void dp_peer_set_vlan_id(struct cdp_soc_t *cdp_soc,
		struct cdp_vdev *vdev_handle, uint8_t *peer_mac,
		uint16_t vlan_id)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_peer *peer = NULL;

	if (!vdev->multipass_en)
		return;

	peer = dp_peer_find_hash_find(soc, peer_mac, 0, vdev->vdev_id);

	if (qdf_unlikely(!peer)) {
		qdf_err("NULL peer");
		return;
	}

	peer->vlan_id = vlan_id;

	/* Ref_cnt is incremented inside dp_peer_find_hash_find().
	 * Decrement it here.
	 */
	dp_peer_unref_delete(peer);
	dp_peer_multipass_list_add(soc, vdev, peer_mac);
}

/**
 * dp_set_vlan_groupkey: set vlan map for vdev
 * @vdev_handle: pointer to vdev
 * @vlan_id: vlan_id
 * @group_key: group key for vlan
 *
 * return: set success/failure
 */
QDF_STATUS dp_set_vlan_groupkey(struct cdp_vdev *vdev_handle,
		uint16_t vlan_id, uint16_t group_key)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;

	if (!vdev->multipass_en)
		return QDF_STATUS_E_INVAL;

	if (!vdev->iv_vlan_map) {
		uint16_t vlan_map_size = (sizeof(uint16_t))*DP_MAX_VLAN_IDS;
		vdev->iv_vlan_map = (uint16_t *)qdf_mem_malloc(vlan_map_size);

		if (!vdev->iv_vlan_map) {
			QDF_TRACE_ERROR(QDF_MODULE_ID_DP, "iv_vlan_map");
			return QDF_STATUS_E_NOMEM;
		}

		/*
		 * 0 is invalid group key.
		 * Initilalize array with invalid group keys.
		 */
		qdf_mem_zero(vdev->iv_vlan_map, vlan_map_size);
	}

	if (vlan_id >= DP_MAX_VLAN_IDS)
		return QDF_STATUS_E_INVAL;

	vdev->iv_vlan_map[vlan_id] = group_key;
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_vdev_multipass_deinit: set vlan map for vdev
 * @vdev_handle: pointer to vdev
 *
 * return: void
 */
void dp_tx_vdev_multipass_deinit(struct dp_vdev *vdev)
{
	struct dp_peer *peer = NULL;
	qdf_spin_lock_bh(&vdev->mpass_peer_mutex);
	TAILQ_FOREACH(peer, &vdev->mpass_peer_list, mpass_peer_list_elem)
		qdf_err("Peers present in mpass list : %llx",
			peer->mac_addr.raw);
	qdf_spin_unlock_bh(&vdev->mpass_peer_mutex);

	if (vdev->iv_vlan_map) {
		qdf_mem_free(vdev->iv_vlan_map);
		vdev->iv_vlan_map = NULL;
	}

	qdf_spinlock_destroy(&vdev->mpass_peer_mutex);
}

/**
 * dp_peer_multipass_list_init: initialize peer mulitpass list
 * @vdev_handle: pointer to vdev
 *
 * return: set success/failure
 */
void dp_peer_multipass_list_init(struct dp_vdev *vdev)
{
	/*
	 * vdev->iv_vlan_map is allocated when the first configuration command
	 * is issued to avoid unnecessary allocation for regular mode VAP.
	 */
	TAILQ_INIT(&vdev->mpass_peer_list);
	qdf_spinlock_create(&vdev->mpass_peer_mutex);
}
#endif
