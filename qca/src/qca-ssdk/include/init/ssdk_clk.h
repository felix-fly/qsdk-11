/*
 * Copyright (c) 2017, 2019, The Linux Foundation. All rights reserved.
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
 */

 #ifndef _SSDK_CLK_H_
#define _SSDK_CLK_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define PPE_RESET_ID	"ppe_rst"
#define UNIPHY0_SOFT_RESET_ID	"uniphy0_soft_rst"
#define UNIPHY0_XPCS_RESET_ID	"uniphy0_xpcs_rst"
#define UNIPHY1_SOFT_RESET_ID	"uniphy1_soft_rst"
#define UNIPHY1_XPCS_RESET_ID	"uniphy1_xpcs_rst"
#define UNIPHY2_SOFT_RESET_ID	"uniphy2_soft_rst"
#define UNIPHY2_XPCS_RESET_ID	"uniphy2_xpcs_rst"
#define UNIPHY0_PORT1_DISABLE_ID	"uniphy0_port1_dis"
#define UNIPHY0_PORT2_DISABLE_ID	"uniphy0_port2_dis"
#define UNIPHY0_PORT3_DISABLE_ID	"uniphy0_port3_dis"
#define UNIPHY0_PORT4_DISABLE_ID	"uniphy0_port4_dis"
#define UNIPHY0_PORT5_DISABLE_ID	"uniphy0_port5_dis"
#define UNIPHY0_PORT_4_5_RESET_ID	"uniphy0_port_4_5_rst"
#define UNIPHY0_PORT_4_RESET_ID	"uniphy0_port_4_rst"

#define SSDK_PORT1_RESET_ID	"nss_port1_rst"
#define SSDK_PORT2_RESET_ID	"nss_port2_rst"
#define SSDK_PORT3_RESET_ID	"nss_port3_rst"
#define SSDK_PORT4_RESET_ID	"nss_port4_rst"
#define SSDK_PORT5_RESET_ID	"nss_port5_rst"
#define SSDK_PORT6_RESET_ID	"nss_port6_rst"

enum ssdk_rst_action {
        SSDK_RESET_DEASSERT      = 0,
        SSDK_RESET_ASSERT        = 1
};

enum unphy_rst_type {
	UNIPHY0_SOFT_RESET_E = 0,
	UNIPHY0_XPCS_RESET_E,
	UNIPHY1_SOFT_RESET_E,
	UNIPHY1_XPCS_RESET_E,
	UNIPHY2_SOFT_RESET_E,
	UNIPHY2_XPCS_RESET_E,
	UNIPHY0_PORT1_DISABLE_E,
	UNIPHY0_PORT2_DISABLE_E,
	UNIPHY0_PORT3_DISABLE_E,
	UNIPHY0_PORT4_DISABLE_E,
	UNIPHY0_PORT5_DISABLE_E,
	UNIPHY0_PORT_4_5_RESET_E,
	UNIPHY0_PORT_4_RESET_E,
	UNIPHY_RST_MAX
};

#define CMN_AHB_CLK		"cmn_ahb_clk"
#define CMN_SYS_CLK		"cmn_sys_clk"
#define UNIPHY0_AHB_CLK	"uniphy0_ahb_clk"
#define UNIPHY0_SYS_CLK	"uniphy0_sys_clk"
#define UNIPHY1_AHB_CLK	"uniphy1_ahb_clk"
#define UNIPHY1_SYS_CLK	"uniphy1_sys_clk"
#define UNIPHY2_AHB_CLK	"uniphy2_ahb_clk"
#define UNIPHY2_SYS_CLK	"uniphy2_sys_clk"
#define PORT1_MAC_CLK		"port1_mac_clk"
#define PORT2_MAC_CLK		"port2_mac_clk"
#define PORT3_MAC_CLK		"port3_mac_clk"
#define PORT4_MAC_CLK		"port4_mac_clk"
#define PORT5_MAC_CLK		"port5_mac_clk"
#define PORT6_MAC_CLK		"port6_mac_clk"
#define NSS_PPE_CLK		"nss_ppe_clk"
#define NSS_PPE_CFG_CLK	"nss_ppe_cfg_clk"
#define NSSNOC_PPE_CLK		"nssnoc_ppe_clk"
#define NSSNOC_PPE_CFG_CLK	"nssnoc_ppe_cfg_clk"
#define NSS_EDMA_CLK		"nss_edma_clk"
#define NSS_EDMA_CFG_CLK	"nss_edma_cfg_clk"
#define NSS_PPE_IPE_CLK		"nss_ppe_ipe_clk"
#define NSS_PPE_BTQ_CLK	"nss_ppe_btq_clk"
#define MDIO_AHB_CLK		"gcc_mdio_ahb_clk"
#define NSSNOC_CLK		"gcc_nss_noc_clk"
#define NSSNOC_SNOC_CLK	"gcc_nssnoc_snoc_clk"
#define MEM_NOC_NSSAXI_CLK	"gcc_mem_noc_nss_axi_clk"
#define CRYPTO_PPE_CLK		"gcc_nss_crypto_clk"
#define NSS_IMEM_CLK		"gcc_nss_imem_clk"
#define NSS_PTP_REF_CLK	"gcc_nss_ptp_ref_clk"
#define SNOC_NSSNOC_CLK	"gcc_snoc_nssnoc_clk"

#define NSS_PORT1_RX_CLK	"nss_port1_rx_clk"
#define NSS_PORT1_TX_CLK	"nss_port1_tx_clk"
#define NSS_PORT2_RX_CLK	"nss_port2_rx_clk"
#define NSS_PORT2_TX_CLK	"nss_port2_tx_clk"
#define NSS_PORT3_RX_CLK	"nss_port3_rx_clk"
#define NSS_PORT3_TX_CLK	"nss_port3_tx_clk"
#define NSS_PORT4_RX_CLK	"nss_port4_rx_clk"
#define NSS_PORT4_TX_CLK	"nss_port4_tx_clk"
#define NSS_PORT5_RX_CLK	"nss_port5_rx_clk"
#define NSS_PORT5_TX_CLK	"nss_port5_tx_clk"
#define NSS_PORT6_RX_CLK	"nss_port6_rx_clk"
#define NSS_PORT6_TX_CLK	"nss_port6_tx_clk"
#define UNIPHY0_PORT1_RX_CLK	"uniphy0_port1_rx_clk"
#define UNIPHY0_PORT1_TX_CLK	"uniphy0_port1_tx_clk"
#define UNIPHY0_PORT2_RX_CLK	"uniphy0_port2_rx_clk"
#define UNIPHY0_PORT2_TX_CLK	"uniphy0_port2_tx_clk"
#define UNIPHY0_PORT3_RX_CLK	"uniphy0_port3_rx_clk"
#define UNIPHY0_PORT3_TX_CLK	"uniphy0_port3_tx_clk"
#define UNIPHY0_PORT4_RX_CLK	"uniphy0_port4_rx_clk"
#define UNIPHY0_PORT4_TX_CLK	"uniphy0_port4_tx_clk"
#define UNIPHY0_PORT5_RX_CLK	"uniphy0_port5_rx_clk"
#define UNIPHY0_PORT5_TX_CLK	"uniphy0_port5_tx_clk"
#define UNIPHY1_PORT5_RX_CLK	"uniphy1_port5_rx_clk"
#define UNIPHY1_PORT5_TX_CLK	"uniphy1_port5_tx_clk"
#define UNIPHY2_PORT6_RX_CLK	"uniphy2_port6_rx_clk"
#define UNIPHY2_PORT6_TX_CLK	"uniphy2_port6_tx_clk"
#define PORT5_RX_SRC		"nss_port5_rx_clk_src"
#define PORT5_TX_SRC		"nss_port5_tx_clk_src"

enum unphy_clk_type {
	NSS_PORT1_RX_CLK_E = 0,
	NSS_PORT1_TX_CLK_E,
	NSS_PORT2_RX_CLK_E,
	NSS_PORT2_TX_CLK_E,
	NSS_PORT3_RX_CLK_E,
	NSS_PORT3_TX_CLK_E,
	NSS_PORT4_RX_CLK_E,
	NSS_PORT4_TX_CLK_E,
	NSS_PORT5_RX_CLK_E,
	NSS_PORT5_TX_CLK_E,
	NSS_PORT6_RX_CLK_E,
	NSS_PORT6_TX_CLK_E,
	UNIPHY0_PORT1_RX_CLK_E,
	UNIPHY0_PORT1_TX_CLK_E,
	UNIPHY0_PORT2_RX_CLK_E,
	UNIPHY0_PORT2_TX_CLK_E,
	UNIPHY0_PORT3_RX_CLK_E,
	UNIPHY0_PORT3_TX_CLK_E,
	UNIPHY0_PORT4_RX_CLK_E,
	UNIPHY0_PORT4_TX_CLK_E,
	UNIPHY0_PORT5_RX_CLK_E,
	UNIPHY0_PORT5_TX_CLK_E,
	UNIPHY1_PORT5_RX_CLK_E,
	UNIPHY1_PORT5_TX_CLK_E,
	UNIPHY2_PORT6_RX_CLK_E,
	UNIPHY2_PORT6_TX_CLK_E,
	PORT5_RX_SRC_E,
	PORT5_TX_SRC_E,
	UNIPHYT_CLK_MAX
};

#define UNIPHY_AHB_CLK_RATE	100000000
#define UNIPHY_SYS_CLK_RATE	19200000
#define CPPE_UNIPHY_SYS_CLK_RATE	24000000
#define PPE_CLK_RATE	300000000
#define MDIO_AHB_RATE	100000000
#define NSS_NOC_RATE	461500000
#define NSSNOC_SNOC_RATE	266670000
#define NSS_IMEM_RATE	400000000
#define PTP_REF_RARE	150000000
#define NSS_AXI_RATE	461500000
#define NSS_PORT5_DFLT_RATE 19200000

#define UNIPHY_CLK_RATE_125M		125000000
#define UNIPHY_CLK_RATE_312M		312500000
#define UNIPHY_DEFAULT_RATE		UNIPHY_CLK_RATE_125M

#define PQSGMII_SPEED_10M_CLK		2500000
#define PQSGMII_SPEED_100M_CLK	25000000
#define PQSGMII_SPEED_1000M_CLK	125000000
#define USXGMII_SPEED_10M_CLK		1250000
#define USXGMII_SPEED_100M_CLK	12500000
#define USXGMII_SPEED_1000M_CLK	125000000
#define USXGMII_SPEED_2500M_CLK	78125000
#define USXGMII_SPEED_5000M_CLK	156250000
#define USXGMII_SPEED_10000M_CLK	312500000
#define SGMII_PLUS_SPEED_2500M_CLK	312500000
#define SGMII_SPEED_10M_CLK	2500000
#define SGMII_SPEED_100M_CLK	25000000
#define SGMII_SPEED_1000M_CLK	125000000

#define CPPE_XGMAC_CLK_REG	0x0194900c
#define CPPE_XGMAC_CLK_SIZE	0x10
#define CPPE_XGMAC_CLK_ENABLE	0x20

enum {
	UNIPHY_RX = 0,
	UNIPHY_TX,
};

void ssdk_uniphy_reset(
	a_uint32_t dev_id,
	enum unphy_rst_type rst_type,
	a_uint32_t action);

void ssdk_port_reset(
	a_uint32_t dev_id,
	a_uint32_t port_id,
	a_uint32_t action);

#if defined(HPPE)
void
qca_gcc_mac_port_clock_set(a_uint32_t dev_id, a_uint32_t port_id,
                                a_bool_t enable);

void
qca_gcc_uniphy_port_clock_set(a_uint32_t dev_id, a_uint32_t uniphy_index,
                                a_uint32_t port_id, a_bool_t enable);
void ssdk_ppe_clock_init(void);
void ssdk_uniphy_raw_clock_reset(a_uint8_t uniphy_index);
void ssdk_uniphy_raw_clock_set(
	a_uint8_t uniphy_index,
	a_uint8_t direction,
	a_uint32_t clock);
void
ssdk_port_speed_clock_set(
	a_uint32_t dev_id,
	a_uint32_t port_id,
	a_uint32_t rate);

void ssdk_ppe_reset_init(void);
sw_error_t
qca_cppe_fpga_xgmac_clock_enable(a_uint32_t dev_id);
void ssdk_port_mac_clock_reset(
	a_uint32_t dev_id,
	a_uint32_t port_id);
#endif

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _SSDK_CLK_H */

