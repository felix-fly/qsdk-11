cmd_drivers/usb/phy/phy-msm-qusb.ko := arm-openwrt-linux-uclibcgnueabi-ld -EL -r  -T ./scripts/module-common.lds --build-id  -T ./arch/arm/kernel/module.lds -o drivers/usb/phy/phy-msm-qusb.ko drivers/usb/phy/phy-msm-qusb.o drivers/usb/phy/phy-msm-qusb.mod.o