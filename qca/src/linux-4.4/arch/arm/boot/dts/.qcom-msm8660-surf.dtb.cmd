cmd_arch/arm/boot/dts/qcom-msm8660-surf.dtb := mkdir -p arch/arm/boot/dts/ ; arm-openwrt-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.qcom-msm8660-surf.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.qcom-msm8660-surf.dtb.dts.tmp arch/arm/boot/dts/qcom-msm8660-surf.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/qcom-msm8660-surf.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.qcom-msm8660-surf.dtb.d.dtc.tmp arch/arm/boot/dts/.qcom-msm8660-surf.dtb.dts.tmp ; cat arch/arm/boot/dts/.qcom-msm8660-surf.dtb.d.pre.tmp arch/arm/boot/dts/.qcom-msm8660-surf.dtb.d.dtc.tmp > arch/arm/boot/dts/.qcom-msm8660-surf.dtb.d

source_arch/arm/boot/dts/qcom-msm8660-surf.dtb := arch/arm/boot/dts/qcom-msm8660-surf.dts

deps_arch/arm/boot/dts/qcom-msm8660-surf.dtb := \
  arch/arm/boot/dts/include/dt-bindings/input/input.h \
  arch/arm/boot/dts/include/dt-bindings/input/linux-event-codes.h \
  arch/arm/boot/dts/qcom-msm8660.dtsi \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/clock/qcom,gcc-msm8660.h \
  arch/arm/boot/dts/include/dt-bindings/soc/qcom,gsbi.h \
  arch/arm/boot/dts/skeleton.dtsi \

arch/arm/boot/dts/qcom-msm8660-surf.dtb: $(deps_arch/arm/boot/dts/qcom-msm8660-surf.dtb)

$(deps_arch/arm/boot/dts/qcom-msm8660-surf.dtb):
