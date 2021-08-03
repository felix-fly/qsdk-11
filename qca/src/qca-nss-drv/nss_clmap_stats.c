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
#include "nss_clmap_stats.h"

DEFINE_SPINLOCK(nss_clmap_stats_lock);

struct nss_clmap_stats *stats_db[NSS_CLMAP_MAX_DEBUG_INTERFACES] = {NULL};

/*
 * nss_clmap_interface_type_str
 * 	Clmap interface type string.
 */
static char *nss_clmap_interface_type_str[NSS_CLMAP_INTERFACE_TYPE_MAX] = {
	"Upstream",
	"Downstream"
};

/*
 * nss_clmap_stats_str
 *	Clmap statistics strings for nss tunnel stats
 */
static char *nss_clmap_stats_str[NSS_CLMAP_INTERFACE_STATS_MAX] = {
	"rx_pkts",
	"rx_bytes",
	"tx_pkts",
	"tx_bytes",
	"rx_queue_0_dropped",
	"rx_queue_1_dropped",
	"rx_queue_2_dropped",
	"rx_queue_3_dropped",
	"MAC DB look up failed",
	"Invalid packet count",
	"Headroom drop",
	"Next node queue full drop",
	"Pbuf alloc failed drop",
	"Linear failed drop",
	"Shared packet count",
	"Ethernet frame error"
};

/*
 * nss_clmap_stats_session_register
 * 	Register debug statistic for clmap session.
 */
bool nss_clmap_stats_session_register(uint32_t if_num, uint32_t if_type, struct net_device *netdev)
{
	uint32_t i;
	bool stats_status = false;

	spin_lock_bh(&nss_clmap_stats_lock);
	for (i = 0; i < NSS_CLMAP_MAX_DEBUG_INTERFACES; i++) {
		if (!stats_db[i]) {
			stats_db[i] = (struct nss_clmap_stats *)kzalloc(sizeof(struct nss_clmap_stats), GFP_KERNEL);
			if (!stats_db[i]) {
				nss_warning("%p: could not allocate memory for statistics database for interface id: %d\n", netdev, if_num);
				break;
			}
			stats_db[i]->valid = true;
			stats_db[i]->nss_if_num = if_num;
			stats_db[i]->nss_if_type = if_type;
			stats_db[i]->if_index = netdev->ifindex;
			stats_status = true;
			break;
		}
	}
	spin_unlock_bh(&nss_clmap_stats_lock);
	return stats_status;
}

/*
 * nss_clmap_stats_session_unregister
 * 	Unregister debug statistic for clmap session.
 */
void nss_clmap_stats_session_unregister(uint32_t if_num)
{
	uint32_t i;

	spin_lock_bh(&nss_clmap_stats_lock);
	for (i = 0; i < NSS_CLMAP_MAX_DEBUG_INTERFACES; i++) {
		if (stats_db[i] && (stats_db[i]->nss_if_num == if_num)) {
			kfree(stats_db[i]);
			stats_db[i] = NULL;
			break;
		}
	}
	spin_unlock_bh(&nss_clmap_stats_lock);
}

/*
 * nss_clmap_get_debug_stats()
 *	Get clmap debug statistics.
 */
static void nss_clmap_get_debug_stats(struct nss_clmap_stats *stats)
{
	uint32_t i;

	spin_lock_bh(&nss_clmap_stats_lock);
	for (i = 0; i < NSS_CLMAP_MAX_DEBUG_INTERFACES; i++) {
		if (stats_db[i]) {
			memcpy(stats, stats_db[i], sizeof(struct nss_clmap_stats));
			stats++;
		}
	}
	spin_unlock_bh(&nss_clmap_stats_lock);
}

/*
 * nss_clmap_stats_read()
 *	Read clmap statistics
 */
static ssize_t nss_clmap_stats_read(struct file *fp, char __user *ubuf,
					size_t sz, loff_t *ppos)
{
	uint32_t max_output_lines = 2 + (NSS_CLMAP_INTERFACE_STATS_MAX * NSS_CLMAP_MAX_DEBUG_INTERFACES + 2) + 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	struct net_device *dev;
	uint32_t id, i;
	struct nss_clmap_stats *clmap_stats = NULL;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(!lbuf)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	/*
	 * Allocate statistics memory only for all interfaces.
	 */
	clmap_stats = kzalloc((NSS_CLMAP_MAX_DEBUG_INTERFACES * sizeof(struct nss_clmap_stats)), GFP_KERNEL);
	if (unlikely(!clmap_stats)) {
		nss_warning("Could not allocate memory for populating clmap statistics\n");
		kfree(lbuf);
		return 0;
	}

	/*
	 * Get clmap statistics.
	 */
	nss_clmap_get_debug_stats(clmap_stats);
	size_wr = scnprintf(lbuf + size_wr, size_al - size_wr,
			"\n clmap Interface statistics start:\n\n");
	for (id = 0; id < NSS_CLMAP_MAX_DEBUG_INTERFACES; id++) {
		struct nss_clmap_stats *clmsp = clmap_stats + id;

		if (!(clmsp->valid)) {
			continue;
		}

		dev = dev_get_by_index(&init_net, clmsp->if_index);
		if (unlikely(!dev)) {
			nss_warning("No netdev available for nss interface id:%d\n", clmsp->nss_if_type);
			continue;
		}

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%d. nss interface id=%d, interface type=%s, netdevice=%s\n", id,
				clmsp->nss_if_num, nss_clmap_interface_type_str[clmsp->nss_if_type], dev->name);
		dev_put(dev);

		for (i = 0; i < NSS_CLMAP_INTERFACE_STATS_MAX; i++) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"\t%s = %llu\n", nss_clmap_stats_str[i],
					clmsp->stats[i]);
		}
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
			"\n clmap Interface statistics end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(clmap_stats);
	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_clmap_stats_sync()
 *	Sync function for clmap statistics
 */
void nss_clmap_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_clmap_stats_msg *stats_msg, uint32_t if_num)
{
	uint32_t i;
	struct nss_clmap_stats *s = NULL;

	NSS_VERIFY_CTX_MAGIC(nss_ctx);

	spin_lock_bh(&nss_clmap_stats_lock);
	for (i = 0; i < NSS_CLMAP_MAX_DEBUG_INTERFACES; i++) {
		if (stats_db[i] && (stats_db[i]->nss_if_num == if_num)) {
			s = stats_db[i];
			break;
		}
	}

	if (!s) {
		spin_unlock_bh(&nss_clmap_stats_lock);
		nss_warning("%p: Interface not found: %u", nss_ctx, if_num);
		return;
	}

        s->stats[NSS_CLMAP_INTERFACE_STATS_RX_PKTS] += stats_msg->node_stats.rx_packets;
        s->stats[NSS_CLMAP_INTERFACE_STATS_RX_BYTES] += stats_msg->node_stats.rx_bytes;
        s->stats[NSS_CLMAP_INTERFACE_STATS_TX_PKTS] += stats_msg->node_stats.tx_packets;
        s->stats[NSS_CLMAP_INTERFACE_STATS_TX_BYTES] += stats_msg->node_stats.tx_bytes;

	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		s->stats[NSS_CLMAP_INTERFACE_STATS_RX_QUEUE_0_DROPPED + i] += stats_msg->node_stats.rx_dropped[i];
	}

	s->stats[NSS_CLMAP_INTERFACE_STATS_DROPPED_MACDB_LOOKUP_FAILED] += stats_msg->dropped_macdb_lookup_failed;
	s->stats[NSS_CLMAP_INTERFACE_STATS_DROPPED_INVALID_PACKET_SIZE] += stats_msg->dropped_invalid_packet_size;
	s->stats[NSS_CLMAP_INTERFACE_STATS_DROPPED_LOW_HEADROOM] += stats_msg->dropped_low_hroom;
	s->stats[NSS_CLMAP_INTERFACE_STATS_DROPPED_NEXT_NODE_QUEUE_FULL] += stats_msg->dropped_next_node_queue_full;
	s->stats[NSS_CLMAP_INTERFACE_STATS_DROPPED_PBUF_ALLOC_FAILED] += stats_msg->dropped_pbuf_alloc_failed;
	s->stats[NSS_CLMAP_INTERFACE_STATS_DROPPED_LINEAR_FAILED] += stats_msg->dropped_linear_failed;
	s->stats[NSS_CLMAP_INTERFACE_STATS_SHARED_PACKET_CNT] += stats_msg->shared_packet_count;
	s->stats[NSS_CLMAP_INTERFACE_STATS_ETHERNET_FRAME_ERROR] += stats_msg->ethernet_frame_error;
	spin_unlock_bh(&nss_clmap_stats_lock);
}

/*
 * nss_clmap_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(clmap)

/*
 * nss_clmap_stats_dentry_create()
 *	Create client map statistics debug entry.
 */
void nss_clmap_stats_dentry_create(void)
{
	nss_stats_create_dentry("clmap", &nss_clmap_stats_ops);
}
