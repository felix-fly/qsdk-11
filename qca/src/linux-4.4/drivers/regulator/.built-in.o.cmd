cmd_drivers/regulator/built-in.o :=  arm-openwrt-linux-uclibcgnueabi-ld -EL    -r -o drivers/regulator/built-in.o drivers/regulator/core.o drivers/regulator/dummy.o drivers/regulator/fixed-helper.o drivers/regulator/helpers.o drivers/regulator/devres.o drivers/regulator/of_regulator.o drivers/regulator/fixed.o drivers/regulator/cpr3-regulator.o drivers/regulator/cpr3-util.o drivers/regulator/cpr3-npu-regulator.o drivers/regulator/cpr4-apss-regulator.o drivers/regulator/gpio-regulator.o drivers/regulator/qcom_rpm-regulator.o drivers/regulator/qcom_spmi-regulator.o 