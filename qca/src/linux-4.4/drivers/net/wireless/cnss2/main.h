/* Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _CNSS_MAIN_H
#define _CNSS_MAIN_H

#include <linux/esoc_client.h>
#include <linux/etherdevice.h>
#include <linux/msm-bus.h>
#include <linux/pm_qos.h>
#include <net/cnss2.h>
#include <soc/qcom/memory_dump.h>
#include <soc/qcom/subsystem_restart.h>

#include "qmi.h"

#define MAX_NO_OF_MAC_ADDR		4
#define QMI_WLFW_MAX_NUM_MEM_SEG 32

struct cnss_vreg_info {
	struct regulator *reg;
	const char *name;
	u32 min_uv;
	u32 max_uv;
	u32 load_ua;
	u32 delay_us;
};

struct cnss_pinctrl_info {
	struct pinctrl *pinctrl;
	struct pinctrl_state *bootstrap_active;
	struct pinctrl_state *wlan_en_active;
	struct pinctrl_state *wlan_en_sleep;
};

struct cnss_subsys_info {
	struct subsys_device *subsys_device;
	struct subsys_desc subsys_desc;
	void *subsys_handle;
	bool subsystem_put_in_progress;
};

struct cnss_ramdump_info {
	struct ramdump_device *ramdump_dev;
	unsigned long ramdump_size;
	void *ramdump_va;
	phys_addr_t ramdump_pa;
	struct msm_dump_data dump_data;
};

struct cnss_dump_seg {
	unsigned long address;
	void *v_address;
	unsigned long size;
	u32 type;
};

struct cnss_dump_data {
	u32 version;
	u32 magic;
	char name[32];
	phys_addr_t paddr;
	int nentries;
	u32 seg_version;
};

struct cnss_ramdump_info_v2 {
	struct ramdump_device *ramdump_dev;
	unsigned long ramdump_size;
	void *dump_data_vaddr;
	bool dump_data_valid;
	struct cnss_dump_data dump_data;
};

struct cnss_esoc_info {
	struct esoc_desc *esoc_desc;
	bool notify_modem_status;
	void *modem_notify_handler;
	int modem_current_status;
};

struct cnss_bus_bw_info {
	struct msm_bus_scale_pdata *bus_scale_table;
	u32 bus_client;
	int current_bw_vote;
};

struct cnss_wlan_mac_addr {
	u8 mac_addr[MAX_NO_OF_MAC_ADDR][ETH_ALEN];
	u32 no_of_mac_addr_set;
};

struct cnss_wlan_mac_info {
	struct cnss_wlan_mac_addr wlan_mac_addr;
	bool is_wlan_mac_set;
};

struct cnss_fw_mem {
	size_t size;
	void *va;
	phys_addr_t pa;
	bool valid;
	u32 type;
};

enum cnss_driver_event_type {
	CNSS_DRIVER_EVENT_SERVER_ARRIVE,
	CNSS_DRIVER_EVENT_SERVER_EXIT,
	CNSS_DRIVER_EVENT_REQUEST_MEM,
	CNSS_DRIVER_EVENT_FW_MEM_READY,
	CNSS_DRIVER_EVENT_FW_READY,
	CNSS_DRIVER_EVENT_COLD_BOOT_CAL_START,
	CNSS_DRIVER_EVENT_COLD_BOOT_CAL_DONE,
	CNSS_DRIVER_EVENT_REGISTER_DRIVER,
	CNSS_DRIVER_EVENT_UNREGISTER_DRIVER,
	CNSS_DRIVER_EVENT_RECOVERY,
	CNSS_DRIVER_EVENT_FORCE_FW_ASSERT,
	CNSS_DRIVER_EVENT_POWER_UP,
	CNSS_DRIVER_EVENT_POWER_DOWN,
	CNSS_DRIVER_EVENT_QDSS_TRACE_REQ_MEM,
	CNSS_DRIVER_EVENT_QDSS_TRACE_SAVE,
	CNSS_DRIVER_EVENT_QDSS_TRACE_FREE,
	CNSS_DRIVER_EVENT_MAX,
};

enum cnss_driver_state {
	CNSS_QMI_WLFW_CONNECTED,
	CNSS_FW_MEM_READY,
	CNSS_FW_READY,
	CNSS_COLD_BOOT_CAL,
	CNSS_DRIVER_LOADING,
	CNSS_DRIVER_UNLOADING,
	CNSS_DRIVER_PROBED,
	CNSS_DRIVER_RECOVERY,
	CNSS_FW_BOOT_RECOVERY,
	CNSS_DEV_ERR_NOTIFY,
	CNSS_DRIVER_DEBUG,
};

struct cnss_recovery_data {
	enum cnss_recovery_reason reason;
};

enum cnss_pins {
	CNSS_WLAN_EN,
	CNSS_PCIE_TXP,
	CNSS_PCIE_TXN,
	CNSS_PCIE_RXP,
	CNSS_PCIE_RXN,
	CNSS_PCIE_REFCLKP,
	CNSS_PCIE_REFCLKN,
	CNSS_PCIE_RST,
	CNSS_PCIE_WAKE,
};

struct cnss_pin_connect_result {
	u32 fw_pwr_pin_result;
	u32 fw_phy_io_pin_result;
	u32 fw_rf_pin_result;
	u32 host_pin_result;
};

struct cnss_plat_data {
	void *wlan_priv;
	struct platform_device *plat_dev;
	struct platform_device_id *plat_dev_id;
	void *pci_dev;
	void *pci_dev_id;
	void *bus_priv;
	struct cnss_vreg_info *vreg_info;
	struct cnss_pinctrl_info pinctrl_info;
	struct cnss_subsys_info subsys_info;
	struct cnss_ramdump_info ramdump_info;
	struct cnss_ramdump_info_v2 ramdump_info_v2;
	struct cnss_esoc_info esoc_info;
	struct cnss_bus_bw_info bus_bw_info;
	struct notifier_block modem_nb;
	struct cnss_platform_cap cap;
	struct pm_qos_request qos_request;
	unsigned long device_id;
	struct cnss_wlan_driver *driver_ops;
	enum cnss_driver_status driver_status;
	u32 recovery_count;
	unsigned long driver_state;
	struct list_head event_list;
	spinlock_t event_lock; /* spinlock for driver work event handling */
	struct work_struct event_work;
	struct workqueue_struct *event_wq;
	struct workqueue_struct *qmi_resp_wq;
	struct qmi_handle *qmi_wlfw_clnt;
	struct work_struct qmi_recv_msg_work;
	struct notifier_block qmi_wlfw_clnt_nb;
	struct wlfw_rf_chip_info_s_v01 chip_info;
	struct wlfw_rf_board_info_s_v01 board_info;
	struct wlfw_soc_info_s_v01 soc_info;
	struct wlfw_fw_version_info_s_v01 fw_version_info;
	u32 fw_mem_seg_len;
	struct cnss_fw_mem fw_mem[QMI_WLFW_MAX_NUM_MEM_SEG_V01];
	struct cnss_fw_mem m3_mem;
	u32 qdss_mem_seg_len;
	struct cnss_fw_mem qdss_mem[QMI_WLFW_MAX_NUM_MEM_SEG];
	int tgt_mem_cfg_mode;
	struct cnss_pin_connect_result pin_result;
	struct dentry *root_dentry;
	atomic_t pm_count;
	struct timer_list fw_boot_timer;
	struct completion power_up_complete;
	unsigned int wlfw_service_instance_id;
	unsigned int service_id;
	struct notifier_block modem_atomic_nb;
	bool cal_done;
	int table_index;
};

int cnss_driver_event_post(struct cnss_plat_data *plat_priv,
			   enum cnss_driver_event_type type,
			   bool sync, void *data);
int cnss_get_vreg(struct cnss_plat_data *plat_priv);
int cnss_get_pinctrl(struct cnss_plat_data *plat_priv);
int cnss_register_subsys(struct cnss_plat_data *plat_priv);
void cnss_unregister_subsys(struct cnss_plat_data *plat_priv);
int cnss_register_ramdump(struct cnss_plat_data *plat_priv);
void cnss_unregister_ramdump(struct cnss_plat_data *plat_priv);
void cnss_set_pin_connect_status(struct cnss_plat_data *plat_priv);
u32 cnss_get_wake_msi(struct cnss_plat_data *plat_priv);
struct cnss_plat_data *cnss_get_plat_priv_by_device_id(int id);
struct cnss_plat_data *cnss_get_plat_priv(struct platform_device *plat_dev);
int cnss_qca6290_shutdown_part2(struct cnss_plat_data *plat_priv);
#endif /* _CNSS_MAIN_H */
