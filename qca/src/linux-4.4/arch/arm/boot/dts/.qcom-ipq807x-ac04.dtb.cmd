cmd_arch/arm/boot/dts/qcom-ipq807x-ac04.dtb := mkdir -p arch/arm/boot/dts/ ; arm-openwrt-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.qcom-ipq807x-ac04.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.qcom-ipq807x-ac04.dtb.dts.tmp arch/arm/boot/dts/qcom-ipq807x-ac04.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/qcom-ipq807x-ac04.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.qcom-ipq807x-ac04.dtb.d.dtc.tmp arch/arm/boot/dts/.qcom-ipq807x-ac04.dtb.dts.tmp ; cat arch/arm/boot/dts/.qcom-ipq807x-ac04.dtb.d.pre.tmp arch/arm/boot/dts/.qcom-ipq807x-ac04.dtb.d.dtc.tmp > arch/arm/boot/dts/.qcom-ipq807x-ac04.dtb.d

source_arch/arm/boot/dts/qcom-ipq807x-ac04.dtb := arch/arm/boot/dts/qcom-ipq807x-ac04.dts

deps_arch/arm/boot/dts/qcom-ipq807x-ac04.dtb := \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-ac04.dts \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-soc.dtsi \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,gcc-ipq807x.h \
  arch/arm/boot/dts/include/dt-bindings/reset/qcom,gcc-ipq807x.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/clock/qca,apss-ipq807x.h \
  arch/arm/boot/dts/include/dt-bindings/input/input.h \
  arch/arm/boot/dts/include/dt-bindings/input/linux-event-codes.h \
  arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-mhi.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-memory.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-coresight.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-spmi-regulator.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-regulator.dtsi \
  arch/arm/boot/dts/include/dt-bindings/spmi/spmi.h \
  arch/arm/boot/dts/include/dt-bindings/iio/qcom,spmi-vadc.h \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-rpm-regulator.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-thermal.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-cpr-regulator.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq807x-ac-cpu.dtsi \

arch/arm/boot/dts/qcom-ipq807x-ac04.dtb: $(deps_arch/arm/boot/dts/qcom-ipq807x-ac04.dtb)

$(deps_arch/arm/boot/dts/qcom-ipq807x-ac04.dtb):
