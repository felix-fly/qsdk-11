/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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

#include "dp_types.h"
#include <dp_internal.h>
#include <dp_htt.h>
#include "dp_li.h"
#include "dp_li_tx.h"
#include "dp_li_rx.h"

qdf_size_t dp_get_context_size_li(enum dp_context_type context_type)
{
	switch (context_type) {
	case DP_CONTEXT_TYPE_SOC:
		return sizeof(struct dp_soc_li);
	case DP_CONTEXT_TYPE_PDEV:
		return sizeof(struct dp_pdev_li);
	case DP_CONTEXT_TYPE_VDEV:
		return sizeof(struct dp_vdev_li);
	case DP_CONTEXT_TYPE_PEER:
		return sizeof(struct dp_peer_li);
	default:
		return 0;
	}
}

static QDF_STATUS dp_soc_attach_li(struct dp_soc *soc)
{
	soc->wbm_sw0_bm_id = hal_tx_get_wbm_sw0_bm_id();

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_soc_detach_li(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_soc_init_li(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_soc_deinit_li(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_pdev_attach_li(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_pdev_detach_li(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_vdev_attach_li(struct dp_soc *soc, struct dp_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dp_vdev_detach_li(struct dp_soc *soc, struct dp_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

qdf_size_t dp_get_soc_context_size_li(void)
{
	return sizeof(struct dp_soc);
}

#ifdef NO_RX_PKT_HDR_TLV
/**
 * dp_rxdma_ring_sel_cfg_li() - Setup RXDMA ring config
 * @soc: Common DP soc handle
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_rxdma_ring_sel_cfg_li(struct dp_soc *soc)
{
	int i;
	int mac_id;
	struct htt_rx_ring_tlv_filter htt_tlv_filter = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_start = 1;
	htt_tlv_filter.mpdu_end = 1;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.packet = 1;
	htt_tlv_filter.packet_header = 0;

	htt_tlv_filter.ppdu_start = 0;
	htt_tlv_filter.ppdu_end = 0;
	htt_tlv_filter.ppdu_end_user_stats = 0;
	htt_tlv_filter.ppdu_end_user_stats_ext = 0;
	htt_tlv_filter.ppdu_end_status_done = 0;
	htt_tlv_filter.enable_fp = 1;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo = 0;

	htt_tlv_filter.fp_mgmt_filter = 0;
	htt_tlv_filter.fp_ctrl_filter = FILTER_CTRL_BA_REQ;
	htt_tlv_filter.fp_data_filter = (FILTER_DATA_UCAST |
					 FILTER_DATA_MCAST |
					 FILTER_DATA_DATA);
	htt_tlv_filter.mo_mgmt_filter = 0;
	htt_tlv_filter.mo_ctrl_filter = 0;
	htt_tlv_filter.mo_data_filter = 0;
	htt_tlv_filter.md_data_filter = 0;

	htt_tlv_filter.offset_valid = true;

	htt_tlv_filter.rx_packet_offset = soc->rx_pkt_tlv_size;
	/*Not subscribing rx_pkt_header*/
	htt_tlv_filter.rx_header_offset = 0;
	htt_tlv_filter.rx_mpdu_start_offset =
				hal_rx_mpdu_start_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_mpdu_end_offset =
				hal_rx_mpdu_end_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_msdu_start_offset =
				hal_rx_msdu_start_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_msdu_end_offset =
				hal_rx_msdu_end_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_attn_offset =
				hal_rx_attn_offset_get(soc->hal_soc);

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		if (!pdev)
			continue;

		for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
			int mac_for_pdev =
				dp_get_mac_id_for_pdev(mac_id, pdev->pdev_id);
			/*
			 * Obtain lmac id from pdev to access the LMAC ring
			 * in soc context
			 */
			int lmac_id =
				dp_get_lmac_id_for_pdev_id(soc, mac_id,
							   pdev->pdev_id);

			htt_h2t_rx_ring_cfg(soc->htt_handle, mac_for_pdev,
					    soc->rx_refill_buf_ring[lmac_id].
					    hal_srng,
					    RXDMA_BUF, RX_DATA_BUFFER_SIZE,
					    &htt_tlv_filter);
		}
	}
	return status;
}
#else

static QDF_STATUS
dp_rxdma_ring_sel_cfg_li(struct dp_soc *soc)
{
	int i;
	int mac_id;
	struct htt_rx_ring_tlv_filter htt_tlv_filter = {0};
	struct dp_srng *rx_mac_srng;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_start = 1;
	htt_tlv_filter.mpdu_end = 1;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.packet = 1;
	htt_tlv_filter.packet_header = 1;

	htt_tlv_filter.ppdu_start = 0;
	htt_tlv_filter.ppdu_end = 0;
	htt_tlv_filter.ppdu_end_user_stats = 0;
	htt_tlv_filter.ppdu_end_user_stats_ext = 0;
	htt_tlv_filter.ppdu_end_status_done = 0;
	htt_tlv_filter.enable_fp = 1;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo = 0;

	htt_tlv_filter.fp_mgmt_filter = 0;
	htt_tlv_filter.fp_ctrl_filter = FILTER_CTRL_BA_REQ;
	htt_tlv_filter.fp_data_filter = (FILTER_DATA_UCAST |
					 FILTER_DATA_MCAST |
					 FILTER_DATA_DATA);
	htt_tlv_filter.mo_mgmt_filter = 0;
	htt_tlv_filter.mo_ctrl_filter = 0;
	htt_tlv_filter.mo_data_filter = 0;
	htt_tlv_filter.md_data_filter = 0;

	htt_tlv_filter.offset_valid = true;

	htt_tlv_filter.rx_packet_offset = soc->rx_pkt_tlv_size;
	htt_tlv_filter.rx_header_offset =
				hal_rx_pkt_tlv_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_mpdu_start_offset =
				hal_rx_mpdu_start_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_mpdu_end_offset =
				hal_rx_mpdu_end_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_msdu_start_offset =
				hal_rx_msdu_start_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_msdu_end_offset =
				hal_rx_msdu_end_offset_get(soc->hal_soc);
	htt_tlv_filter.rx_attn_offset =
				hal_rx_attn_offset_get(soc->hal_soc);

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		if (!pdev)
			continue;

		for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
			int mac_for_pdev =
				dp_get_mac_id_for_pdev(mac_id, pdev->pdev_id);
			/*
			 * Obtain lmac id from pdev to access the LMAC ring
			 * in soc context
			 */
			int lmac_id =
				dp_get_lmac_id_for_pdev_id(soc, mac_id,
							   pdev->pdev_id);

			rx_mac_srng = dp_get_rxdma_ring(pdev, lmac_id);
			htt_h2t_rx_ring_cfg(soc->htt_handle, mac_for_pdev,
					    rx_mac_srng->hal_srng,
					    RXDMA_BUF, RX_DATA_BUFFER_SIZE,
					    &htt_tlv_filter);
		}
	}
	return status;

}
#endif

void dp_initialize_arch_ops_li(struct dp_arch_ops *arch_ops)
{
#ifndef QCA_HOST_MODE_WIFI_DISABLED
	arch_ops->tx_hw_enqueue = dp_tx_hw_enqueue_li;
	arch_ops->dp_rx_process = dp_rx_process_li;
	arch_ops->tx_comp_get_params_from_hal_desc =
		dp_tx_comp_get_params_from_hal_desc_li;
	arch_ops->dp_wbm_get_rx_desc_from_hal_desc =
			dp_wbm_get_rx_desc_from_hal_desc_li;
	arch_ops->dp_tx_desc_pool_init = dp_tx_desc_pool_init_li;
	arch_ops->dp_tx_desc_pool_deinit = dp_tx_desc_pool_deinit_li;
	arch_ops->dp_rx_desc_pool_init = dp_rx_desc_pool_init_li;
	arch_ops->dp_rx_desc_pool_deinit = dp_rx_desc_pool_deinit_li;
#else
	arch_ops->dp_rx_desc_pool_init = dp_rx_desc_pool_init_generic;
	arch_ops->dp_rx_desc_pool_deinit = dp_rx_desc_pool_deinit_generic;
#endif
	arch_ops->txrx_get_context_size = dp_get_context_size_li;
	arch_ops->txrx_soc_attach = dp_soc_attach_li;
	arch_ops->txrx_soc_detach = dp_soc_detach_li;
	arch_ops->txrx_soc_init = dp_soc_init_li;
	arch_ops->txrx_soc_deinit = dp_soc_deinit_li;
	arch_ops->txrx_pdev_attach = dp_pdev_attach_li;
	arch_ops->txrx_pdev_detach = dp_pdev_detach_li;
	arch_ops->txrx_vdev_attach = dp_vdev_attach_li;
	arch_ops->txrx_vdev_detach = dp_vdev_detach_li;
	arch_ops->dp_rx_desc_cookie_2_va =
			dp_rx_desc_cookie_2_va_li;

	arch_ops->dp_rxdma_ring_sel_cfg = dp_rxdma_ring_sel_cfg_li;
}

