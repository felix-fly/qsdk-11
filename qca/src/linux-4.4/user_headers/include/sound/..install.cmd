cmd_/home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/sound/.install := bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/sound ./include/uapi/sound asequencer.h asound.h asound_fm.h compress_offload.h compress_params.h emu10k1.h firewire.h hdsp.h hdspm.h sb16_csp.h sfnt_info.h; bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/sound ./include/sound ; bash scripts/headers_install.sh /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/sound ./include/generated/uapi/sound ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/sound/$$F; done; touch /home/ubuntu/qsdk/build_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/linux-ipq_ipq60xx/linux-4.4.60/user_headers/include/sound/.install
