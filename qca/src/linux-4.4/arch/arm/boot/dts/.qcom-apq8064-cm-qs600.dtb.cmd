cmd_arch/arm/boot/dts/qcom-apq8064-cm-qs600.dtb := mkdir -p arch/arm/boot/dts/ ; arm-openwrt-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.qcom-apq8064-cm-qs600.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.qcom-apq8064-cm-qs600.dtb.dts.tmp arch/arm/boot/dts/qcom-apq8064-cm-qs600.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/qcom-apq8064-cm-qs600.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.qcom-apq8064-cm-qs600.dtb.d.dtc.tmp arch/arm/boot/dts/.qcom-apq8064-cm-qs600.dtb.dts.tmp ; cat arch/arm/boot/dts/.qcom-apq8064-cm-qs600.dtb.d.pre.tmp arch/arm/boot/dts/.qcom-apq8064-cm-qs600.dtb.d.dtc.tmp > arch/arm/boot/dts/.qcom-apq8064-cm-qs600.dtb.d

source_arch/arm/boot/dts/qcom-apq8064-cm-qs600.dtb := arch/arm/boot/dts/qcom-apq8064-cm-qs600.dts

deps_arch/arm/boot/dts/qcom-apq8064-cm-qs600.dtb := \
  arch/arm/boot/dts/qcom-apq8064-v2.0.dtsi \
  arch/arm/boot/dts/qcom-apq8064.dtsi \
  arch/arm/boot/dts/skeleton.dtsi \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,gcc-msm8960.h \
  arch/arm/boot/dts/include/dt-bindings/reset/qcom,gcc-msm8960.h \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,mmcc-msm8960.h \
  arch/arm/boot/dts/include/dt-bindings/soc/qcom,gsbi.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  arch/arm/boot/dts/include/dt-bindings/pinctrl/qcom,pmic-gpio.h \

arch/arm/boot/dts/qcom-apq8064-cm-qs600.dtb: $(deps_arch/arm/boot/dts/qcom-apq8064-cm-qs600.dtb)

$(deps_arch/arm/boot/dts/qcom-apq8064-cm-qs600.dtb):
