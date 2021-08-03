cmd_arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dtb := mkdir -p arch/arm/boot/dts/ ; arm-openwrt-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.qcom-msm8974-sony-xperia-honami.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.qcom-msm8974-sony-xperia-honami.dtb.dts.tmp arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.qcom-msm8974-sony-xperia-honami.dtb.d.dtc.tmp arch/arm/boot/dts/.qcom-msm8974-sony-xperia-honami.dtb.dts.tmp ; cat arch/arm/boot/dts/.qcom-msm8974-sony-xperia-honami.dtb.d.pre.tmp arch/arm/boot/dts/.qcom-msm8974-sony-xperia-honami.dtb.d.dtc.tmp > arch/arm/boot/dts/.qcom-msm8974-sony-xperia-honami.dtb.d

source_arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dtb := arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dts

deps_arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dtb := \
  arch/arm/boot/dts/qcom-msm8974.dtsi \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,gcc-msm8974.h \
    $(wildcard include/config/noc/clk/src.h) \
  arch/arm/boot/dts/skeleton.dtsi \
  arch/arm/boot/dts/qcom-pm8841.dtsi \
  arch/arm/boot/dts/include/dt-bindings/spmi/spmi.h \
  arch/arm/boot/dts/qcom-pm8941.dtsi \
  arch/arm/boot/dts/include/dt-bindings/iio/qcom,spmi-vadc.h \

arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dtb: $(deps_arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dtb)

$(deps_arch/arm/boot/dts/qcom-msm8974-sony-xperia-honami.dtb):
