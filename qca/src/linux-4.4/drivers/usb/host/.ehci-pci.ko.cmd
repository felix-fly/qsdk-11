cmd_drivers/usb/host/ehci-pci.ko := arm-openwrt-linux-uclibcgnueabi-ld -EL -r  -T ./scripts/module-common.lds --build-id  -T ./arch/arm/kernel/module.lds -o drivers/usb/host/ehci-pci.ko drivers/usb/host/ehci-pci.o drivers/usb/host/ehci-pci.mod.o
