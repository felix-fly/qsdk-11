cmd_arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dtb := mkdir -p arch/arm/boot/dts/ ; arm-openwrt-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.qcom-ipq6018-cp02-c1.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.qcom-ipq6018-cp02-c1.dtb.dts.tmp arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.qcom-ipq6018-cp02-c1.dtb.d.dtc.tmp arch/arm/boot/dts/.qcom-ipq6018-cp02-c1.dtb.dts.tmp ; cat arch/arm/boot/dts/.qcom-ipq6018-cp02-c1.dtb.d.pre.tmp arch/arm/boot/dts/.qcom-ipq6018-cp02-c1.dtb.d.dtc.tmp > arch/arm/boot/dts/.qcom-ipq6018-cp02-c1.dtb.d

source_arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dtb := arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dts

deps_arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dtb := \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq6018-cp02-c1.dts \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq6018.dtsi \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,gcc-ipq6018.h \
  arch/arm/boot/dts/include/dt-bindings/reset/qcom,gcc-ipq6018.h \
  arch/arm/boot/dts/include/dt-bindings/clock/qca,apss-ipq6018.h \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq6018-memory.dtsi \
  arch/arm/boot/dts/include/dt-bindings/pinctrl/qcom,pmic-gpio.h \
  arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq6018-coresight.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq6018-rpm-regulator.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq6018-cpr-regulator.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq6018-cp-cpu.dtsi \
  arch/arm/boot/dts/../../../arm64/boot/dts/qcom/qcom-ipq6018-thermal.dtsi \
  arch/arm/boot/dts/qcom-ipq6018.dtsi \

arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dtb: $(deps_arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dtb)

$(deps_arch/arm/boot/dts/qcom-ipq6018-cp02-c1.dtb):
