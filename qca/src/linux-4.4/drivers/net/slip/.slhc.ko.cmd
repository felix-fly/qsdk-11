cmd_drivers/net/slip/slhc.ko := arm-openwrt-linux-uclibcgnueabi-ld -EL -r  -T ./scripts/module-common.lds --build-id  -T ./arch/arm/kernel/module.lds -o drivers/net/slip/slhc.ko drivers/net/slip/slhc.o drivers/net/slip/slhc.mod.o