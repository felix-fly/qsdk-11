cmd_lib/crc-ccitt.ko := arm-openwrt-linux-uclibcgnueabi-ld -EL -r  -T ./scripts/module-common.lds --build-id  -T ./arch/arm/kernel/module.lds -o lib/crc-ccitt.ko lib/crc-ccitt.o lib/crc-ccitt.mod.o
