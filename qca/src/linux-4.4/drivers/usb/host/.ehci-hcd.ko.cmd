cmd_drivers/usb/host/ehci-hcd.ko := arm-openwrt-linux-uclibcgnueabi-ld -EL -r  -T ./scripts/module-common.lds --build-id  -T ./arch/arm/kernel/module.lds -o drivers/usb/host/ehci-hcd.ko drivers/usb/host/ehci-hcd.o drivers/usb/host/ehci-hcd.mod.o
