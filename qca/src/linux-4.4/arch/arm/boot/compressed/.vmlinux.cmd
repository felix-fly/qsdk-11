cmd_arch/arm/boot/compressed/vmlinux := arm-openwrt-linux-uclibcgnueabi-ld -EL    --defsym _kernel_bss_size=345032 --defsym zreladdr=0x42208000 -p --no-undefined -X -T arch/arm/boot/compressed/vmlinux.lds arch/arm/boot/compressed/head.o arch/arm/boot/compressed/piggy.xzkern.o arch/arm/boot/compressed/misc.o arch/arm/boot/compressed/decompress.o arch/arm/boot/compressed/string.o arch/arm/boot/compressed/hyp-stub.o arch/arm/boot/compressed/lib1funcs.o arch/arm/boot/compressed/ashldi3.o arch/arm/boot/compressed/bswapsdi2.o -o arch/arm/boot/compressed/vmlinux
