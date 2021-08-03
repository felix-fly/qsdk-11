ccflags-y := -Werror
ccflags-y += -I$(obj)/include
ccflags-y += -I$(obj)/../exports
ccflags-y += -DNSS_NL_DEBUG_LEVEL=4
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"

ccflags-y += -DCONFIG_NSS_NLIPV4=1
ccflags-y += -DCONFIG_NSS_NLIPV6=1
ccflags-y += -DCONFIG_NSS_NLIPSEC=1
ccflags-y += -DCONFIG_NSS_NLOAM=1
ccflags-y += -DCONFIG_NSS_NLGRE_REDIR=1

qca-nss-netlink-objs := nss_nl.o
qca-nss-netlink-objs += nss_nlgre_redir.o
qca-nss-netlink-objs += nss_nlgre_redir_mgr.o
qca-nss-netlink-objs += nss_nlipv4.o
qca-nss-netlink-objs += nss_nlipv6.o
qca-nss-netlink-objs += nss_nlipsec.o
qca-nss-netlink-objs += nss_nloam.o
obj-m += qca-nss-netlink.o

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ccflags-y += -DCONFIG_NSS_NLCRYPTOV2=1
qca-nss-netlink-objs += nss_nlcryptov2.o
else
ccflags-y += -DCONFIG_NSS_NLCRYPTO=1
qca-nss-netlink-objs += nss_nlcrypto.o
endif
