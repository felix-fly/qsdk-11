/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */

/* nss_ipsec_klips.c
 *	NSS IPsec offload glue for Openswan/KLIPS
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/memory.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/if.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <net/esp.h>
#include <net/protocol.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>

#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_cfi_if.h>
#include <nss_ipsecmgr.h>
#include <ecm_interface_ipsec.h>
#include <ecm_notifier.h>
#if defined(NSS_L2TPV2_ENABLED)
#include <nss_l2tpmgr.h>
#endif
#include "nss_ipsec_klips.h"

#define NSS_IPSEC_KLIPS_BASE_NAME "ipsec"
#define NSS_IPSEC_KLIPS_TUNNEL_MAX 8
#define NSS_IPSEC_KLIPS_SES_MASK 0xffff
#define NSS_IPSEC_KLIPS_FLAG_NATT 0x00000001
#define NSS_IPSEC_KLIPS_FLAG_TRANSPORT_MODE 0x00000002
#define NSS_IPSEC_KLIPS_SKB_CB_MAGIC 0xAAAB

/*
 * This is used by KLIPS for communicate the device along with the
 * packet. We need this to derive the mapping of the incoming flow
 * to the IPsec tunnel
 */
struct nss_ipsec_klips_skb_cb {
	struct net_device *hlos_dev;
	uint32_t flags;
	uint16_t replay_win;
	uint16_t magic;
};

/*
 * Per tunnel object created w.r.t the HLOS IPsec stack
 */
struct nss_ipsec_klips_sa {
	struct list_head list;
	struct nss_ipsecmgr_sa_tuple outer;
	uint32_t sid;
	bool ecm_accel_outer;
};

/*
 * CFI IPsec netdevice  mapping between HLOS devices and NSS devices
 * Essentially various HLOS IPsec devices will be used as indexes to
 * map into the NSS devices. For example
 * "ipsec0" --> "ipsectun0"
 * "ipsec1" --> "ipsectun1"
 *
 * Where the numeric suffix of "ipsec0", "ipsec1" is used to index into
 * the table
 */
struct nss_ipsec_klips_tun {
	struct net_device *klips_dev;
	struct net_device *nss_dev;
	struct list_head sa_list;
};

/*
 * NSS IPsec CFI tunnel map table
 */
struct nss_ipsec_klips_tun_map {
	uint16_t max;
	uint16_t used;
	rwlock_t lock;
	struct nss_ipsec_klips_tun *tbl;
};

static int tunnel_max = NSS_IPSEC_KLIPS_TUNNEL_MAX;
module_param(tunnel_max, int, 0644);
MODULE_PARM_DESC(tunnel_max, "Maximum number of tunnels to offload");

static struct nss_ipsec_klips_tun_map tunnel_map;	/* per tunnel device table */

/*
 * Original ESP ptotocol handlers
 */
static const struct net_protocol *klips_esp_handler;
static const struct inet6_protocol *klips_esp6_handler;

static int nss_ipsec_klips_offload_esp(struct sk_buff *skb);

/*
 * IPv4 ESP handler
 */
static struct net_protocol esp_protocol = {
	.handler = nss_ipsec_klips_offload_esp,
	.no_policy = 1,
	.netns_ok  = 1,
};

/*
 * IPv6 ESP handler
 */
static struct inet6_protocol esp6_protocol = {
	.handler = nss_ipsec_klips_offload_esp,
	.flags = INET6_PROTO_NOPOLICY,
};

/*
 * nss_ipsec_klips_v6addr_ntoh()
 *	Network to host order
 */
static inline void nss_ipsec_klips_v6addr_ntoh(uint32_t *dest, uint32_t *src)
{
	dest[0] = ntohl(src[0]);
	dest[1] = ntohl(src[1]);
	dest[2] = ntohl(src[2]);
	dest[3] = ntohl(src[3]);
}

/*
 * nss_ipsec_klips_get_blk_len
 *	get ipsec algorithm specific iv and block len
 */
static int32_t nss_ipsec_klips_get_blk_len(enum nss_crypto_cmn_algo algo)
{
	switch (algo) {
	case NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA160_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES128_CBC_MD5_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES192_CBC_MD5_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES256_CBC_MD5_HMAC:
		return AES_BLOCK_SIZE;

	case NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC:
	case NSS_CRYPTO_CMN_ALGO_3DES_CBC_MD5_HMAC:
		return DES_BLOCK_SIZE;

	case NSS_CRYPTO_CMN_ALGO_NULL:
		return 0;

	default:
		nss_ipsec_klips_err("Invalid algorithm\n");
		return -1;
	}
}

/*
 * nss_ipsec_klips_get_algo
 *	get ipsec manager specfic algorithm
 */
static enum nss_ipsecmgr_algo nss_ipsec_klips_get_algo(enum nss_crypto_cmn_algo algo)
{
	switch (algo) {
	case NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC:
		return NSS_IPSECMGR_ALGO_AES_CBC_SHA1_HMAC;

	case NSS_CRYPTO_CMN_ALGO_AES128_CBC_MD5_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES192_CBC_MD5_HMAC:
	case NSS_CRYPTO_CMN_ALGO_AES256_CBC_MD5_HMAC:
		return NSS_IPSECMGR_ALGO_AES_CBC_MD5_HMAC;

	case NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC:
		return NSS_IPSECMGR_ALGO_3DES_CBC_SHA1_HMAC;

	case NSS_CRYPTO_CMN_ALGO_3DES_CBC_MD5_HMAC:
		return NSS_IPSECMGR_ALGO_3DES_CBC_MD5_HMAC;

	default:
		nss_ipsec_klips_err("Invalid algorithm\n");
		return NSS_IPSECMGR_ALGO_MAX;
	}
}

/*
 * nss_ipsec_klips_get_skb_cb()
 * 	Get pointer to skb->cb typecasted to private structure.
 */
static inline struct nss_ipsec_klips_skb_cb *nss_ipsec_klips_get_skb_cb(struct sk_buff *skb)
{
	struct nss_ipsec_klips_skb_cb *ipsec_cb;

	/*
	 * Force compilation error if structure size is bigger than the allowed size.
	 */
	BUILD_BUG_ON(sizeof(struct nss_ipsec_klips_skb_cb) > sizeof(skb->cb));

	/*
	 * Verify cb is not overwritten by anyone.
	 */
	ipsec_cb = (struct nss_ipsec_klips_skb_cb *)skb->cb;
	BUG_ON(ipsec_cb->magic != NSS_IPSEC_KLIPS_SKB_CB_MAGIC);

	return ipsec_cb;
}

/*
 * nss_ipsec_klips_get_tun()
 * 	get tunnel entry for given klips dev.
 */
static struct nss_ipsec_klips_tun *nss_ipsec_klips_get_tun(struct net_device *klips_dev)
{
	struct nss_ipsec_klips_tun *tun;
	int i;

	/*
	 * Read/write lock needs to taken by the caller since sa
	 * table is looked up here
	 */
	BUG_ON(write_can_lock(&tunnel_map.lock));

	if (!klips_dev) {
		return NULL;
	}

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_map.max; i++, tun++) {
		if (!tun->nss_dev || !tun->klips_dev)
			continue;

		if (klips_dev == tun->klips_dev)
			return tun;
	}

	return NULL;
}

/*
 * nss_ipsec_klips_get_tun_dev()
 *	Get ipsecmgr tunnel netdevice for klips netdevice
 *
 * The device returned has had a reference added and the pointer is safe until
 * the user calls dev_put to indicate they have finished with it
 */
static struct net_device *nss_ipsec_klips_get_tun_dev(struct net_device *klips_dev)
{
	struct nss_ipsec_klips_tun *tun;
	struct net_device *tun_dev;

	write_lock_bh(&tunnel_map.lock);

	tun = nss_ipsec_klips_get_tun(klips_dev);
	if (!tun) {
		write_unlock_bh(&tunnel_map.lock);
		return NULL;
	}

	tun_dev = tun->nss_dev;
	dev_hold(tun_dev);
	write_unlock_bh(&tunnel_map.lock);

	return tun_dev;
}

/*
 * nss_ipsec_klips_match_addr()
 * 	Match ip_addr with addresses associated with net_device.
 */
static bool nss_ipsec_klips_match_addr(struct net_device *dev, struct sk_buff *skb)
{
	struct inet6_dev *in6_dev;
	struct inet6_ifaddr *ifa;
	struct ipv6hdr *ip6h;
	struct iphdr *iph;
	bool match = false;

	iph = ip_hdr(skb);

	if (iph->version == IPVERSION) {
		struct in_device *in_dev;
		struct in_ifaddr *ifa;

		in_dev = in_dev_get(dev);
		if (!in_dev) {
			nss_ipsec_klips_warn("%p: Failed to find in_dev\n", dev);
			return false;
		}

		rcu_read_lock();
		for (ifa = in_dev->ifa_list; ifa && !match; ifa = ifa->ifa_next) {
			match = (ifa->ifa_local == iph->daddr);
		}

		rcu_read_unlock();

		in_dev_put(in_dev);
		return match;
	}

	ip6h = ipv6_hdr(skb);

	in6_dev = in6_dev_get(dev);
	if (!in6_dev) {
		nss_ipsec_klips_warn("%p: Failed to find in6_dev\n", dev);
		return false;
	}

	read_lock_bh(&in6_dev->lock);

	list_for_each_entry(ifa, &in6_dev->addr_list, if_list) {
		match = ipv6_addr_equal(&ifa->addr, &ip6h->daddr);
		if (match) {
			break;
		}
	}

	read_unlock_bh(&in6_dev->lock);

	in6_dev_put(in6_dev);
	return match;
}

/*
 * nss_ipsec_klips_get_tun_by_addr()
 * 	Get the tunnel entry for given ip header from tunnel map table.
 */
static struct nss_ipsec_klips_tun *nss_ipsec_klips_get_tun_by_addr(struct sk_buff *skb)
{
	struct nss_ipsec_klips_tun *tun;
	int i;

	/*
	 * Read/write lock needs to be taken by the caller since tunnel
	 * table is looked up here
	 */
	BUG_ON(write_can_lock(&tunnel_map.lock));

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_map.max; i++, tun++) {
		if (!tun->klips_dev) {
			continue;
		}

		if (nss_ipsec_klips_match_addr(tun->klips_dev, skb)) {
			return tun;
		}
	}

	return NULL;
}

/*
 * nss_ipsec_klips_get_index()
 *	given an interface name retrived the numeric suffix
 */
static int16_t nss_ipsec_klips_get_index(uint8_t *name)
{
	uint8_t *next_char;
	int16_t idx;

	if (strncmp(name, NSS_IPSEC_KLIPS_BASE_NAME, strlen(NSS_IPSEC_KLIPS_BASE_NAME)))
		return -1;

	next_char = name + strlen(NSS_IPSEC_KLIPS_BASE_NAME);
	if ((*next_char < '0') || (*next_char > '9'))
		return -1;

	for (idx = 0; (*next_char >= '0') && (*next_char <= '9'); next_char++)
		idx = (*next_char - '0') + (idx * 10);

	return idx;
}

/*
 * nss_ipsec_klips_sa_lookup()
 *	Look for an SA based on crypto index.
 */
static struct nss_ipsec_klips_sa *nss_ipsec_klips_sa_lookup(struct nss_ipsec_klips_tun *tun, uint16_t crypto_idx)
{
	struct list_head *head = &tun->sa_list;
	struct nss_ipsec_klips_sa *sa;
	struct nss_ipsec_klips_sa *tmp;

	/*
	 * Read/write lock needs to taken by the caller since sa
	 * table is looked up here
	 */
	BUG_ON(write_can_lock(&tunnel_map.lock));

	list_for_each_entry_safe(sa, tmp, head, list) {
		if (sa->sid == crypto_idx)
			return sa;
	}

	return NULL;
}

/*
 * nss_ipsec_klips_sa_flush()
 *	Flush all SA entries
 */
static void nss_ipsec_klips_sa_flush(struct nss_ipsec_klips_tun *tun, struct net_device *nss_dev)
{
	struct list_head *head = &tun->sa_list;
	struct nss_ipsec_klips_sa *sa;
	struct nss_ipsec_klips_sa *tmp;

	/*
	 * Read/write lock needs to taken by the caller since sa
	 * table is modified here
	 */
	BUG_ON(write_can_lock(&tunnel_map.lock));

	list_for_each_entry_safe(sa, tmp, head, list) {
		list_del_init(&sa->list);
		nss_ipsecmgr_sa_del(nss_dev, &sa->outer);
		kfree(sa);
	}
}

/*
 * nss_ipsec_klips_free_session()
 *	Free particular session on NSS.
 */
static int32_t nss_ipsec_klips_free_session(uint32_t crypto_sid)
{
	uint16_t crypto_idx = crypto_sid & NSS_IPSEC_KLIPS_SES_MASK;
	struct nss_ipsec_klips_tun *tun;
	struct nss_ipsec_klips_sa *sa = NULL;
	struct net_device *nss_dev;
	int i;

	/*
	 * Write lock needs to be taken here since SA table is
	 * getting modified
	 */
	write_lock_bh(&tunnel_map.lock);

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_map.max; i++, tun++) {
		if (!tun->nss_dev || !tun->klips_dev)
			continue;

		sa = nss_ipsec_klips_sa_lookup(tun, crypto_idx);
		if (sa) {
			list_del_init(&sa->list);
			break;
		}
	}

	if (!sa) {
		write_unlock_bh(&tunnel_map.lock);
		return -ENOENT;
	}

	nss_dev = tun->nss_dev;
	dev_hold(nss_dev);

	write_unlock_bh(&tunnel_map.lock);

	/*
	 * The SA is now removed from the list hence
	 * we can access it without locks
	 */
	nss_ipsecmgr_sa_del(nss_dev, &sa->outer);
	dev_put(nss_dev);

	kfree(sa);
	return 0;
}

/*
 * nss_ipsec_klips_outer2sa_tuple()
 *	Fill sa_tuple from outer header and return start of payload
 */
static void *nss_ipsec_klips_outer2sa_tuple(uint8_t *outer, bool natt, struct nss_ipsecmgr_sa_tuple *tuple,
				uint8_t *ttl, bool decap)
{
	struct ipv6hdr *ip6h = (struct ipv6hdr *)outer;
	struct iphdr *ip4h = (struct iphdr *)outer;
	struct ip_esp_hdr *esph;

	memset(tuple, 0, sizeof(*tuple));
	if (ip4h->version == IPVERSION) {
		outer += sizeof(*ip4h);

		tuple->src_ip[0] = ntohl(ip4h->saddr);
		tuple->dest_ip[0] = ntohl(ip4h->daddr);
		tuple->proto_next_hdr = ip4h->protocol;
		tuple->ip_version = IPVERSION;
		*ttl = ip4h->ttl;

		/*
		 * TODO: NAT-T ports can be programmable; Add
		 * support for loading programmed ports by user
		 */
		if (natt) {
			tuple->sport = NSS_IPSECMGR_NATT_PORT_DATA;
			tuple->dport = NSS_IPSECMGR_NATT_PORT_DATA;
			tuple->proto_next_hdr = IPPROTO_UDP;

			/*
			 * TODO: Find out why we need decap flag
			 */
			outer += decap ? sizeof(struct udphdr) : 0;
		}

		esph = (struct ip_esp_hdr *)outer;
		tuple->spi_index = ntohl(esph->spi);

		return outer + sizeof(*esph);
	}

	BUG_ON(ip6h->version != 6);
	BUG_ON(ip6h->nexthdr != IPPROTO_ESP);

	outer += sizeof(*ip6h);
	esph = (struct ip_esp_hdr *)outer;
	nss_ipsec_klips_v6addr_ntoh(tuple->src_ip, ip6h->saddr.s6_addr32);
	nss_ipsec_klips_v6addr_ntoh(tuple->dest_ip, ip6h->daddr.s6_addr32);

	tuple->spi_index = ntohl(esph->spi);
	tuple->proto_next_hdr = IPPROTO_ESP;
	*ttl = ip6h->hop_limit;
	tuple->ip_version = 6;

	return outer + sizeof(*esph);
}

/*
 * nss_ipsec_klips_outer2flow_tuple()
 *	Fill inner flow
 */
static bool nss_ipsec_klips_outer2flow_tuple(uint8_t *outer, bool natt, struct nss_ipsecmgr_flow_tuple *tuple)
{
	struct ipv6hdr *ip6h = (struct ipv6hdr *)outer;
	struct iphdr *ip4h = (struct iphdr *)outer;
	struct ip_esp_hdr *esph;

	memset(tuple, 0, sizeof(*tuple));
	if (ip4h->version == IPVERSION) {
		outer += sizeof(*ip4h);

		tuple->src_ip[0] = ntohl(ip4h->saddr);
		tuple->dest_ip[0] = ntohl(ip4h->daddr);
		tuple->proto_next_hdr = ip4h->protocol;
		tuple->ip_version = IPVERSION;

		if (natt) {
			tuple->sport = NSS_IPSECMGR_NATT_PORT_DATA;
			tuple->dport = NSS_IPSECMGR_NATT_PORT_DATA;
			tuple->proto_next_hdr = IPPROTO_UDP;
			outer += sizeof(struct udphdr);
		}

		esph = (struct ip_esp_hdr *)outer;
		tuple->spi_index = ntohl(esph->spi);
		return true;
	}

	if ((ip6h->version != 6) || (ip6h->nexthdr != IPPROTO_ESP)) {
		return false;
	}

	outer += sizeof(*ip6h);
	esph = (struct ip_esp_hdr *)outer;

	nss_ipsec_klips_v6addr_ntoh(tuple->src_ip, ip6h->saddr.s6_addr32);
	nss_ipsec_klips_v6addr_ntoh(tuple->dest_ip, ip6h->daddr.s6_addr32);

	tuple->spi_index = ntohl(esph->spi);
	tuple->proto_next_hdr = IPPROTO_ESP;
	tuple->ip_version = 6;
	return true;
}

/*
 * nss_ipsec_klips_inner2flow_tuple()
 *	Fill inner flow
 */
static void nss_ipsec_klips_inner2flow_tuple(uint8_t *ip, uint8_t proto, struct nss_ipsecmgr_flow_tuple *tuple)
{
	struct ipv6hdr *ip6h = (struct ipv6hdr *)ip;
	struct iphdr *iph = (struct iphdr *)ip;

	/*
	 * TODO: Since, we are pushing 3-tuple for every 5-tuple
	 * It is possible that the each 3-tuple maps to multiple unique
	 * 5-tuple rules. Thus we need to add support for identifying
	 * them and then allow adding or deleting of 3-tuple correctly
	 */

	tuple->sport = 0;
	tuple->dport = 0;
	tuple->use_pattern = 0;
	tuple->ip_version = iph->version;

	if (tuple->ip_version == IPVERSION) {
		tuple->src_ip[0] = ntohl(iph->saddr);
		tuple->dest_ip[0] = ntohl(iph->daddr);
		tuple->proto_next_hdr = proto ? proto : iph->protocol;
		return;
	}

	BUG_ON(tuple->ip_version != 6);

	nss_ipsec_klips_v6addr_ntoh(tuple->src_ip, ip6h->saddr.s6_addr32);
	nss_ipsec_klips_v6addr_ntoh(tuple->dest_ip, ip6h->daddr.s6_addr32);

	if (ip6h->nexthdr == NEXTHDR_FRAGMENT) {
		struct frag_hdr *fragh = (struct frag_hdr *)(ip + sizeof(*ip6h));
		proto = fragh->nexthdr;
	}

	tuple->proto_next_hdr = proto ? proto : ip6h->nexthdr;
}

/*
 * nss_ipsec_klips_sa2ecm_tuple()
 *	Fill ecm connection tuple.
 */
static void nss_ipsec_klips_sa2ecm_tuple(struct nss_ipsecmgr_sa_tuple *sa, struct ecm_notifier_connection_tuple *tuple)
{
	tuple->dst_port = sa->dport;
	tuple->src_port = sa->sport;
	tuple->ip_ver = sa->ip_version;
	tuple->protocol = sa->proto_next_hdr;

	if (sa->ip_version == IPVERSION) {
		tuple->src.in.s_addr = sa->src_ip[0];
		tuple->dest.in.s_addr = sa->dest_ip[0];
		return;
	}

	BUG_ON(sa->ip_version != 6);

	tuple->src.in6.s6_addr32[0] = sa->src_ip[0];
	tuple->src.in6.s6_addr32[1] = sa->src_ip[1];
	tuple->src.in6.s6_addr32[2] = sa->src_ip[2];
	tuple->src.in6.s6_addr32[3] = sa->src_ip[3];

	tuple->dest.in6.s6_addr32[0] = sa->dest_ip[0];
	tuple->dest.in6.s6_addr32[1] = sa->dest_ip[1];
	tuple->dest.in6.s6_addr32[2] = sa->dest_ip[2];
	tuple->dest.in6.s6_addr32[3] = sa->dest_ip[3];
}

/*
 * nss_ipsec_klips_fallback_esp_handler()
 *	Fallback to original ESP protocol handler.
 */
static int nss_ipsec_klips_fallback_esp_handler(struct sk_buff *skb)
{

	switch (ip_hdr(skb)->version) {
	case IPVERSION: {
		struct net_protocol *esp_handler;
		xchg(&esp_handler, klips_esp_handler);

		if (esp_handler && esp_handler->handler) {
			return esp_handler->handler(skb);
		}

		nss_ipsec_klips_warn("%p: Fallback ESP handler not present for IPv4\n", skb);
		break;
	}

	case 6: {
		struct inet6_protocol *esp_handler;
		xchg(&esp_handler, klips_esp6_handler);

		if (esp_handler && esp_handler->handler) {
			return esp_handler->handler(skb);
		}

		nss_ipsec_klips_warn("%p: Fallback ESP handler not present for IPv6\n", skb);
		break;
	}

	default:
		nss_ipsec_klips_warn("%p: Invalid IP header version:%u\n", skb, ip_hdr(skb)->version);
		break;
	}

	nss_ipsec_klips_warn("%p: Droping SKB", skb);
	dev_kfree_skb_any(skb);
	return 0;
}

/*
 * nss_ipsec_klips_init_trans_offload()
 * 	Reset all headers added by KLIPS for transport mode.
 */
static void nss_ipsec_klips_init_trans_offload(struct sk_buff *skb, int8_t iv_len, uint8_t hash_len)
{
	struct iphdr iph;
	uint8_t ip_proto;
	uint8_t *tail;
	uint8_t pad;

	/*
	 * Note: We need to reset the SKB to the inner payload.
	 * Strip the outer ESP header added by KLIPs, move outer
	 * IP header before payload and trim the trailer by reading
	 * the inner payload length
	 */
	skb_reset_network_header(skb);
	tail = skb_tail_pointer(skb) - hash_len - (2 * sizeof(uint8_t));

	ip_proto = tail[1];
	pad = tail[0];
	skb_trim(skb, (tail - skb->data) - pad);

	if (ip_hdr(skb)->version == IPVERSION) {
		memcpy(&iph, skb->data, sizeof(iph));
		skb_pull(skb, sizeof(iph) + sizeof(struct ip_esp_hdr) + iv_len);
		iph.protocol = ip_proto;
		iph.tot_len = htons(skb->len + sizeof(iph));
		memcpy(skb_push(skb, sizeof(iph)), &iph, sizeof(iph));
		skb_reset_network_header(skb);
		return;
	}

	BUG_ON(ip_hdr(skb)->version != IPVERSION);
}

/*
 * nss_ipsec_klips_init_tun_offload()
 * 	Reset all headers added by KLIPS for tunnel mode.
 */
static void nss_ipsec_klips_init_tun_offload(struct sk_buff *skb, int8_t iv_len)
{
	struct ipv6hdr *ip6h;
	struct iphdr *iph;
	int trim_len;

	/*
	 * Note: We need to reset the SKB to the inner payload.
	 * Strip the outer header added by KLIPs and then trim
	 * the trailer by reading the inner payload length
	 */
	skb_reset_network_header(skb);
	iph = ip_hdr(skb);

	skb_pull(skb, iph->version == IPVERSION ? sizeof(*iph) : sizeof(*ip6h));
	skb_pull(skb, sizeof(struct ip_esp_hdr) + iv_len);

	skb_reset_network_header(skb);
	iph = ip_hdr(skb);

	if (iph->version == IPVERSION) {
		trim_len = ntohs(iph->tot_len);
	} else {
		ip6h = ipv6_hdr(skb);
		trim_len = ntohs(ip6h->payload_len) + sizeof(*ip6h);
	}

	skb_trim(skb, trim_len);
}

/*
 * nss_ipsec_klips_offload_inner()
 * 	Offload SKB to NSS for encapsulation.
 *
 * This function will return 0 if packet is offloaded else return 1.
 */
static int32_t nss_ipsec_klips_offload_inner(struct sk_buff *orig_skb, struct nss_cfi_crypto_info *crypto)
{
	uint16_t crypto_idx = crypto->sid & NSS_IPSEC_KLIPS_SES_MASK;
	struct nss_ipsecmgr_flow_tuple flow_tuple = {0};
	struct nss_ipsec_klips_skb_cb *ipsec_cb;
	struct nss_ipsecmgr_sa_tuple sa_tuple;
	struct nss_ipsec_klips_tun *tun;
	struct nss_ipsec_klips_sa *sa;
	nss_ipsecmgr_status_t status;
	struct net_device *nss_dev;
	bool ecm_accel_outer;
	struct sk_buff *skb;
	int8_t iv_len;

	iv_len = nss_ipsec_klips_get_blk_len(crypto->algo);
	if (iv_len < 0) {
		nss_ipsec_klips_warn("%p:Failed to map valid IV and block length\n", orig_skb);
		return 0;
	}

	ipsec_cb = nss_ipsec_klips_get_skb_cb(orig_skb);
	if (!ipsec_cb) {
		nss_ipsec_klips_warn("%p:Unable to get ipsec cb\n", orig_skb);
		return 0;
	}

	/*
	 * Write lock needs to be taken here as the tunnel map table
	 * field is modified.
	 */
	write_lock(&tunnel_map.lock);

	tun = nss_ipsec_klips_get_tun(ipsec_cb->hlos_dev);
	if (!tun) {
		write_unlock(&tunnel_map.lock);
		nss_ipsec_klips_warn("%p: Failed to find tun entry\n", ipsec_cb->hlos_dev);
		return 1;
	}

	nss_dev = tun->nss_dev;
	BUG_ON(!nss_dev);

	sa = nss_ipsec_klips_sa_lookup(tun, crypto_idx);
	if (!sa) {
		write_unlock(&tunnel_map.lock);
		nss_ipsec_klips_trace("%p: Failed to find SA entry(%u)\n", tun, crypto_idx);
		return 1;
	}

	/*
	 * Check if SA is present,else let it process through KLIPS.
	 * Note: This can happen for the first few encapsulation packet
	 */
	if (!nss_ipsecmgr_sa_verify(nss_dev, &sa->outer)) {
		write_unlock(&tunnel_map.lock);
		return 1;
	}

	/*
	 * Copy SA tuple.
	 */
	sa_tuple = sa->outer;
	ecm_accel_outer = sa->ecm_accel_outer;

	/*
	 * Check if ECM has pushed the rule. If present than push the inner flow rule.
	 */
	if (!ecm_accel_outer) {
		struct ecm_notifier_connection_tuple ecm_tuple = {0};
		enum ecm_notifier_connection_state ecm_state;

		nss_ipsec_klips_sa2ecm_tuple(&sa_tuple, &ecm_tuple);

		ecm_state = ecm_notifier_connection_state_get(&ecm_tuple);
		if (ecm_state == ECM_NOTIFIER_CONNECTION_STATE_ACCEL) {
			ecm_accel_outer = sa->ecm_accel_outer = true;
		}

		nss_ipsec_klips_trace("%p: Get ecm connection state(%u)\n", tun, ecm_state);
	}

	dev_hold(nss_dev);
	write_unlock(&tunnel_map.lock);

	/*
	 * If it is a IPv6 packet in transport mode, drop it as
	 * this flow is not supported.
	 */
	if (ipsec_cb->flags & NSS_IPSEC_KLIPS_FLAG_TRANSPORT_MODE) {
		skb_reset_network_header(orig_skb);
		if (unlikely(ip_hdr(orig_skb)->version != IPVERSION)) {
			nss_ipsec_klips_warn("%p:IPv6 transport mode offload is not supported\n", orig_skb);
			dev_put(nss_dev);
			return 1;
		}
	}

	/*
	 * Fix the original SKB and then do a copy of it for NSS.
	 * We need to do this because the cache lines modified by KLIPS will not be
	 * needed anymore. Addtionally, the dma_map_single only flush/invalidates the
	 * SKB from head to tail. In this case the tail has shortened.
	 */
	if (ipsec_cb->flags & NSS_IPSEC_KLIPS_FLAG_TRANSPORT_MODE) {
		nss_ipsec_klips_init_trans_offload(orig_skb, iv_len, crypto->hash_len);
	} else {
		nss_ipsec_klips_init_tun_offload(orig_skb, iv_len);
	}

	/*
	 * We create a copy of the KLIPS skb; since this will be transmitted out
	 * of NSS. We cannot expect it to return to host. Hence, we overwrite
	 * the skb with the clone.
	 */
	skb = skb_copy_expand(orig_skb, nss_dev->needed_headroom, nss_dev->needed_tailroom, GFP_ATOMIC);
	if (!skb) {
		nss_ipsec_klips_err("%p: Unable to create copy of SKB\n", nss_dev);
		dev_put(nss_dev);
		return 0;
	}

	/*
	 * We need to release all resources hold by SKB before sending it to NSS.
	 * Reset the SKB and make it orphan.
	 */
	skb_scrub_packet(skb, true);
	skb->skb_iif = orig_skb->skb_iif;

	nss_ipsec_klips_inner2flow_tuple(skb->data, 0, &flow_tuple);

	/*
	 * Try to send packet to NSS and drop the packet incase of failure.
	 * TODO: At higher packet rate, Initial packets can go out-of-order when ECM outer flow is
	 * not present and packets has to go through exception path.
	 */
	status = nss_ipsecmgr_sa_tx_inner(nss_dev, &sa_tuple, skb);
	if (status != NSS_IPSECMGR_OK) {
		nss_ipsec_klips_trace("%p: Failed to transmit encap packet, error(%u)\n", skb, status);
		dev_kfree_skb_any(skb);
	}

	/*
	 * Enable the IPsec inner flow once ECM has accelerated the outer flow.
	 * This will limit the packet rate in encap direction till ECM pushes the
	 * rule and should minimize the out-of-order issue when ECM flow is not present.
	 */
	if (ecm_accel_outer) {
		nss_ipsecmgr_flow_add(nss_dev, &flow_tuple, &sa_tuple);
	}

	dev_put(nss_dev);
	return 0;
}

/*
 * nss_ipsec_klips_offload_outer()
 *	Offload IPsec encapsulated packets.
 */
static int nss_ipsec_klips_offload_outer(struct sk_buff *skb, struct nss_ipsecmgr_sa_tuple *sa_tuple,
					struct nss_ipsecmgr_flow_tuple *flow_tuple)
{
	struct nss_ipsec_klips_tun *tun;
	nss_ipsecmgr_status_t status;
	struct net_device *nss_dev;
	int ifindex;

	/*
	 * Check if we are tracking net_device for this IP.
	 * If not then this is for IPsec bypass.
	 */
	write_lock(&tunnel_map.lock);
	tun = nss_ipsec_klips_get_tun_by_addr(skb);
	if (!tun) {
		write_unlock(&tunnel_map.lock);
		return nss_ipsec_klips_fallback_esp_handler(skb);
	}

	ifindex = tun->klips_dev->ifindex;
	nss_dev = tun->nss_dev;
	dev_hold(nss_dev);

	write_unlock(&tunnel_map.lock);

	/*
	 * Check if SA is present,else fallback to KLIPS.
	 * Note: This can happen for the first few decapsulation packet
	 */
	if (!nss_ipsecmgr_sa_verify(nss_dev, sa_tuple)) {
		dev_put(nss_dev);
		return nss_ipsec_klips_fallback_esp_handler(skb);
	}

	/*
	 * In case of Decap, skb->data points to ESP header.
	 * Set the skb->data to outer IP header.
	 */
	skb_push(skb, skb->data - skb_network_header(skb));

	/*
	 * If skb was cloned (likely due to a packet sniffer), Make a copy.
	 * skb_cow() has check for skb_cloned().
	 */
	if (skb_cow(skb, skb_headroom(skb))) {
		nss_ipsec_klips_warn("%p: Failed to create writable copy, Droping", skb);
		goto drop_skb;
	}

	/*
	 * We need to release all resources hold by SKB before sending it to NSS.
	 * Reset the SKB and make it orphan.
	 */
	skb_scrub_packet(skb, true);
	skb->skb_iif = ifindex;

	/*
	 * Offload further processing to NSS.
	 * Note: This can fail if the queue is full
	 */
	status = nss_ipsecmgr_sa_tx_outer(nss_dev, sa_tuple, skb);
	if (status != NSS_IPSECMGR_OK) {
		dev_kfree_skb_any(skb);
	}

	/*
	 * Add outer flow as SKB is offloaded.
	 */
	nss_ipsecmgr_flow_add(nss_dev, flow_tuple, sa_tuple);

	dev_put(nss_dev);
	return 0;

drop_skb:
	dev_kfree_skb_any(skb);
	dev_put(nss_dev);
	return 0;
}

/*
 * nss_ipsec_klips_offload_esp()
 *	ESP Protocol handler for IPsec encapsulated packets.
 */
static int nss_ipsec_klips_offload_esp(struct sk_buff *skb)
{
	struct nss_ipsecmgr_flow_tuple flow_tuple = {0};
	struct nss_ipsecmgr_sa_tuple sa_tuple = {0};
	uint8_t ttl;

	nss_ipsec_klips_trace("%p: SKb recieved by KLIPS plugin\n", skb);

	nss_ipsec_klips_outer2sa_tuple(skb_network_header(skb), false, &sa_tuple, &ttl, true);
	nss_ipsec_klips_outer2flow_tuple(skb_network_header(skb), false, &flow_tuple);

	return nss_ipsec_klips_offload_outer(skb, &sa_tuple, &flow_tuple);
}

/*
 * nss_ipsec_klips_trap_encap()
 *	Trap IPsec pkts for sending encap fast path rules.
 */
static int32_t nss_ipsec_klips_trap_encap(struct sk_buff *skb, struct nss_cfi_crypto_info *crypto)
{
	uint16_t crypto_idx = crypto->sid & NSS_IPSEC_KLIPS_SES_MASK;
	struct nss_ipsecmgr_sa_tuple sa_tuple = {0};
	struct nss_ipsec_klips_skb_cb *ipsec_cb;
	struct nss_ipsec_klips_sa *sa_entry;
	struct nss_ipsecmgr_sa_data sa = {0};
	struct nss_ipsec_klips_tun *tun;
	nss_ipsecmgr_status_t status;
	struct net_device *nss_dev;
	enum nss_ipsecmgr_algo algo;
	bool transport_mode;
	bool natt;
	int8_t iv_blk_len;
	uint8_t *payload;
	uint32_t if_num;
	uint8_t ttl;

	BUG_ON(skb_is_nonlinear(skb) || skb_has_frag_list(skb));

	iv_blk_len = nss_ipsec_klips_get_blk_len(crypto->algo);
	if (iv_blk_len < 0) {
		nss_ipsec_klips_warn("%p:Failed to map valid IV and block length\n", skb);
		return -EOPNOTSUPP;
	}

	algo = nss_ipsec_klips_get_algo(crypto->algo);
	if (algo >= NSS_IPSECMGR_ALGO_MAX) {
		nss_ipsec_klips_warn("%p:Failed to map valid algo\n", skb);
		return -EOPNOTSUPP;
	}

	ipsec_cb = nss_ipsec_klips_get_skb_cb(skb);
	if (!ipsec_cb) {
		nss_ipsec_klips_warn("%p:Unable to get ipsec cb\n", skb);
		return -ENOENT;
	}

	transport_mode = ipsec_cb->flags & NSS_IPSEC_KLIPS_FLAG_TRANSPORT_MODE;
	natt = ipsec_cb->flags & NSS_IPSEC_KLIPS_FLAG_NATT;

	/*
	 * construct SA information
	 */
	nss_ipsecmgr_sa_cmn_init_idx(&sa.cmn, algo, crypto->sid,
				iv_blk_len, iv_blk_len, crypto->hash_len,
				transport_mode,			/* enable transport mode */
				false,				/* no_trailer */
				false,				/* esn */
				natt);				/* natt */

	/*
	 * KLIPS adds NATT/UDP header after encrypt.
	 */
	payload = nss_ipsec_klips_outer2sa_tuple(skb->data, natt, &sa_tuple, &ttl, false);
	BUG_ON(!payload);

	sa.type = NSS_IPSECMGR_SA_TYPE_ENCAP;
	sa.encap.ttl_hop_limit = ttl;

	/*
	 * Read lock needs to be taken here as the tunnel map table
	 * is looked up
	 */
	write_lock(&tunnel_map.lock);

	tun = nss_ipsec_klips_get_tun(ipsec_cb->hlos_dev);
	if (!tun) {
		write_unlock(&tunnel_map.lock);
		nss_ipsec_klips_warn("%p:Failed to find NSS device mapped to KLIPS device\n", skb);
		return -ENOENT;
	}

	nss_dev = tun->nss_dev;
	BUG_ON(!nss_dev);

	/*
	 * This information is used by ECM to know input interface.
	 */
	skb->skb_iif = tun->klips_dev->ifindex;

	sa_entry = nss_ipsec_klips_sa_lookup(tun, crypto_idx);
	if (!sa_entry) {
		/*
		 * Allocate a new Entry
		 */
		sa_entry = kzalloc(sizeof(*sa_entry), GFP_ATOMIC);
		if (!sa_entry) {
			write_unlock(&tunnel_map.lock);
			return -ENOMEM;
		}

		sa_entry->sid = crypto_idx;
		memcpy(&sa_entry->outer, &sa_tuple, sizeof(sa_entry->outer));

		INIT_LIST_HEAD(&sa_entry->list);
		list_add_tail(&sa_entry->list, &tun->sa_list);
	}

	/*
	 * We blindly attempt an add to the IPsec manager database
	 * If, this is a duplicate entry then it will return a status then we
	 * move forward with flow add. Otherwise we have stale entry
	 * in our database which needs to be removed
	 */

	status = nss_ipsecmgr_sa_add(nss_dev, &sa_tuple, &sa, &if_num);

	/*
	 * If, SA add fails then there is no need to add the flow
	 */
	if ((status != NSS_IPSECMGR_OK) && (status != NSS_IPSECMGR_DUPLICATE_SA)) {
		list_del_init(&sa_entry->list);
		write_unlock(&tunnel_map.lock);
		kfree(sa_entry);
		return -EINVAL;
	}

	write_unlock(&tunnel_map.lock);
	nss_ipsec_klips_trace("%p: Encap SA rule message sent\n", tun);

	return 0;
}

/*
 * nss_ipsec_klips_trap_decap()
 *	Trap IPsec pkts for sending decap fast path rules.
 */
static int32_t nss_ipsec_klips_trap_decap(struct sk_buff *skb, struct nss_cfi_crypto_info *crypto)
{
	struct nss_ipsecmgr_flow_tuple flow_tuple = {0};
	struct nss_ipsecmgr_sa_tuple sa_tuple = {0};
	uint16_t crypto_idx = crypto->sid & NSS_IPSEC_KLIPS_SES_MASK;
	struct nss_ipsec_klips_tun *tun;
	struct nss_ipsec_klips_skb_cb *skb_cb;
	struct nss_ipsecmgr_sa_data sa = {0};
	struct nss_ipsec_klips_sa *sa_entry;
	nss_ipsecmgr_status_t status;
	struct net_device *nss_dev;
	enum nss_ipsecmgr_algo algo;
	bool transport_mode;
	int8_t iv_blk_len;
	uint32_t if_num;
	uint8_t *payload;
	uint8_t ttl;
	bool natt;

	iv_blk_len = nss_ipsec_klips_get_blk_len(crypto->algo);
	if (iv_blk_len < 0) {
		nss_ipsec_klips_warn("%p:Failed to map valid IV and block length\n", skb);
		return -EOPNOTSUPP;
	}

	algo = nss_ipsec_klips_get_algo(crypto->algo);
	if (algo >= NSS_IPSECMGR_ALGO_MAX) {
		nss_ipsec_klips_warn("%p:Failed to map valid algo\n", skb);
		return -EOPNOTSUPP;
	}

	skb_cb = nss_ipsec_klips_get_skb_cb(skb);
	if (!skb_cb) {
		nss_ipsec_klips_warn("%p:skb->cb is NULL\n", skb);
		return -EINVAL;
	}

	transport_mode = skb_cb->flags & NSS_IPSEC_KLIPS_FLAG_TRANSPORT_MODE;
	natt = skb_cb->flags & NSS_IPSEC_KLIPS_FLAG_NATT;

	/*
	 * construct SA information
	 */
	nss_ipsecmgr_sa_cmn_init_idx(&sa.cmn, algo, crypto->sid,
				iv_blk_len, iv_blk_len, crypto->hash_len,
				transport_mode,	/* enable transport mode */
				false,	/* no_trailer */
				false,	/* esn */
				natt	/* natt */
				);

	/*
	 * construct outer flow information
	 */
	payload = nss_ipsec_klips_outer2sa_tuple(skb_network_header(skb), natt, &sa_tuple, &ttl, true);
	BUG_ON(!payload);

	if (!nss_ipsec_klips_outer2flow_tuple(skb_network_header(skb), natt, &flow_tuple)) {
		nss_ipsec_klips_warn("%p: Invalid packet\n", skb);
		return -EINVAL;
	}

	sa.type = NSS_IPSECMGR_SA_TYPE_DECAP;
	sa.decap.replay_win = NSS_IPSEC_KLIPS_BITS2BYTE(skb_cb->replay_win);

	/*
	 * Read lock needs to be taken here as the tunnel map table
	 * is looked up
	 */
	write_lock(&tunnel_map.lock);

	tun = nss_ipsec_klips_get_tun(skb_cb->hlos_dev);
	if (!tun) {
		write_unlock(&tunnel_map.lock);
		nss_ipsec_klips_warn("%p:Failed to find NSS device mapped to KLIPS device\n", skb);
		return -ENOENT;
	}

	nss_dev = tun->nss_dev;
	BUG_ON(!nss_dev);

	/*
	 * This information is used by ECM to know input interface.
	 */
	skb->skb_iif = tun->klips_dev->ifindex;

	sa_entry = nss_ipsec_klips_sa_lookup(tun, crypto_idx);
	if (!sa_entry) {
		/*
		 * Allocate a new Entry
		 */
		sa_entry = kzalloc(sizeof(*sa_entry), GFP_ATOMIC);
		if (!sa_entry) {
			write_unlock(&tunnel_map.lock);
			return -ENOMEM;
		}

		sa_entry->sid = crypto_idx;
		memcpy(&sa_entry->outer, &sa_tuple, sizeof(sa_entry->outer));

		INIT_LIST_HEAD(&sa_entry->list);
		list_add_tail(&sa_entry->list, &tun->sa_list);
	}

	/*
	 * We blindly attempt an add to the IPsec manager database
	 * If, this is a duplicate entry then it will return a status then we
	 * move forward with flow add. Otherwise we have stale entry
	 * in our database which needs to be removed
	 */

	status = nss_ipsecmgr_sa_add(nss_dev, &sa_tuple, &sa, &if_num);

	/*
	 * If, SA add fails then there is no need to add the flow
	 */
	if ((status != NSS_IPSECMGR_OK) && (status != NSS_IPSECMGR_DUPLICATE_SA)) {
		list_del_init(&sa_entry->list);
		write_unlock(&tunnel_map.lock);
		kfree(sa_entry);
		return -EINVAL;
	}

	write_unlock(&tunnel_map.lock);
	nss_ipsec_klips_trace("%p: Decap SA rule message sent\n", tun);

	return 0;
}

/*
 * nss_ipsec_klips_get_dev_and_type()
 *	Get ipsecmgr tunnel netdevice and type for klips netdevice
 */
static struct net_device *nss_ipsec_klips_get_dev_and_type(struct net_device *klips_dev, struct sk_buff *skb,
							int32_t *type)
{

	switch (ip_hdr(skb)->version) {
	case 4: {
		struct iphdr *iph = ip_hdr(skb);
		uint16_t natt_port = ntohs(NSS_IPSECMGR_NATT_PORT_DATA);

		/*
		 * Protocol is ESP, type is OUTER
		 */
		if (iph->protocol == IPPROTO_ESP) {
			*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER;
			break;
		}

		skb_set_transport_header(skb, sizeof(*iph));
		if ((iph->protocol == IPPROTO_UDP) && (udp_hdr(skb)->dest == natt_port)) {
			*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER;
			break;
		}

		*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_INNER;
		break;
	}

	case 6: {
		struct ipv6hdr *ip6h = ipv6_hdr(skb);

		/*
		 * Protocol is ESP, type is OUTER
		 */
		if (ip6h->nexthdr == IPPROTO_ESP) {
			*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER;
			break;
		}

		/*
		 * All other case it is Inner
		 */
		*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_INNER;
		break;
	}

	default:
		nss_ipsec_klips_warn("%p: Packet is not IPv4 or IPv6. version=%d\n", klips_dev, ip_hdr(skb)->version);
		return NULL;
	}

	return nss_ipsec_klips_get_tun_dev(klips_dev);
}

/*
 * nss_ipsec_klips_flow_delete()
 *	Delete a ipsec flow.
 */
static bool nss_ipsec_klips_flow_delete(struct net_device *nss_dev, struct nss_ipsecmgr_flow_tuple *flow_tuple)
{
	struct nss_ipsecmgr_sa_tuple sa_tuple = {0};

	nss_ipsec_klips_trace("%p: Flow delete for tuple src_ip= %u:%u:%u:%u, dest_ip= %u:%u:%u:%u,\
			proto_next_hdr=%u, ip_version= %u\n", nss_dev, flow_tuple->src_ip[0],
			flow_tuple->src_ip[1], flow_tuple->src_ip[2], flow_tuple->src_ip[3],
			flow_tuple->dest_ip[0], flow_tuple->dest_ip[1], flow_tuple->dest_ip[2],
			flow_tuple->dest_ip[3], flow_tuple->proto_next_hdr, flow_tuple->ip_version);

	if (nss_ipsecmgr_flow_get_sa(nss_dev, flow_tuple, &sa_tuple) != NSS_IPSECMGR_OK) {
		nss_ipsec_klips_trace("%p: SA not found\n", nss_dev);
		return false;
	}

	nss_ipsecmgr_flow_del(nss_dev, flow_tuple, &sa_tuple);

	/*
	 * TODO:handle case when tx message to NSS fails in nss_ipsecmgr_flow_del()
	 */
	nss_ipsec_klips_trace("%p: IPSec Flow deleted\n", nss_dev);
	return true;
}

/*
 * nss_ipsec_klips_register_esp_handler()
 * 	Register ESP handler.
 */
static void nss_ipsec_klips_register_esp_handler(void)
{
	const struct net_protocol *ip_prot;

	/*
	 * This function must be called with rtnl lock.
	 */
	ASSERT_RTNL();

	if (klips_esp6_handler && klips_esp_handler) {
		nss_ipsec_klips_trace("ESP handler already registered\n");
		return;
	}

	if (inet_update_protocol(&esp_protocol, IPPROTO_ESP, &klips_esp_handler) < 0) {
		nss_ipsec_klips_err("Failed to update ESP protocol handler for IPv4\n");

		/*
		 * In error case function is modifying this variable, revert it to NULL.
		 */
		xchg(&klips_esp_handler, NULL);
		return;
	}

	if (inet6_update_protocol(&esp6_protocol, IPPROTO_ESP, &klips_esp6_handler) < 0) {
		nss_ipsec_klips_err("Failed to update ESP protocol handler for IPv6\n");

		/*
		 * Revert v4 ESP handler to original handler.
		 */
		xchg(&klips_esp6_handler, NULL);
		inet_update_protocol(klips_esp_handler, IPPROTO_ESP, &ip_prot);
		xchg(&klips_esp_handler, NULL);
		return;
	}

	nss_ipsec_klips_info("ESP handler registered\n");
}

/*
 * nss_ipsec_klips_unregister_esp_handler()
 * 	Unregister ESP handler.
 */
static void nss_ipsec_klips_unregister_esp_handler(void)
{
	const struct inet6_protocol *ip6_prot;
	const struct net_protocol *ip_prot;

	/*
	 * This function must be called with rtnl lock.
	 */
	ASSERT_RTNL();

	if (klips_esp_handler) {
		inet_update_protocol(klips_esp_handler, IPPROTO_ESP, &ip_prot);
		xchg(&klips_esp_handler, NULL);
		nss_ipsec_klips_info("IPv4 ESP handler un-registered\n");
	}

	if (klips_esp6_handler) {
		inet6_update_protocol(klips_esp6_handler, IPPROTO_ESP, &ip6_prot);
		xchg(&klips_esp6_handler, NULL);
		nss_ipsec_klips_info("IPv6 ESP handler un-registered\n");
	}
}

/*
 * nss_ipsec_klips_dev_event()
 *	notifier function for IPsec device events.
 */
static int nss_ipsec_klips_dev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *klips_dev = netdev_notifier_info_to_dev(ptr);
	struct nss_ipsecmgr_callback ipsec_cb = {0};
	struct nss_ipsec_klips_tun *tun;
	bool remove_esp_handler = false;
	struct net_device *nss_dev;
	int16_t index = 0;
	int mtu = 0;

	if (tunnel_map.max <= tunnel_map.used)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_REGISTER:
		index = nss_ipsec_klips_get_index(klips_dev->name);
		if ((index < 0) || (index >= tunnel_map.max)) {
			nss_ipsec_klips_trace("Netdev(%s) is not KLIPS IPsec related\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * All exception packets are handled by IPsec manager and
		 * hence, the lack of need to register a data callback.
		 * The event callback can be used, if the need arises
		 * to send an asynchronous event to OCF layer.
		 */
		ipsec_cb.skb_dev = klips_dev;
		ipsec_cb.data_cb = NULL;
		ipsec_cb.event_cb = NULL;

		nss_ipsec_klips_info("IPsec interface being registered: %s\n", klips_dev->name);

		nss_dev = nss_ipsecmgr_tunnel_add(&ipsec_cb);
		if (!nss_dev) {
			nss_ipsec_klips_err("NSS IPsec tunnel dev allocation failed for %s\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * Write lock is needed here since tunnel map table
		 * is modified
		 */
		write_lock_bh(&tunnel_map.lock);

		tunnel_map.used++;
		tunnel_map.tbl[index].nss_dev = nss_dev;
		tunnel_map.tbl[index].klips_dev = klips_dev;
		dev_hold(klips_dev);

		write_unlock_bh(&tunnel_map.lock);

		/*
		 * Register ESP handler.
		 */
		nss_ipsec_klips_register_esp_handler();
		break;

	case NETDEV_UNREGISTER:
		index = nss_ipsec_klips_get_index(klips_dev->name);
		if ((index < 0) || (index >= tunnel_map.max)) {
			nss_ipsec_klips_trace("Netdev(%s) is not KLIPS IPsec related\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * Read lock necessary as tunnel map table is accessed but
		 * not modified
		 */
		write_lock_bh(&tunnel_map.lock);
		tun = &tunnel_map.tbl[index];

		if (!tun->klips_dev || !tun->nss_dev) {
			write_unlock_bh(&tunnel_map.lock);
			nss_ipsec_klips_err("%p:Failed to find tunnel map\n", klips_dev);
			return NOTIFY_DONE;
		}

		if (tun->klips_dev != klips_dev) {
			write_unlock_bh(&tunnel_map.lock);
			nss_ipsec_klips_err("Failed to find NSS IPsec tunnel dev for %s\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * Since, we created the NSS device then we should not never see this happen
		 */
		nss_dev = tun->nss_dev;
		BUG_ON(!nss_dev);

		nss_ipsec_klips_info("IPsec interface being unregistered: %s\n", klips_dev->name);
		nss_ipsec_klips_sa_flush(tun, nss_dev);

		/*
		 * Write lock is needed here since tunnel map table
		 * is modified
		 */
		tun->nss_dev = NULL;
		tun->klips_dev = NULL;
		tunnel_map.used--;

		/*
		 * Unregister the ESP handler only when all tunnels are unregistered.
		 */
		remove_esp_handler = !tunnel_map.used;

		write_unlock_bh(&tunnel_map.lock);

		nss_ipsecmgr_tunnel_del(nss_dev);
		dev_put(klips_dev);

		if (remove_esp_handler) {
			nss_ipsec_klips_unregister_esp_handler();
		}
		break;

	case NETDEV_CHANGEMTU:
		index = nss_ipsec_klips_get_index(klips_dev->name);
		if ((index < 0) || (index >= tunnel_map.max)) {
			nss_ipsec_klips_trace("Netdev(%s) is not KLIPS IPsec related\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		/*
		 * Read lock necessary as tunnel map table is accessed but
		 * not modified
		 */
		write_lock_bh(&tunnel_map.lock);
		tun = &tunnel_map.tbl[index];

		if (!tun->klips_dev || !tun->nss_dev) {
			write_unlock_bh(&tunnel_map.lock);
			nss_ipsec_klips_err("%p:Failed to find tunnel map\n", klips_dev);
			return NOTIFY_DONE;
		}

		if (tun->klips_dev != klips_dev) {
			write_unlock_bh(&tunnel_map.lock);
			nss_ipsec_klips_err("Failed to find NSS IPsec tunnel dev for %s\n", klips_dev->name);
			return NOTIFY_DONE;
		}

		nss_dev = tun->nss_dev;
		mtu = klips_dev->mtu;
		write_unlock_bh(&tunnel_map.lock);

		dev_set_mtu(nss_dev, mtu);
		break;

	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}

/*
 * nss_ipsec_klips_ecm_conn_to_tuple()
 * 	Converts ecm_notifier_connection_data to nss_ipsecmgr_flow_tuple.
 */
static inline bool nss_ipsec_klips_ecm_conn_to_tuple(struct ecm_notifier_connection_data *conn,
		struct nss_ipsecmgr_flow_tuple *tuple, bool is_return)
{
	struct in6_addr *sip6, *dip6;
	struct in_addr *sip, *dip;
	int i;

	memset(tuple, 0, sizeof(*tuple));
	tuple->ip_version = conn->tuple.ip_ver;
	tuple->proto_next_hdr = conn->tuple.protocol;

	if (is_return) {
		sip = &conn->tuple.dest.in;
		dip = &conn->tuple.src.in;
		sip6 = &conn->tuple.dest.in6;
		dip6 = &conn->tuple.src.in6;
		tuple->sport = conn->tuple.dst_port;
		tuple->dport = conn->tuple.src_port;
	} else {
		dip = &conn->tuple.dest.in;
		sip = &conn->tuple.src.in;
		dip6 = &conn->tuple.dest.in6;
		sip6 = &conn->tuple.src.in6;
		tuple->dport = conn->tuple.dst_port;
		tuple->sport = conn->tuple.src_port;
	}

	switch (tuple->ip_version) {
	case 4:
		tuple->dest_ip[0] = dip->s_addr;
		tuple->src_ip[0] = sip->s_addr;
		break;

	case 6:
		for (i = 0; i < 4; i++) {
			tuple->dest_ip[i] = dip6->s6_addr32[i];
			tuple->src_ip[i] = sip6->s6_addr32[i];
		}
		break;

	default:
		/*
                 * Shouldn't come here.
                 */
                nss_ipsec_klips_err("%p: Invalid protocol\n", conn);
		return false;
	}

	return true;
}

/*
 * nss_ipsec_klips_ecm_conn_notify()
 *	Notifier function for ECM connection events.
 */
static int nss_ipsec_klips_ecm_conn_notify(struct notifier_block *nb, unsigned long event, void *ptr)
{
	struct ecm_notifier_connection_data *conn = ptr;
	struct nss_ipsecmgr_flow_tuple flow_tuple = {0};
	struct net_device *nss_dev;
	bool is_return = false;

	if (event != ECM_NOTIFIER_ACTION_CONNECTION_REMOVED) {

		/*
		 * Invalid event, Do nothing.
		 */
		nss_ipsec_klips_trace("%p: Invalid event recieved\n", nb);
		return NOTIFY_OK;
	}

	nss_ipsec_klips_trace("%p: Event ECM_NOTIFIER_ACTION_CONNECTION_REMOVE recieved\n", nb);

	switch (conn->tuple.protocol) {
	case IPPROTO_ESP:

		/*
		 * Connection belongs to outer flow and it will delete when parent SA gets deref.
		 */
		nss_ipsec_klips_trace("%p: Connection protocol is ESP, no action required\n", nb);
		return NOTIFY_OK;

	case IPPROTO_UDP:

		/*
		 * If Connection belongs to NAT-T (Outer flow) then it will delete when parent SA gets deref.
		 */
		if (conn->tuple.src_port == NSS_IPSECMGR_NATT_PORT_DATA) {
			nss_ipsec_klips_trace("%p: Connection is with NAT-T source port, no action required\n", nb);
			return NOTIFY_OK;
		} else if (conn->tuple.dst_port == NSS_IPSECMGR_NATT_PORT_DATA) {
			nss_ipsec_klips_trace("%p: Connection is with NAT-T dest port, no action required\n", nb);
			return NOTIFY_OK;
		}

		break;

	default:
		break;
	}

	nss_dev = nss_ipsec_klips_get_tun_dev(conn->from_dev);
	if (nss_dev) {
		is_return = true;
		nss_ipsec_klips_trace("%p: Tunnel Device found in 'from' dir\n", conn);
		goto found;
	}

	nss_dev = nss_ipsec_klips_get_tun_dev(conn->to_dev);
	if (!nss_dev) {
		nss_ipsec_klips_trace("%p: Tunnel Device not found for 'to_dev' & 'from_dev'\n", conn);
		return NOTIFY_DONE;
	}

	nss_ipsec_klips_trace("%p: Tunnel Device found in 'to' dir\n", conn);

found:
	if (!nss_ipsec_klips_ecm_conn_to_tuple(conn, &flow_tuple, is_return)) {
		nss_ipsec_klips_err("%p: Invalid connection data\n", conn);
		dev_put(nss_dev);
		return NOTIFY_DONE;
	}

	if (!nss_ipsec_klips_flow_delete(nss_dev, &flow_tuple)) {
		nss_ipsec_klips_trace("%p: nss_ipsec_klips_flow_delete failed\n", conn);
	}

	dev_put(nss_dev);
	return NOTIFY_OK;
}

static struct notifier_block nss_ipsec_klips_notifier = {
	.notifier_call = nss_ipsec_klips_dev_event,
};

static struct ecm_interface_ipsec_callback nss_ipsec_klips_ecm =  {
	.tunnel_get_and_hold = nss_ipsec_klips_get_dev_and_type
};

static struct notifier_block nss_ipsec_klips_ecm_conn_notifier = {
	.notifier_call = nss_ipsec_klips_ecm_conn_notify,
};

#if defined(NSS_L2TPV2_ENABLED)
static struct l2tpmgr_ipsecmgr_cb nss_ipsec_klips_l2tp =  {
	.cb = nss_ipsec_klips_get_tun_dev
};
#endif

/*
 * nss_ipsec_klips_init_module()
 *	Initialize IPsec rule tables and register various callbacks
 */
int __init nss_ipsec_klips_init_module(void)
{
	struct nss_ipsec_klips_tun *tun;
	int i;

	nss_ipsec_klips_info("NSS IPsec (platform - IPQ807x , %s) loaded\n", NSS_IPSEC_KLIPS_BUILD_ID);

	tunnel_map.tbl = vzalloc(sizeof(struct nss_ipsec_klips_tun) * tunnel_max);
	if (!tunnel_map.tbl) {
		nss_ipsec_klips_warn("Unable to allocate tunnel map table\n");
		return -1;
	}

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_max; i++, tun++) {
		tun->nss_dev = NULL;
		tun->klips_dev = NULL;
		INIT_LIST_HEAD(&tun->sa_list);
	}

	rwlock_init(&tunnel_map.lock);

	tunnel_map.max = tunnel_max;
	tunnel_map.used = 0;

	register_netdevice_notifier(&nss_ipsec_klips_notifier);
	nss_cfi_ocf_register_ipsec(nss_ipsec_klips_trap_encap, nss_ipsec_klips_trap_decap, nss_ipsec_klips_free_session,
					nss_ipsec_klips_offload_inner);

	ecm_interface_ipsec_register_callbacks(&nss_ipsec_klips_ecm);
	ecm_notifier_register_connection_notify(&nss_ipsec_klips_ecm_conn_notifier);
#if defined(NSS_L2TPV2_ENABLED)
	l2tpmgr_register_ipsecmgr_callback(&nss_ipsec_klips_l2tp);
#endif
	return 0;
}

/*
 * nss_ipsec_klips_exit_module()
 *	Unregister callbacks/notifiers and clear all stale data
 */
void __exit nss_ipsec_klips_exit_module(void)
{
	struct nss_ipsec_klips_tun *tun;
	int i;

	/*
	 * Detach the trap handlers and Linux NETDEV notifiers, before
	 * unwinding the tunnels
	 */

	ecm_notifier_unregister_connection_notify(&nss_ipsec_klips_ecm_conn_notifier);
	ecm_interface_ipsec_unregister_callbacks();
#if defined(NSS_L2TPV2_ENABLED)
	l2tpmgr_unregister_ipsecmgr_callback();
#endif

	nss_cfi_ocf_unregister_ipsec();
	unregister_netdevice_notifier(&nss_ipsec_klips_notifier);

	/*
	 * Write lock needs to be taken here since SA table is
	 * getting modified
	 */
	write_lock_bh(&tunnel_map.lock);

	for (i = 0, tun = tunnel_map.tbl; i < tunnel_map.max; i++, tun++) {
		if (!tun->nss_dev || !tun->klips_dev)
			continue;

		/*
		 * Find the NSS device associated with klips device
		 */
		nss_ipsec_klips_sa_flush(tun, tun->nss_dev);
		nss_ipsecmgr_tunnel_del(tun->nss_dev);
		BUG_ON(!list_empty(&tun->sa_list));

		/*
		 * Reset the tunnel entry
		 */
		tun->nss_dev = NULL;
		tun->klips_dev = NULL;
		tunnel_map.used--;
	}

	write_unlock_bh(&tunnel_map.lock);

	tunnel_map.used = tunnel_max;
	tunnel_map.max = 0;

	vfree(tunnel_map.tbl);
	nss_ipsec_klips_info("module unloaded\n");
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS IPsec offload glue");

module_init(nss_ipsec_klips_init_module);
module_exit(nss_ipsec_klips_exit_module);
