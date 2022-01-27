/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
#include "cdp_txrx_cmn_struct.h"
#include "dp_types.h"
#include "dp_tx.h"
#include "dp_li_tx.h"
#include "dp_tx_desc.h"
#include <dp_internal.h>
#include <dp_htt.h>
#include <hal_li_api.h>
#include <hal_li_tx.h>

/*mapping between hal encrypt type and cdp_sec_type*/
#define MAX_CDP_SEC_TYPE 12
uint8_t sec_type_map[MAX_CDP_SEC_TYPE] = {HAL_TX_ENCRYPT_TYPE_NO_CIPHER,
					  HAL_TX_ENCRYPT_TYPE_WEP_128,
					  HAL_TX_ENCRYPT_TYPE_WEP_104,
					  HAL_TX_ENCRYPT_TYPE_WEP_40,
					  HAL_TX_ENCRYPT_TYPE_TKIP_WITH_MIC,
					  HAL_TX_ENCRYPT_TYPE_TKIP_NO_MIC,
					  HAL_TX_ENCRYPT_TYPE_AES_CCMP_128,
					  HAL_TX_ENCRYPT_TYPE_WAPI,
					  HAL_TX_ENCRYPT_TYPE_AES_CCMP_256,
					  HAL_TX_ENCRYPT_TYPE_AES_GCMP_128,
					  HAL_TX_ENCRYPT_TYPE_AES_GCMP_256,
					  HAL_TX_ENCRYPT_TYPE_WAPI_GCM_SM4};

void dp_tx_comp_get_params_from_hal_desc_li(struct dp_soc *soc,
					    void *tx_comp_hal_desc,
					    struct dp_tx_desc_s **r_tx_desc)
{
	uint8_t pool_id;
	uint32_t tx_desc_id;

	tx_desc_id = hal_tx_comp_get_desc_id(tx_comp_hal_desc);
	pool_id = (tx_desc_id & DP_TX_DESC_ID_POOL_MASK) >>
			DP_TX_DESC_ID_POOL_OS;

	/* Find Tx descriptor */
	*r_tx_desc = dp_tx_desc_find(soc, pool_id,
				     (tx_desc_id & DP_TX_DESC_ID_PAGE_MASK) >>
							DP_TX_DESC_ID_PAGE_OS,
				     (tx_desc_id & DP_TX_DESC_ID_OFFSET_MASK) >>
						DP_TX_DESC_ID_OFFSET_OS);
	/* Pool id is not matching. Error */
	if ((*r_tx_desc)->pool_id != pool_id) {
		dp_tx_comp_alert("Tx Comp pool id %d not matched %d",
				 pool_id, (*r_tx_desc)->pool_id);

		qdf_assert_always(0);
	}
}

QDF_STATUS
dp_tx_hw_enqueue_li(struct dp_soc *soc, struct dp_vdev *vdev,
		    struct dp_tx_desc_s *tx_desc, uint16_t fw_metadata,
		    struct cdp_tx_exception_metadata *tx_exc_metadata,
		    struct dp_tx_msdu_info_s *msdu_info)
{
	void *hal_tx_desc;
	uint32_t *hal_tx_desc_cached;
	int coalesce = 0;
	struct dp_tx_queue *tx_q = &msdu_info->tx_queue;
	uint8_t ring_id = tx_q->ring_id & DP_TX_QUEUE_MASK;
	uint8_t tid = msdu_info->tid;

	/*
	 * Setting it initialization statically here to avoid
	 * a memset call jump with qdf_mem_set call
	 */
	uint8_t cached_desc[HAL_TX_DESC_LEN_BYTES] = { 0 };

	enum cdp_sec_type sec_type = ((tx_exc_metadata &&
			tx_exc_metadata->sec_type != CDP_INVALID_SEC_TYPE) ?
			tx_exc_metadata->sec_type : vdev->sec_type);

	/* Return Buffer Manager ID */
	uint8_t bm_id = dp_tx_get_rbm_id(soc, ring_id);

	hal_ring_handle_t hal_ring_hdl = NULL;

	QDF_STATUS status = QDF_STATUS_E_RESOURCES;

	if (!dp_tx_is_desc_id_valid(soc, tx_desc->id)) {
		dp_err_rl("Invalid tx desc id:%d", tx_desc->id);
		return QDF_STATUS_E_RESOURCES;
	}

	hal_tx_desc_cached = (void *)cached_desc;

	hal_tx_desc_set_buf_addr(soc->hal_soc, hal_tx_desc_cached,
				 tx_desc->dma_addr, bm_id, tx_desc->id,
				 (tx_desc->flags & DP_TX_DESC_FLAG_FRAG));
	hal_tx_desc_set_lmac_id(soc->hal_soc, hal_tx_desc_cached,
				vdev->lmac_id);
	hal_tx_desc_set_search_type(soc->hal_soc, hal_tx_desc_cached,
				    vdev->search_type);
	hal_tx_desc_set_search_index(soc->hal_soc, hal_tx_desc_cached,
				     vdev->bss_ast_idx);
	hal_tx_desc_set_dscp_tid_table_id(soc->hal_soc, hal_tx_desc_cached,
					  vdev->dscp_tid_map_id);

	hal_tx_desc_set_encrypt_type(hal_tx_desc_cached,
				     sec_type_map[sec_type]);
	hal_tx_desc_set_cache_set_num(soc->hal_soc, hal_tx_desc_cached,
				      (vdev->bss_ast_hash & 0xF));

	hal_tx_desc_set_fw_metadata(hal_tx_desc_cached, fw_metadata);
	hal_tx_desc_set_buf_length(hal_tx_desc_cached, tx_desc->length);
	hal_tx_desc_set_buf_offset(hal_tx_desc_cached, tx_desc->pkt_offset);
	hal_tx_desc_set_encap_type(hal_tx_desc_cached, tx_desc->tx_encap_type);
	hal_tx_desc_set_addr_search_flags(hal_tx_desc_cached,
					  vdev->hal_desc_addr_search_flags);

	if (tx_desc->flags & DP_TX_DESC_FLAG_TO_FW)
		hal_tx_desc_set_to_fw(hal_tx_desc_cached, 1);

	/* verify checksum offload configuration*/
	if (vdev->csum_enabled &&
	    ((qdf_nbuf_get_tx_cksum(tx_desc->nbuf) ==
	      QDF_NBUF_TX_CKSUM_TCP_UDP) ||
	      qdf_nbuf_is_tso(tx_desc->nbuf)))  {
		hal_tx_desc_set_l3_checksum_en(hal_tx_desc_cached, 1);
		hal_tx_desc_set_l4_checksum_en(hal_tx_desc_cached, 1);
	}

	if (tid != HTT_TX_EXT_TID_INVALID)
		hal_tx_desc_set_hlos_tid(hal_tx_desc_cached, tid);

	if (tx_desc->flags & DP_TX_DESC_FLAG_MESH)
		hal_tx_desc_set_mesh_en(soc->hal_soc, hal_tx_desc_cached, 1);

	if (qdf_unlikely(vdev->pdev->delay_stats_flag) ||
	    qdf_unlikely(wlan_cfg_is_peer_ext_stats_enabled(soc->wlan_cfg_ctx)))
		tx_desc->timestamp = qdf_ktime_to_ms(qdf_ktime_real_get());

	dp_verbose_debug("length:%d , type = %d, dma_addr %llx, offset %d desc id %u",
			 tx_desc->length,
			 (tx_desc->flags & DP_TX_DESC_FLAG_FRAG),
			 (uint64_t)tx_desc->dma_addr, tx_desc->pkt_offset,
			 tx_desc->id);

	hal_ring_hdl = dp_tx_get_hal_ring_hdl(soc, ring_id);

	if (qdf_unlikely(dp_tx_hal_ring_access_start(soc, hal_ring_hdl))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s %d : HAL RING Access Failed -- %pK",
			 __func__, __LINE__, hal_ring_hdl);
		DP_STATS_INC(soc, tx.tcl_ring_full[ring_id], 1);
		DP_STATS_INC(vdev, tx_i.dropped.enqueue_fail, 1);
		return status;
	}

	/* Sync cached descriptor with HW */

	hal_tx_desc = hal_srng_src_get_next(soc->hal_soc, hal_ring_hdl);
	if (qdf_unlikely(!hal_tx_desc)) {
		dp_verbose_debug("TCL ring full ring_id:%d", ring_id);
		DP_STATS_INC(soc, tx.tcl_ring_full[ring_id], 1);
		DP_STATS_INC(vdev, tx_i.dropped.enqueue_fail, 1);
		goto ring_access_fail;
	}

	tx_desc->flags |= DP_TX_DESC_FLAG_QUEUED_TX;
	dp_vdev_peer_stats_update_protocol_cnt_tx(vdev, tx_desc->nbuf);
	hal_tx_desc_sync(hal_tx_desc_cached, hal_tx_desc);
	coalesce = dp_tx_attempt_coalescing(soc, vdev, tx_desc, tid);
	DP_STATS_INC_PKT(vdev, tx_i.processed, 1, tx_desc->length);
	dp_tx_update_stats(soc, tx_desc->nbuf);
	status = QDF_STATUS_SUCCESS;

	dp_tx_hw_desc_update_evt((uint8_t *)hal_tx_desc_cached,
				 hal_ring_hdl, soc);

ring_access_fail:
	dp_tx_ring_access_end_wrapper(soc, hal_ring_hdl, coalesce);

	return status;
}

QDF_STATUS dp_tx_desc_pool_init_li(struct dp_soc *soc,
				   uint16_t pool_desc_num,
				   uint8_t pool_id)
{
	uint32_t id, count, page_id, offset, pool_id_32;
	struct dp_tx_desc_s *tx_desc_elem;
	struct dp_tx_desc_pool_s *tx_desc_pool;
	uint16_t num_desc_per_page;

	tx_desc_pool = &soc->tx_desc[pool_id];
	tx_desc_elem = tx_desc_pool->freelist;
	count = 0;
	pool_id_32 = (uint32_t)pool_id;
	num_desc_per_page = tx_desc_pool->desc_pages.num_element_per_page;
	while (tx_desc_elem) {
		page_id = count / num_desc_per_page;
		offset = count % num_desc_per_page;
		id = ((pool_id_32 << DP_TX_DESC_ID_POOL_OS) |
			(page_id << DP_TX_DESC_ID_PAGE_OS) | offset);

		tx_desc_elem->id = id;
		tx_desc_elem->pool_id = pool_id;
		tx_desc_elem = tx_desc_elem->next;
		count++;
	}

	return QDF_STATUS_SUCCESS;
}

void dp_tx_desc_pool_deinit_li(struct dp_soc *soc,
			       struct dp_tx_desc_pool_s *tx_desc_pool,
			       uint8_t pool_id)
{
}
