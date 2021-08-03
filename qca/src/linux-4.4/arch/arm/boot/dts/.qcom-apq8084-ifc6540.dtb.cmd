cmd_arch/arm/boot/dts/qcom-apq8084-ifc6540.dtb := mkdir -p arch/arm/boot/dts/ ; arm-openwrt-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.qcom-apq8084-ifc6540.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.qcom-apq8084-ifc6540.dtb.dts.tmp arch/arm/boot/dts/qcom-apq8084-ifc6540.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/qcom-apq8084-ifc6540.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.qcom-apq8084-ifc6540.dtb.d.dtc.tmp arch/arm/boot/dts/.qcom-apq8084-ifc6540.dtb.dts.tmp ; cat arch/arm/boot/dts/.qcom-apq8084-ifc6540.dtb.d.pre.tmp arch/arm/boot/dts/.qcom-apq8084-ifc6540.dtb.d.dtc.tmp > arch/arm/boot/dts/.qcom-apq8084-ifc6540.dtb.d

source_arch/arm/boot/dts/qcom-apq8084-ifc6540.dtb := arch/arm/boot/dts/qcom-apq8084-ifc6540.dts

deps_arch/arm/boot/dts/qcom-apq8084-ifc6540.dtb := \
  arch/arm/boot/dts/qcom-apq8084.dtsi \
  arch/arm/boot/dts/skeleton.dtsi \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,gcc-apq8084.h \
    $(wildcard include/config/noc/clk/src.h) \
  arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  arch/arm/boot/dts/qcom-pma8084.dtsi \
  arch/arm/boot/dts/include/dt-bindings/iio/qcom,spmi-vadc.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/spmi/spmi.h \

arch/arm/boot/dts/qcom-apq8084-ifc6540.dtb: $(deps_arch/arm/boot/dts/qcom-apq8084-ifc6540.dtb)

$(deps_arch/arm/boot/dts/qcom-apq8084-ifc6540.dtb):
