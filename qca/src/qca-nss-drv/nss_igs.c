/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#include "nss_tx_rx_common.h"
#include "nss_igs_stats.h"

/*
 * nss_igs_verify_if_num()
 *	Verify interface number passed to us.
 */
bool nss_igs_verify_if_num(uint32_t if_num)
{
	enum nss_dynamic_interface_type if_type;

	if_type = nss_dynamic_interface_get_type(nss_igs_get_context(), if_num);

	if (if_type == NSS_DYNAMIC_INTERFACE_TYPE_IGS) {
		return true;
	}
	return false;
}
EXPORT_SYMBOL(nss_igs_verify_if_num);

/*
 * nss_igs_handler()
 *	Handle NSS -> HLOS messages for igs device
 */
static void nss_igs_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm,
		 void *app_data)
{
	void *ctx;
	nss_igs_msg_callback_t cb;

	NSS_VERIFY_CTX_MAGIC(nss_ctx);
	BUG_ON(!nss_igs_verify_if_num(ncm->interface));

	/*
	 * Is this a valid request/response packet?
	 */
	if (ncm->type >= NSS_IGS_MSG_MAX) {
		nss_warning("%p: received invalid message %d for IGS interface", nss_ctx, ncm->type);
		return;
	}

	if (nss_cmn_get_msg_len(ncm) > sizeof(struct nss_igs_msg)) {
		nss_warning("%p: tx request for another interface: %d", nss_ctx, ncm->interface);
		return;
	}

	switch (ncm->type) {
	case NSS_IGS_MSG_SYNC_STATS:
		/*
		 * Debug stats embedded in stats msg.
		 */
		nss_igs_stats_sync(nss_ctx, ncm, ncm->interface);
		break;
	}

	/*
	 * Update the callback and app_data for NOTIFY messages
	 */
	if (ncm->response == NSS_CMN_RESPONSE_NOTIFY) {
		ncm->cb = (nss_ptr_t)nss_top_main.if_rx_msg_callback[ncm->interface];
		ncm->app_data = (nss_ptr_t)app_data;
	}

	/*
	 * callback
	 */
	cb = (nss_igs_msg_callback_t)ncm->cb;
	ctx = (void *)ncm->app_data;

	/*
	 * call igs callback
	 */
	if (!cb) {
		nss_warning("%p: No callback for igs interface %d",
			    nss_ctx, ncm->interface);
		return;
	}

	cb(ctx, ncm);
}

/*
 * nss_igs_unregister_if()
 *	Un-registers IGS interface from the NSS firmware.
 */
void nss_igs_unregister_if(uint32_t if_num)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.igs_handler_id];

	nss_assert(nss_ctx);
	nss_assert(nss_igs_verify_if_num(if_num));

	nss_core_unregister_subsys_dp(nss_ctx, if_num);

	nss_top_main.if_rx_msg_callback[if_num] = NULL;

	nss_core_unregister_handler(nss_ctx, if_num);

	nss_igs_stats_reset(if_num);
}
EXPORT_SYMBOL(nss_igs_unregister_if);

/*
 * nss_igs_register_if()
 *	Registers the IGS interface with NSS FW.
 */
struct nss_ctx_instance *nss_igs_register_if(uint32_t if_num, uint32_t type,
		 nss_igs_msg_callback_t event_callback, struct net_device *netdev,
		 uint32_t features)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.igs_handler_id];

	nss_assert(nss_ctx);
	nss_assert(nss_igs_verify_if_num(if_num));

	nss_core_register_subsys_dp(nss_ctx, if_num, NULL, 0, netdev, netdev, features);
	nss_core_set_subsys_dp_type(nss_ctx, netdev, if_num, type);

	nss_top_main.if_rx_msg_callback[if_num] = event_callback;

	nss_core_register_handler(nss_ctx, if_num, nss_igs_handler, netdev);
	nss_igs_stats_dentry_create();

	nss_igs_stats_init(if_num, netdev);

	return nss_ctx;
}
EXPORT_SYMBOL(nss_igs_register_if);

/*
 * nss_igs_get_context()
 *	Get the IGS context.
 */
struct nss_ctx_instance *nss_igs_get_context()
{
	return (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.igs_handler_id];
}
EXPORT_SYMBOL(nss_igs_get_context);
