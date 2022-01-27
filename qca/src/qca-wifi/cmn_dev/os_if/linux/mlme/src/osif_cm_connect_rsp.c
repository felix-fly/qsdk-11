/*
 * Copyright (c) 2012-2015, 2020-2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: osif_cm_connect_rsp.c
 *
 * This file maintains definitaions of connect response apis.
 */

#include <linux/version.h>
#include <linux/nl80211.h>
#include <net/cfg80211.h>
#include "wlan_osif_priv.h"
#include "osif_cm_rsp.h"
#include "osif_cm_util.h"
#include "wlan_cfg80211.h"
#include "wlan_cfg80211_scan.h"

#ifdef CONN_MGR_ADV_FEATURE
void osif_cm_get_assoc_req_ie_data(struct element_info *assoc_req,
				   size_t *ie_data_len,
				   const uint8_t **ie_data_ptr)
{
	/* Validate IE and length */
	if (!assoc_req->len || !assoc_req->ptr ||
	    assoc_req->len <= WLAN_ASSOC_REQ_IES_OFFSET)
		return;

	*ie_data_len = assoc_req->len - WLAN_ASSOC_REQ_IES_OFFSET;
	*ie_data_ptr = assoc_req->ptr + WLAN_ASSOC_REQ_IES_OFFSET;
}

void osif_cm_get_assoc_rsp_ie_data(struct element_info *assoc_rsp,
				   size_t *ie_data_len,
				   const uint8_t **ie_data_ptr)
{
	/* Validate IE and length */
	if (!assoc_rsp->len || !assoc_rsp->ptr ||
	    assoc_rsp->len <= WLAN_ASSOC_RSP_IES_OFFSET)
		return;

	*ie_data_len = assoc_rsp->len - WLAN_ASSOC_RSP_IES_OFFSET;
	*ie_data_ptr = assoc_rsp->ptr + WLAN_ASSOC_RSP_IES_OFFSET;
}

#else

void osif_cm_get_assoc_req_ie_data(struct element_info *assoc_req,
				   size_t *ie_data_len,
				   const uint8_t **ie_data_ptr)
{
	*ie_data_len = assoc_req->len;
	*ie_data_ptr = assoc_req->ptr;
}

void osif_cm_get_assoc_rsp_ie_data(struct element_info *assoc_rsp,
				   size_t *ie_data_len,
				   const uint8_t **ie_data_ptr)
{
	*ie_data_len = assoc_rsp->len;
	*ie_data_ptr = assoc_rsp->ptr;
}

#endif

/**
 * osif_validate_connect_and_reset_src_id() - Validate connect response and
 * resets source and id
 * @osif_priv: Pointer to vdev osif priv
 * @rsp: Connection manager connect response
 *
 * This function validates connect response and if the connect
 * response is valid, resets the source and id of the command
 *
 * Context: Any context. Takes and releases cmd id spinlock.
 * Return: QDF_STATUS
 */
static QDF_STATUS
osif_validate_connect_and_reset_src_id(struct vdev_osif_priv *osif_priv,
				       struct wlan_cm_connect_resp *rsp)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/*
	 * Do not send to kernel if last osif cookie doesnt match or
	 * or source is CM_OSIF_CFG_CONNECT with success status.
	 * If cookie matches reset the cookie and source.
	 */
	qdf_spinlock_acquire(&osif_priv->cm_info.cmd_id_lock);
	if (rsp->cm_id != osif_priv->cm_info.last_id ||
	    (osif_priv->cm_info.last_source == CM_OSIF_CFG_CONNECT &&
	     QDF_IS_STATUS_SUCCESS(rsp->connect_status))) {
		osif_debug("Ignore as cm_id(0x%x)/src(%d) != cm_id(0x%x)/src(%d) OR source is CFG connect with status %d",
			   rsp->cm_id, CM_OSIF_CONNECT,
			   osif_priv->cm_info.last_id,
			   osif_priv->cm_info.last_source,
			   rsp->connect_status);
		status = QDF_STATUS_E_INVAL;
		goto rel_lock;
	}

	osif_cm_reset_id_and_src_no_lock(osif_priv);
rel_lock:
	qdf_spinlock_release(&osif_priv->cm_info.cmd_id_lock);

	return status;
}

static enum ieee80211_statuscode
osif_get_statuscode(enum wlan_status_code status)
{
	return (enum ieee80211_statuscode)status;
}

static enum ieee80211_statuscode
osif_get_connect_status_code(struct wlan_cm_connect_resp *rsp)
{
	enum ieee80211_statuscode status = WLAN_STATUS_SUCCESS;

	if (QDF_IS_STATUS_ERROR(rsp->connect_status)) {
		if (rsp->status_code)
			status = osif_get_statuscode(rsp->status_code);
		else
			status = WLAN_STATUS_UNSPECIFIED_FAILURE;
	}

	return status;
}

/**
 * osif_convert_timeout_reason() - Convert to kernel specific enum
 * @timeout_reason: reason for connect timeout
 *
 * This function is used to convert host timeout
 * reason enum to kernel specific enum.
 *
 * Context: Any context.
 * Return: nl timeout enum
 */

static enum nl80211_timeout_reason
osif_convert_timeout_reason(enum wlan_cm_connect_fail_reason reason)
{
	switch (reason) {
	case CM_JOIN_TIMEOUT:
		return NL80211_TIMEOUT_SCAN;
	case CM_AUTH_TIMEOUT:
		return NL80211_TIMEOUT_AUTH;
	case CM_ASSOC_TIMEOUT:
		return NL80211_TIMEOUT_ASSOC;
	default:
		return NL80211_TIMEOUT_UNSPECIFIED;
	}
}

#if defined CFG80211_CONNECT_BSS || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))

#if defined CFG80211_CONNECT_TIMEOUT_REASON_CODE || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
/**
 * osif_connect_timeout() - API to send connection timeout reason
 * @dev: network device
 * @bssid: bssid to which we want to associate
 * @reason: reason for connect timeout
 *
 * This API is used to send connection timeout reason to supplicant
 *
 * Context: Any context.
 * Return: Void
 */
static void
osif_connect_timeout(struct net_device *dev, const u8 *bssid,
		     enum wlan_cm_connect_fail_reason reason)
{
	enum nl80211_timeout_reason nl_timeout_reason;

	nl_timeout_reason = osif_convert_timeout_reason(reason);

	osif_debug("nl_timeout_reason %d", nl_timeout_reason);

	cfg80211_connect_timeout(dev, bssid, NULL, 0, GFP_KERNEL,
				 nl_timeout_reason);
}

/**
 * __osif_connect_bss() - API to send connection status to supplicant
 * @dev: network device
 * @bss: bss info
 * @connect_rsp: Connection manager connect response
 *
 * Context: Any context.
 * Return: void
 */
static void __osif_connect_bss(struct net_device *dev,
			       struct cfg80211_bss *bss,
			       struct wlan_cm_connect_resp *rsp,
			       enum ieee80211_statuscode status)
{
	enum nl80211_timeout_reason nl_timeout_reason;
	size_t req_len = 0;
	const uint8_t *req_ptr = NULL;
	size_t rsp_len = 0;
	const uint8_t *rsp_ptr = NULL;

	nl_timeout_reason = osif_convert_timeout_reason(rsp->reason);

	osif_debug("nl_timeout_reason %d", nl_timeout_reason);

	osif_cm_get_assoc_req_ie_data(&rsp->connect_ies.assoc_req,
				      &req_len, &req_ptr);
	osif_cm_get_assoc_rsp_ie_data(&rsp->connect_ies.assoc_rsp,
				      &rsp_len, &rsp_ptr);

	cfg80211_connect_bss(dev, rsp->bssid.bytes, bss,
			     req_ptr, req_len, rsp_ptr, rsp_len, status,
			     GFP_KERNEL, nl_timeout_reason);
}
#else /* CFG80211_CONNECT_TIMEOUT_REASON_CODE */

#if defined CFG80211_CONNECT_TIMEOUT || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
static void osif_connect_timeout(
			struct net_device *dev,
			const u8 *bssid,
			enum wlan_cm_connect_fail_reason reason)
{
	cfg80211_connect_timeout(dev, bssid, NULL, 0, GFP_KERNEL);
}
#endif

static void __osif_connect_bss(struct net_device *dev,
			       struct cfg80211_bss *bss,
			       struct wlan_cm_connect_resp *rsp,
			       ieee80211_statuscode status)
{
	size_t req_len = 0;
	const uint8_t *req_ptr = NULL;
	size_t rsp_len = 0;
	const uint8_t *rsp_ptr = NULL;

	osif_cm_get_assoc_req_ie_data(&rsp->connect_ies.assoc_req,
				      &req_len, &req_ptr);
	osif_cm_get_assoc_rsp_ie_data(&rsp->connect_ies.assoc_rsp,
				      &rsp_len, &rsp_ptr);

	cfg80211_connect_bss(dev, rsp->bssid.bytes, bss,
			     req_ptr, req_len, rsp_ptr, rsp_len,
			     status, GFP_KERNEL);
}
#endif /* CFG80211_CONNECT_TIMEOUT_REASON_CODE */

/**
 * osif_connect_bss() - API to send connection status to supplicant
 * @dev: network device
 * @bss: bss info
 * @connect_rsp: Connection manager connect response
 *
 * The API is a wrapper to send connection status to supplicant
 *
 * Context: Any context.
 * Return: Void
 */
#if defined CFG80211_CONNECT_TIMEOUT || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
static void osif_connect_bss(struct net_device *dev, struct cfg80211_bss *bss,
			     struct wlan_cm_connect_resp *rsp)
{
	enum ieee80211_statuscode status = WLAN_STATUS_SUCCESS;

	osif_enter_dev(dev);

	if (rsp->reason == CM_JOIN_TIMEOUT ||
	    rsp->reason == CM_AUTH_TIMEOUT ||
	    rsp->reason == CM_ASSOC_TIMEOUT) {
		osif_connect_timeout(dev, rsp->bssid.bytes,
				     rsp->reason);
	} else {
		status = osif_get_connect_status_code(rsp);

		__osif_connect_bss(dev, bss, rsp, status);
	}
}
#else /* CFG80211_CONNECT_TIMEOUT */
static void osif_connect_bss(struct net_device *dev, struct cfg80211_bss *bss,
			     struct wlan_cm_connect_resp *rsp)
{
	enum ieee80211_statuscode status;

	osif_enter_dev(dev);

	status = osif_get_connect_status_code(rsp);
	__osif_connect_bss(dev, bss, rsp, status);
}
#endif /* CFG80211_CONNECT_TIMEOUT */

#if defined(WLAN_FEATURE_FILS_SK)

#if (defined(CFG80211_CONNECT_DONE) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))) && \
	(LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))

#if defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
/**
 * osif_populate_fils_params() - Populate FILS keys to connect response
 * @conn_rsp_params: connect response to supplicant
 * @fils_ie: Fils ie information from connection manager
 *
 * Context: Any context.
 * Return: None
 */
static void
osif_populate_fils_params(struct cfg80211_connect_resp_params *rsp_params,
			  struct fils_connect_rsp_params *fils_ie)
{
	/*  Increment seq number to be used for next FILS */
	rsp_params->fils_erp_next_seq_num = fils_ie->fils_seq_num + 1;
	rsp_params->update_erp_next_seq_num = true;
	rsp_params->fils_kek = fils_ie->kek;
	rsp_params->fils_kek_len = fils_ie->kek_len;
	rsp_params->pmk = fils_ie->fils_pmk;
	rsp_params->pmk_len = fils_ie->fils_pmk_len;
	rsp_params->pmkid = fils_ie->fils_pmkid;
	osif_debug("erp_next_seq_num:%d", rsp_params->fils_erp_next_seq_num);
}
#else /* CFG80211_FILS_SK_OFFLOAD_SUPPORT */
static inline void
osif_populate_fils_params(struct cfg80211_connect_resp_params *rsp_params,
			  struct fils_connect_rsp_params *fils_ie)

{ }
#endif /* CFG80211_FILS_SK_OFFLOAD_SUPPORT */

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
/**
 * osif_populate_fils_params() - Populate FILS keys to connect response
 * @conn_rsp_params: connect response to supplicant
 * @fils_ie: Fils ie information from connection manager
 *
 * Context: Any context.
 * Return: None
 */
static void
osif_populate_fils_params(struct cfg80211_connect_resp_params *rsp_params,
			  struct fils_connect_rsp_params *fils_ie)

{
	/* Increament seq number to be used for next FILS */
	rsp_params->fils.erp_next_seq_num = fils_ie->fils_seq_num + 1;
	rsp_params->fils.update_erp_next_seq_num = true;
	rsp_params->fils.kek = fils_ie->kek;
	rsp_params->fils.kek_len = fils_ie->kek_len;
	rsp_params->fils.pmk = fils_ie->fils_pmk;
	rsp_params->fils.pmk_len = fils_ie->fils_pmk_len;
	rsp_params->fils.pmkid = fils_ie->fils_pmkid;
	osif_debug("erp_next_seq_num:%d", rsp_params->fils.erp_next_seq_num);
}
#endif /* CFG80211_CONNECT_DONE */

#if defined(CFG80211_CONNECT_DONE) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))

/**
 * osif_connect_done() - Wrapper API to call cfg80211_connect_done
 * @dev: network device
 * @bss: bss info
 * @connect_rsp: Connection manager connect response
 * @vdev: pointer to vdev
 *
 * This API is used as wrapper to send FILS key/sequence number
 * params etc. to supplicant in case of FILS connection
 *
 * Context: Any context.
 * Return: None
 */
static void osif_connect_done(struct net_device *dev, struct cfg80211_bss *bss,
			      struct wlan_cm_connect_resp *rsp,
			      struct wlan_objmgr_vdev *vdev)
{
	struct cfg80211_connect_resp_params conn_rsp_params;
	enum ieee80211_statuscode status;

	osif_enter_dev(dev);

	status = osif_get_connect_status_code(rsp);
	qdf_mem_zero(&conn_rsp_params, sizeof(conn_rsp_params));

	if (!rsp->connect_ies.fils_ie) {
		conn_rsp_params.status = WLAN_STATUS_UNSPECIFIED_FAILURE;
	} else {
		conn_rsp_params.status = status;
		conn_rsp_params.bssid = rsp->bssid.bytes;
		conn_rsp_params.timeout_reason =
			osif_convert_timeout_reason(rsp->reason);
		osif_cm_get_assoc_req_ie_data(&rsp->connect_ies.assoc_req,
					      &conn_rsp_params.req_ie_len,
					      &conn_rsp_params.req_ie);
		osif_cm_get_assoc_rsp_ie_data(&rsp->connect_ies.assoc_rsp,
					      &conn_rsp_params.resp_ie_len,
					      &conn_rsp_params.resp_ie);
		conn_rsp_params.bss = bss;
		osif_populate_fils_params(&conn_rsp_params,
					  rsp->connect_ies.fils_ie);
		osif_cm_save_gtk(vdev, rsp);
	}

	osif_debug("Connect resp status  %d", conn_rsp_params.status);
	cfg80211_connect_done(dev, &conn_rsp_params, GFP_KERNEL);
	if (rsp->connect_ies.fils_ie && rsp->connect_ies.fils_ie->hlp_data_len)
		osif_cm_set_hlp_data(dev, vdev, rsp);
}
#else /* CFG80211_CONNECT_DONE */
static inline void
osif_connect_done(struct net_device *dev, struct cfg80211_bss *bss,
		  struct wlan_cm_connect_resp *rsp,
		  struct wlan_objmgr_vdev *vdev)
{ }
#endif /* CFG80211_CONNECT_DONE */
#endif /* WLAN_FEATURE_FILS_SK */

#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_CONNECT_DONE) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)))
/**
 * osif_fils_update_connect_results() - API to send fils connection status to
 * supplicant.
 * @dev: network device
 * @bss: bss info
 * @connect_rsp: Connection manager connect response
 * @vdev: pointer to vdev
 *
 * The API is a wrapper to send connection status to supplicant
 *
 * Context: Any context.
 * Return: 0 if success else failure
 */
static int osif_update_connect_results(struct net_device *dev,
				       struct cfg80211_bss *bss,
				       struct wlan_cm_connect_resp *rsp,
				       struct wlan_objmgr_vdev *vdev)
{
	if (!rsp->is_fils_connection) {
		osif_debug("fils IE is NULL");
		return -EINVAL;
	}
	osif_connect_done(dev, bss, rsp, vdev);

	return 0;
}
#else /* WLAN_FEATURE_FILS_SK && CFG80211_CONNECT_DONE */

static inline int osif_update_connect_results(struct net_device *dev,
					      struct cfg80211_bss *bss,
					      struct wlan_cm_connect_resp *rsp,
					      struct wlan_objmgr_vdev *vdev)
{
	return -EINVAL;
}
#endif /* WLAN_FEATURE_FILS_SK && CFG80211_CONNECT_DONE */

static void osif_indcate_connect_results(struct wlan_objmgr_vdev *vdev,
					 struct vdev_osif_priv *osif_priv,
					 struct wlan_cm_connect_resp *rsp)
{
	struct cfg80211_bss *bss = NULL;
	struct ieee80211_channel *chan;

	if (QDF_IS_STATUS_SUCCESS(rsp->connect_status)) {
		chan = ieee80211_get_channel(osif_priv->wdev->wiphy,
					     rsp->freq);
		bss = wlan_cfg80211_get_bss(osif_priv->wdev->wiphy, chan,
					    rsp->bssid.bytes,
					    rsp->ssid.ssid,
					    rsp->ssid.length);
	}

	if (osif_update_connect_results(osif_priv->wdev->netdev, bss,
					rsp, vdev))
		osif_connect_bss(osif_priv->wdev->netdev, bss, rsp);
}
#else  /* CFG80211_CONNECT_BSS */
static void osif_indcate_connect_results(struct wlan_objmgr_vdev *vdev,
					 struct vdev_osif_priv *osif_priv,
					 struct wlan_cm_connect_resp *rsp)
{
	enum ieee80211_statuscode status;
	size_t req_len = 0;
	const uint8_t *req_ptr = NULL;
	size_t rsp_len = 0;
	const uint8_t *rsp_ptr = NULL;

	status = osif_get_connect_status_code(rsp);
	osif_cm_get_assoc_req_ie_data(&rsp->connect_ies.assoc_req,
				      &req_len, &req_ptr);
	osif_cm_get_assoc_rsp_ie_data(&rsp->connect_ies.assoc_rsp,
				      &rsp_len, &rsp_ptr);
	cfg80211_connect_result(osif_priv->wdev->netdev,
				rsp->bssid.bytes, req_ptr, req_len,
				rsp_ptr, rsp_len, status, GFP_KERNEL);
}
#endif /* CFG80211_CONNECT_BSS */

#ifdef CONN_MGR_ADV_FEATURE
static inline
bool osif_cm_is_unlink_bss_required(struct wlan_cm_connect_resp *rsp)
{
	if (QDF_IS_STATUS_SUCCESS(rsp->connect_status))
		return false;

	if (rsp->reason == CM_NO_CANDIDATE_FOUND ||
	    rsp->reason == CM_JOIN_TIMEOUT ||
	    rsp->reason == CM_AUTH_TIMEOUT ||
	    rsp->reason == CM_ASSOC_TIMEOUT)
		return true;

	return false;
}
static inline void osif_check_and_unlink_bss(struct wlan_objmgr_vdev *vdev,
					     struct vdev_osif_priv *osif_priv,
					     struct wlan_cm_connect_resp *rsp)
{
	if (osif_cm_is_unlink_bss_required(rsp))
		osif_cm_unlink_bss(vdev, osif_priv, &rsp->bssid, rsp->ssid.ssid,
				   rsp->ssid.length);
}
#else
static inline void osif_check_and_unlink_bss(struct wlan_objmgr_vdev *vdev,
					     struct vdev_osif_priv *osif_priv,
					     struct wlan_cm_connect_resp *rsp)
{}
#endif

QDF_STATUS osif_connect_handler(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_resp *rsp)
{
	struct vdev_osif_priv *osif_priv  = wlan_vdev_get_ospriv(vdev);
	QDF_STATUS status;

	osif_nofl_info("%s(vdevid-%d): " QDF_MAC_ADDR_FMT " Connect with " QDF_MAC_ADDR_FMT " SSID \"%.*s\" is %s cm_id 0x%x cm_reason %d status_code %d is_reassoc %d",
		       osif_priv->wdev->netdev->name, rsp->vdev_id,
		       QDF_MAC_ADDR_REF(wlan_vdev_mlme_get_macaddr(vdev)),
		       QDF_MAC_ADDR_REF(rsp->bssid.bytes),
		       rsp->ssid.length, rsp->ssid.ssid,
		       rsp->connect_status ? "FAILURE" : "SUCCESS", rsp->cm_id,
		       rsp->reason, rsp->status_code, rsp->is_reassoc);

	osif_check_and_unlink_bss(vdev, osif_priv, rsp);

	status = osif_validate_connect_and_reset_src_id(osif_priv, rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_cm_connect_comp_ind(vdev, rsp, OSIF_NOT_HANDLED);
		return status;
	}

	osif_cm_connect_comp_ind(vdev, rsp, OSIF_PRE_USERSPACE_UPDATE);
	if (rsp->is_reassoc)
		osif_indicate_reassoc_results(vdev, osif_priv, rsp);
	else
		osif_indcate_connect_results(vdev, osif_priv, rsp);
	osif_cm_connect_comp_ind(vdev, rsp, OSIF_POST_USERSPACE_UPDATE);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS osif_failed_candidate_handler(struct wlan_objmgr_vdev *vdev,
					 struct wlan_cm_connect_resp *rsp)
{
	struct vdev_osif_priv *osif_priv  = wlan_vdev_get_ospriv(vdev);

	osif_nofl_info("%s(vdevid-%d): " QDF_MAC_ADDR_FMT " Connect with " QDF_MAC_ADDR_FMT " SSID \"%.*s\" FAILED cm_id 0x%x cm_reason %d reason_code %d",
		       osif_priv->wdev->netdev->name, rsp->vdev_id,
		       QDF_MAC_ADDR_REF(wlan_vdev_mlme_get_macaddr(vdev)),
		       QDF_MAC_ADDR_REF(rsp->bssid.bytes),
		       rsp->ssid.length, rsp->ssid.ssid, rsp->cm_id,
		       rsp->reason, rsp->status_code);

	osif_check_and_unlink_bss(vdev, osif_priv, rsp);

	return QDF_STATUS_SUCCESS;
}
