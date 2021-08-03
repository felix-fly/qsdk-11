/*
 ***************************************************************************
 * Copyright (c) 2014-2015,2019, The Linux Foundation. All rights reserved.
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
 * nss_nlgre_redir_mgr_interface_map()
 * 	Interface map message.
 */
bool nss_nlgre_redir_mgr_interface_map(uint32_t vap_nssif, uint32_t tun_type, struct nss_nlgre_redir_map *interface_map_params);

/*
 * nss_nlgre_redir_mgr_interface_unmap()
 * 	Interface unmap message.
 */
bool nss_nlgre_redir_mgr_interface_unmap(uint32_t vap_nssif, struct nss_nlgre_redir_unmap *interface_unmap_params);

/*
 * nss_nlgre_redir_mgr_set_next_hop()
 * 	Sets next hop as gre-redir for wifi.
 */
bool nss_nlgre_redir_mgr_set_next_hop(struct nss_nlgre_redir_set_next *interface_setnext_params);

/*
 * nss_nlgre_redir_mgr_create_tun()
 * 	Allocates net_dev and configures tunnel.
 */
int nss_nlgre_redir_mgr_create_tun(struct nss_nlgre_redir_create_tun *create_params);

/*
 * nss_nlgre_redir_mgr_destroy_tun()
 * 	Unregisters and deallocs dynamic interfaces.
 */
bool nss_nlgre_redir_mgr_destroy_tun(struct net_device *dev);

/*
 * nss_gre_redir_tunnel_types nss_nlgre_redir_mgr_tunnel_type()
 * 	Returns the tunnel type
 */
enum nss_gre_redir_tunnel_types nss_nlgre_redir_mgr_tunnel_type(char *tun_type);
