cmd_/home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/tc_ematch/.install := bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/tc_ematch ./include/uapi/linux/tc_ematch tc_em_cmp.h tc_em_meta.h tc_em_nbyte.h tc_em_text.h; bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/tc_ematch ./include/linux/tc_ematch ; bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/tc_ematch ./include/generated/uapi/linux/tc_ematch ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/tc_ematch/$$F; done; touch /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/linux/tc_ematch/.install