cmd_net/netfilter/xt_time.ko := arm-openwrt-linux-uclibcgnueabi-ld -EL -r  -T ./scripts/module-common.lds --build-id  -T ./arch/arm/kernel/module.lds -o net/netfilter/xt_time.ko net/netfilter/xt_time.o net/netfilter/xt_time.mod.o
