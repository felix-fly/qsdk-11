cmd_arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dtb := mkdir -p arch/arm/boot/dts/ ; arm-openwrt-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.qcom-ipq4019-ap.dk04.1-c5.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.qcom-ipq4019-ap.dk04.1-c5.dtb.dts.tmp arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.qcom-ipq4019-ap.dk04.1-c5.dtb.d.dtc.tmp arch/arm/boot/dts/.qcom-ipq4019-ap.dk04.1-c5.dtb.dts.tmp ; cat arch/arm/boot/dts/.qcom-ipq4019-ap.dk04.1-c5.dtb.d.pre.tmp arch/arm/boot/dts/.qcom-ipq4019-ap.dk04.1-c5.dtb.d.dtc.tmp > arch/arm/boot/dts/.qcom-ipq4019-ap.dk04.1-c5.dtb.d

source_arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dtb := arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dts

deps_arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dtb := \
  arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1.dtsi \
  arch/arm/boot/dts/qcom-ipq4019.dtsi \
  arch/arm/boot/dts/skeleton.dtsi \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,gcc-ipq4019.h \
  arch/arm/boot/dts/include/dt-bindings/reset/qcom,gcc-ipq4019.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/soc/ipq,tcsr.h \
  arch/arm/boot/dts/qcom-ipq4019-mhi.dtsi \
  arch/arm/boot/dts/include/dt-bindings/input/input.h \
  arch/arm/boot/dts/include/dt-bindings/input/linux-event-codes.h \
  arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \

arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dtb: $(deps_arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dtb)

$(deps_arch/arm/boot/dts/qcom-ipq4019-ap.dk04.1-c5.dtb):