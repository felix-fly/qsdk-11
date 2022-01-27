/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
#include "dp_peer.h"
#include "qdf_nbuf.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_tx.h"
#include "dp_mscs.h"

#define DP_MSCS_INVALID_TID 0xFF
#define DP_MSCS_VALID_TID_MASK 0x7

/**
 * dp_mscs_peer_lookup_n_get_priority() - Get priority for MSCS peer
 * @soc_hdl - soc handle
 * @src_mac_addr - src mac address from connection
 * @nbuf - network buffer
 *
 * Return: 0 when peer has active mscs session and valid user priority
 */
int dp_mscs_peer_lookup_n_get_priority(struct cdp_soc_t *soc_hdl,
		uint8_t *src_mac_addr, uint8_t *dst_mac_addr, qdf_nbuf_t nbuf)
{
	struct dp_peer *src_peer;
	struct dp_peer *dst_peer;
	uint8_t user_prio_bitmap;
	uint8_t user_prio_limit;
	uint8_t user_prio;
	int status = 0;
	struct dp_soc *dpsoc = cdp_soc_t_to_dp_soc(soc_hdl);

	if (!dpsoc) {
		QDF_TRACE(QDF_MODULE_ID_MSCS, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid soc\n", __func__);
		return DP_MSCS_PEER_LOOKUP_STATUS_ALLOW_INVALID_QOS_TAG_UPDATE;
	}

	/*
	 * Find the MSCS peer from global soc
	 */
	src_peer = dp_peer_find_hash_find(dpsoc, src_mac_addr, 0,
			DP_VDEV_ALL, DP_MOD_ID_MSCS);

	if (!src_peer) {
		dst_peer = dp_peer_find_hash_find(dpsoc, dst_mac_addr, 0,
				DP_VDEV_ALL, DP_MOD_ID_MSCS);
		if (dst_peer && dst_peer->mscs_active &&
				!qdf_nbuf_get_priority(nbuf)) {
			/*
			 * This is a downlink flow with priority 0 for MSCS
			 * client, this should not be accelerated as uplink
			 * flow is the one which needs to be accelerated first
			 */
			dp_peer_unref_delete(dst_peer, DP_MOD_ID_MSCS);
			return DP_MSCS_PEER_LOOKUP_STATUS_DENY_QOS_TAG_UPDATE;
		} else {
			if (dst_peer)
				dp_peer_unref_delete(dst_peer, DP_MOD_ID_MSCS);

			/*
			 * No WLAN client peer found with this peer mac
			 */
			return DP_MSCS_PEER_LOOKUP_STATUS_ALLOW_INVALID_QOS_TAG_UPDATE;
		}
	}

	/*
	 * check if there is any active MSCS session for this peer
	 */
	if (!src_peer->mscs_active) {
		QDF_TRACE(QDF_MODULE_ID_MSCS, QDF_TRACE_LEVEL_DEBUG,
		"%s: MSCS session not active on peer or peer delete in progress\n", __func__);
		status = DP_MSCS_PEER_LOOKUP_STATUS_ALLOW_INVALID_QOS_TAG_UPDATE;
		goto fail;
	}

	/*
	 * Get user priority bitmap for this peer MSCS active session
	 */
	user_prio_bitmap = src_peer->mscs_ipv4_parameter.user_priority_bitmap;
	user_prio_limit = src_peer->mscs_ipv4_parameter.user_priority_limit;
	user_prio = qdf_nbuf_get_priority(nbuf) & DP_MSCS_VALID_TID_MASK;

	/*
	 * check if nbuf priority is matching with any of the valid priority value
	 */
	if (!((1 << user_prio) & user_prio_bitmap)) {
		QDF_TRACE(QDF_MODULE_ID_MSCS, QDF_TRACE_LEVEL_DEBUG,
		"%s: Nbuf TID is not valid, no match in user prioroty bitmap\n", __func__);
		status = DP_MSCS_PEER_LOOKUP_STATUS_DENY_QOS_TAG_UPDATE;
		goto fail;
	}

	user_prio = QDF_MIN(user_prio, user_prio_limit);

	/*
	 * Update skb priority
	 */
	qdf_nbuf_set_priority(nbuf, user_prio);
	QDF_TRACE(QDF_MODULE_ID_MSCS, QDF_TRACE_LEVEL_DEBUG,
			"%s: User priority for this MSCS session %d\n", __func__, user_prio);
	status = DP_MSCS_PEER_LOOKUP_STATUS_ALLOW_MSCS_QOS_TAG_UPDATE;

fail:
	if (src_peer)
		dp_peer_unref_delete(src_peer, DP_MOD_ID_MSCS);
	return status;
}

qdf_export_symbol(dp_mscs_peer_lookup_n_get_priority);
