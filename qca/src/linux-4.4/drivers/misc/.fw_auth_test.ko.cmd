cmd_drivers/misc/fw_auth_test.ko := arm-openwrt-linux-uclibcgnueabi-ld -EL -r  -T ./scripts/module-common.lds --build-id  -T ./arch/arm/kernel/module.lds -o drivers/misc/fw_auth_test.ko drivers/misc/fw_auth_test.o drivers/misc/fw_auth_test.mod.o