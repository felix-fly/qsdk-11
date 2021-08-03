cmd_arch/arm/boot/compressed/piggy.xzkern.o := arm-openwrt-linux-uclibcgnueabi-gcc -Wp,-MD,arch/arm/boot/compressed/.piggy.xzkern.o.d  -nostdinc -isystem /home/ubuntu/qsdk/staging_dir/toolchain-arm_cortex-a7_gcc-4.8-linaro_uClibc-1.0.14_eabi/lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.8.3/include -I./arch/arm/include -Iarch/arm/include/generated/uapi -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian   -D__ASSEMBLY__ -fno-PIE -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float -Wa,-gdwarf-2 -DCC_HAVE_ASM_GOTO        -DZIMAGE     -c -o arch/arm/boot/compressed/piggy.xzkern.o arch/arm/boot/compressed/piggy.xzkern.S

source_arch/arm/boot/compressed/piggy.xzkern.o := arch/arm/boot/compressed/piggy.xzkern.S

deps_arch/arm/boot/compressed/piggy.xzkern.o := \
  arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/cpu/v7m.h) \
    $(wildcard include/config/thumb2/kernel.h) \

arch/arm/boot/compressed/piggy.xzkern.o: $(deps_arch/arm/boot/compressed/piggy.xzkern.o)

$(deps_arch/arm/boot/compressed/piggy.xzkern.o):
