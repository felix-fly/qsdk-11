cmd_drivers/ata/libahci_platform.ko := arm-openwrt-linux-uclibcgnueabi-ld -EL -r  -T ./scripts/module-common.lds --build-id  -T ./arch/arm/kernel/module.lds -o drivers/ata/libahci_platform.ko drivers/ata/libahci_platform.o drivers/ata/libahci_platform.mod.o