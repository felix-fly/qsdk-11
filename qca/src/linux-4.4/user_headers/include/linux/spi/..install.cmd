cmd_/home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/spi/.install := bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/spi ./include/uapi/linux/spi spidev.h; bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/spi ./include/linux/spi ; bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/spi ./include/generated/uapi/linux/spi ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/spi/$$F; done; touch /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/spi/.install
