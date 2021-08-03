cmd_arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dtb := mkdir -p arch/arm/boot/dts/ ; arm-openwrt-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.qcom-ipq4019-ap.dk07.1-c1.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.qcom-ipq4019-ap.dk07.1-c1.dtb.dts.tmp arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.qcom-ipq4019-ap.dk07.1-c1.dtb.d.dtc.tmp arch/arm/boot/dts/.qcom-ipq4019-ap.dk07.1-c1.dtb.dts.tmp ; cat arch/arm/boot/dts/.qcom-ipq4019-ap.dk07.1-c1.dtb.d.pre.tmp arch/arm/boot/dts/.qcom-ipq4019-ap.dk07.1-c1.dtb.d.dtc.tmp > arch/arm/boot/dts/.qcom-ipq4019-ap.dk07.1-c1.dtb.d

source_arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dtb := arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dts

deps_arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dtb := \
  arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1.dtsi \
  arch/arm/boot/dts/qcom-ipq4019.dtsi \
  arch/arm/boot/dts/skeleton.dtsi \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,gcc-ipq4019.h \
  arch/arm/boot/dts/include/dt-bindings/reset/qcom,gcc-ipq4019.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/soc/ipq,tcsr.h \
  arch/arm/boot/dts/include/dt-bindings/input/input.h \
  arch/arm/boot/dts/include/dt-bindings/input/linux-event-codes.h \
  arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \

arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dtb: $(deps_arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dtb)

$(deps_arch/arm/boot/dts/qcom-ipq4019-ap.dk07.1-c1.dtb):
