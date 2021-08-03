	.arch armv7-a
	.fpu softvfp
	.eabi_attribute 20, 1	@ Tag_ABI_FP_denormal
	.eabi_attribute 21, 1	@ Tag_ABI_FP_exceptions
	.eabi_attribute 23, 3	@ Tag_ABI_FP_number_model
	.eabi_attribute 24, 1	@ Tag_ABI_align8_needed
	.eabi_attribute 25, 1	@ Tag_ABI_align8_preserved
	.eabi_attribute 26, 2	@ Tag_ABI_enum_size
	.eabi_attribute 30, 4	@ Tag_ABI_optimization_goals
	.eabi_attribute 34, 1	@ Tag_CPU_unaligned_access
	.eabi_attribute 18, 4	@ Tag_ABI_PCS_wchar_t
	.file	"asm-offsets.c"
@ GNU C (OpenWrt/Linaro GCC 4.8-2014.04 35ebd114e275+r49254) version 4.8.3 (arm-openwrt-linux-uclibcgnueabi)
@	compiled by GNU C version 7.5.0, GMP version 5.1.3, MPFR version 3.1.2, MPC version 1.0.2
@ GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
@ options passed:  -nostdinc -I ./arch/arm/include
@ -I arch/arm/include/generated/uapi -I arch/arm/include/generated
@ -I include -I ./arch/arm/include/uapi -I arch/arm/include/generated/uapi
@ -I ./include/uapi -I include/generated/uapi
@ -idirafter /home/ubuntu/qsdk/staging_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/usr/include
@ -D __KERNEL__ -D __LINUX_ARM_ARCH__=7 -U arm -D CC_HAVE_ASM_GOTO
@ -D KBUILD_STR(s)=#s -D KBUILD_BASENAME=KBUILD_STR(asm_offsets)
@ -D KBUILD_MODNAME=KBUILD_STR(asm_offsets)
@ -isystem /home/ubuntu/qsdk/staging_dir/toolchain-arm_cortex-a7_gcc-4.8-linaro_uClibc-1.0.14_eabi/lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.8.3/include
@ -include ./include/linux/kconfig.h -MD arch/arm/kernel/.asm-offsets.s.d
@ arch/arm/kernel/asm-offsets.c -mlittle-endian -mabi=aapcs-linux
@ -mno-thumb-interwork -mfpu=vfp -marm -march=armv7-a -mfloat-abi=soft
@ -mtls-dialect=gnu -auxbase-strip arch/arm/kernel/asm-offsets.s -g -Os
@ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs
@ -Werror=implicit-function-declaration -Wno-format-security
@ -Wno-maybe-uninitialized -Wframe-larger-than=1024
@ -Wno-unused-but-set-variable -Wdeclaration-after-statement
@ -Wno-pointer-sign -Werror=implicit-int -Werror=strict-prototypes -Werror
@ -std=gnu90 -fno-strict-aliasing -fno-common -fno-PIE -fno-dwarf2-cfi-asm
@ -fno-ipa-sra -funwind-tables -fno-delete-null-pointer-checks
@ -fno-caller-saves -fno-stack-protector -fomit-frame-pointer
@ -fno-var-tracking-assignments -femit-struct-debug-baseonly
@ -fno-var-tracking -fno-strict-overflow -fconserve-stack
@ -ffunction-sections -fdata-sections -fverbose-asm
@ --param allow-store-data-races=0
@ options enabled:  -faggressive-loop-optimizations -fauto-inc-dec
@ -fbranch-count-reg -fcombine-stack-adjustments -fcompare-elim
@ -fcprop-registers -fcrossjumping -fcse-follow-jumps -fdata-sections
@ -fdefer-pop -fdevirtualize -fearly-inlining
@ -feliminate-unused-debug-types -fexpensive-optimizations
@ -fforward-propagate -ffunction-cse -ffunction-sections -fgcse -fgcse-lm
@ -fgnu-runtime -fguess-branch-probability -fhoist-adjacent-loads -fident
@ -fif-conversion -fif-conversion2 -findirect-inlining -finline
@ -finline-atomics -finline-functions -finline-functions-called-once
@ -finline-small-functions -fipa-cp -fipa-profile -fipa-pure-const
@ -fipa-reference -fira-hoist-pressure -fira-share-save-slots
@ -fira-share-spill-slots -fivopts -fkeep-static-consts
@ -fleading-underscore -fmath-errno -fmerge-constants -fmerge-debug-strings
@ -fmove-loop-invariants -fomit-frame-pointer -foptimize-register-move
@ -foptimize-sibling-calls -fpartial-inlining -fpeephole -fpeephole2 -fplt
@ -fprefetch-loop-arrays -freg-struct-return -fregmove -freorder-blocks
@ -freorder-functions -frerun-cse-after-loop
@ -fsched-critical-path-heuristic -fsched-dep-count-heuristic
@ -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
@ -fsched-pressure -fsched-rank-heuristic -fsched-spec
@ -fsched-spec-insn-heuristic -fsched-stalled-insns-dep -fschedule-insns2
@ -fsection-anchors -fshow-column -fshrink-wrap -fsigned-zeros
@ -fsplit-ivs-in-unroller -fsplit-wide-types -fstrict-volatile-bitfields
@ -fsync-libcalls -fthread-jumps -ftoplevel-reorder -ftrapping-math
@ -ftree-bit-ccp -ftree-builtin-call-dce -ftree-ccp -ftree-ch
@ -ftree-coalesce-vars -ftree-copy-prop -ftree-copyrename -ftree-cselim
@ -ftree-dce -ftree-dominator-opts -ftree-dse -ftree-forwprop -ftree-fre
@ -ftree-loop-if-convert -ftree-loop-im -ftree-loop-ivcanon
@ -ftree-loop-optimize -ftree-parallelize-loops= -ftree-phiprop -ftree-pre
@ -ftree-pta -ftree-reassoc -ftree-scev-cprop -ftree-sink
@ -ftree-slp-vectorize -ftree-slsr -ftree-sra -ftree-switch-conversion
@ -ftree-tail-merge -ftree-ter -ftree-vrp -funit-at-a-time -funwind-tables
@ -fverbose-asm -fzero-initialized-in-bss -marm -mlittle-endian
@ -msched-prolog -muclibc -munaligned-access -mvectorize-with-neon-quad

	.text
.Ltext0:
#APP
	.macro	it, cond
	.endm
	.macro	itt, cond
	.endm
	.macro	ite, cond
	.endm
	.macro	ittt, cond
	.endm
	.macro	itte, cond
	.endm
	.macro	itet, cond
	.endm
	.macro	itee, cond
	.endm
	.macro	itttt, cond
	.endm
	.macro	ittte, cond
	.endm
	.macro	ittet, cond
	.endm
	.macro	ittee, cond
	.endm
	.macro	itett, cond
	.endm
	.macro	itete, cond
	.endm
	.macro	iteet, cond
	.endm
	.macro	iteee, cond
	.endm

	.section	.text.startup.main,"ax",%progbits
	.align	2
	.global	main
	.type	main, %function
main:
	.fnstart
.LFB1870:
	.file 1 "arch/arm/kernel/asm-offsets.c"
	.loc 1 60 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 61 0
#APP
@ 61 "arch/arm/kernel/asm-offsets.c" 1
	
->TSK_ACTIVE_MM #412 offsetof(struct task_struct, active_mm)	@
@ 0 "" 2
	.loc 1 65 0
@ 65 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 66 0
@ 66 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_FLAGS #0 offsetof(struct thread_info, flags)	@
@ 0 "" 2
	.loc 1 67 0
@ 67 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_PREEMPT #4 offsetof(struct thread_info, preempt_count)	@
@ 0 "" 2
	.loc 1 68 0
@ 68 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_ADDR_LIMIT #8 offsetof(struct thread_info, addr_limit)	@
@ 0 "" 2
	.loc 1 69 0
@ 69 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_TASK #12 offsetof(struct thread_info, task)	@
@ 0 "" 2
	.loc 1 70 0
@ 70 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_CPU #16 offsetof(struct thread_info, cpu)	@
@ 0 "" 2
	.loc 1 71 0
@ 71 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_CPU_DOMAIN #20 offsetof(struct thread_info, cpu_domain)	@
@ 0 "" 2
	.loc 1 72 0
@ 72 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_CPU_SAVE #24 offsetof(struct thread_info, cpu_context)	@
@ 0 "" 2
	.loc 1 73 0
@ 73 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_USED_CP #76 offsetof(struct thread_info, used_cp)	@
@ 0 "" 2
	.loc 1 74 0
@ 74 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_TP_VALUE #92 offsetof(struct thread_info, tp_value)	@
@ 0 "" 2
	.loc 1 75 0
@ 75 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_FPSTATE #104 offsetof(struct thread_info, fpstate)	@
@ 0 "" 2
	.loc 1 77 0
@ 77 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_VFPSTATE #248 offsetof(struct thread_info, vfpstate)	@
@ 0 "" 2
	.loc 1 79 0
@ 79 "arch/arm/kernel/asm-offsets.c" 1
	
->VFP_CPU #272 offsetof(union vfp_state, hard.cpu)	@
@ 0 "" 2
	.loc 1 91 0
@ 91 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 92 0
@ 92 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R0 #0 offsetof(struct pt_regs, ARM_r0)	@
@ 0 "" 2
	.loc 1 93 0
@ 93 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R1 #4 offsetof(struct pt_regs, ARM_r1)	@
@ 0 "" 2
	.loc 1 94 0
@ 94 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R2 #8 offsetof(struct pt_regs, ARM_r2)	@
@ 0 "" 2
	.loc 1 95 0
@ 95 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R3 #12 offsetof(struct pt_regs, ARM_r3)	@
@ 0 "" 2
	.loc 1 96 0
@ 96 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R4 #16 offsetof(struct pt_regs, ARM_r4)	@
@ 0 "" 2
	.loc 1 97 0
@ 97 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R5 #20 offsetof(struct pt_regs, ARM_r5)	@
@ 0 "" 2
	.loc 1 98 0
@ 98 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R6 #24 offsetof(struct pt_regs, ARM_r6)	@
@ 0 "" 2
	.loc 1 99 0
@ 99 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R7 #28 offsetof(struct pt_regs, ARM_r7)	@
@ 0 "" 2
	.loc 1 100 0
@ 100 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R8 #32 offsetof(struct pt_regs, ARM_r8)	@
@ 0 "" 2
	.loc 1 101 0
@ 101 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R9 #36 offsetof(struct pt_regs, ARM_r9)	@
@ 0 "" 2
	.loc 1 102 0
@ 102 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R10 #40 offsetof(struct pt_regs, ARM_r10)	@
@ 0 "" 2
	.loc 1 103 0
@ 103 "arch/arm/kernel/asm-offsets.c" 1
	
->S_FP #44 offsetof(struct pt_regs, ARM_fp)	@
@ 0 "" 2
	.loc 1 104 0
@ 104 "arch/arm/kernel/asm-offsets.c" 1
	
->S_IP #48 offsetof(struct pt_regs, ARM_ip)	@
@ 0 "" 2
	.loc 1 105 0
@ 105 "arch/arm/kernel/asm-offsets.c" 1
	
->S_SP #52 offsetof(struct pt_regs, ARM_sp)	@
@ 0 "" 2
	.loc 1 106 0
@ 106 "arch/arm/kernel/asm-offsets.c" 1
	
->S_LR #56 offsetof(struct pt_regs, ARM_lr)	@
@ 0 "" 2
	.loc 1 107 0
@ 107 "arch/arm/kernel/asm-offsets.c" 1
	
->S_PC #60 offsetof(struct pt_regs, ARM_pc)	@
@ 0 "" 2
	.loc 1 108 0
@ 108 "arch/arm/kernel/asm-offsets.c" 1
	
->S_PSR #64 offsetof(struct pt_regs, ARM_cpsr)	@
@ 0 "" 2
	.loc 1 109 0
@ 109 "arch/arm/kernel/asm-offsets.c" 1
	
->S_OLD_R0 #68 offsetof(struct pt_regs, ARM_ORIG_r0)	@
@ 0 "" 2
	.loc 1 110 0
@ 110 "arch/arm/kernel/asm-offsets.c" 1
	
->S_FRAME_SIZE #72 sizeof(struct pt_regs)	@
@ 0 "" 2
	.loc 1 111 0
@ 111 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 124 0
@ 124 "arch/arm/kernel/asm-offsets.c" 1
	
->MM_CONTEXT_ID #360 offsetof(struct mm_struct, context.id.counter)	@
@ 0 "" 2
	.loc 1 125 0
@ 125 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 127 0
@ 127 "arch/arm/kernel/asm-offsets.c" 1
	
->VMA_VM_MM #32 offsetof(struct vm_area_struct, vm_mm)	@
@ 0 "" 2
	.loc 1 128 0
@ 128 "arch/arm/kernel/asm-offsets.c" 1
	
->VMA_VM_FLAGS #40 offsetof(struct vm_area_struct, vm_flags)	@
@ 0 "" 2
	.loc 1 129 0
@ 129 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 130 0
@ 130 "arch/arm/kernel/asm-offsets.c" 1
	
->VM_EXEC #4 VM_EXEC	@
@ 0 "" 2
	.loc 1 131 0
@ 131 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 132 0
@ 132 "arch/arm/kernel/asm-offsets.c" 1
	
->PAGE_SZ #4096 PAGE_SIZE	@
@ 0 "" 2
	.loc 1 133 0
@ 133 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 134 0
@ 134 "arch/arm/kernel/asm-offsets.c" 1
	
->SYS_ERROR0 #10420224 0x9f0000	@
@ 0 "" 2
	.loc 1 135 0
@ 135 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 136 0
@ 136 "arch/arm/kernel/asm-offsets.c" 1
	
->SIZEOF_MACHINE_DESC #104 sizeof(struct machine_desc)	@
@ 0 "" 2
	.loc 1 137 0
@ 137 "arch/arm/kernel/asm-offsets.c" 1
	
->MACHINFO_TYPE #0 offsetof(struct machine_desc, nr)	@
@ 0 "" 2
	.loc 1 138 0
@ 138 "arch/arm/kernel/asm-offsets.c" 1
	
->MACHINFO_NAME #4 offsetof(struct machine_desc, name)	@
@ 0 "" 2
	.loc 1 139 0
@ 139 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 140 0
@ 140 "arch/arm/kernel/asm-offsets.c" 1
	
->PROC_INFO_SZ #52 sizeof(struct proc_info_list)	@
@ 0 "" 2
	.loc 1 141 0
@ 141 "arch/arm/kernel/asm-offsets.c" 1
	
->PROCINFO_INITFUNC #16 offsetof(struct proc_info_list, __cpu_flush)	@
@ 0 "" 2
	.loc 1 142 0
@ 142 "arch/arm/kernel/asm-offsets.c" 1
	
->PROCINFO_MM_MMUFLAGS #8 offsetof(struct proc_info_list, __cpu_mm_mmu_flags)	@
@ 0 "" 2
	.loc 1 143 0
@ 143 "arch/arm/kernel/asm-offsets.c" 1
	
->PROCINFO_IO_MMUFLAGS #12 offsetof(struct proc_info_list, __cpu_io_mmu_flags)	@
@ 0 "" 2
	.loc 1 144 0
@ 144 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 152 0
@ 152 "arch/arm/kernel/asm-offsets.c" 1
	
->CPU_SLEEP_SIZE #36 offsetof(struct processor, suspend_size)	@
@ 0 "" 2
	.loc 1 153 0
@ 153 "arch/arm/kernel/asm-offsets.c" 1
	
->CPU_DO_SUSPEND #40 offsetof(struct processor, do_suspend)	@
@ 0 "" 2
	.loc 1 154 0
@ 154 "arch/arm/kernel/asm-offsets.c" 1
	
->CPU_DO_RESUME #44 offsetof(struct processor, do_resume)	@
@ 0 "" 2
	.loc 1 160 0
@ 160 "arch/arm/kernel/asm-offsets.c" 1
	
->SLEEP_SAVE_SP_SZ #8 sizeof(struct sleep_save_sp)	@
@ 0 "" 2
	.loc 1 161 0
@ 161 "arch/arm/kernel/asm-offsets.c" 1
	
->SLEEP_SAVE_SP_PHYS #4 offsetof(struct sleep_save_sp, save_ptr_stash_phys)	@
@ 0 "" 2
	.loc 1 162 0
@ 162 "arch/arm/kernel/asm-offsets.c" 1
	
->SLEEP_SAVE_SP_VIRT #0 offsetof(struct sleep_save_sp, save_ptr_stash)	@
@ 0 "" 2
	.loc 1 164 0
@ 164 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 165 0
@ 165 "arch/arm/kernel/asm-offsets.c" 1
	
->DMA_BIDIRECTIONAL #0 DMA_BIDIRECTIONAL	@
@ 0 "" 2
	.loc 1 166 0
@ 166 "arch/arm/kernel/asm-offsets.c" 1
	
->DMA_TO_DEVICE #1 DMA_TO_DEVICE	@
@ 0 "" 2
	.loc 1 167 0
@ 167 "arch/arm/kernel/asm-offsets.c" 1
	
->DMA_FROM_DEVICE #2 DMA_FROM_DEVICE	@
@ 0 "" 2
	.loc 1 168 0
@ 168 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 169 0
@ 169 "arch/arm/kernel/asm-offsets.c" 1
	
->CACHE_WRITEBACK_ORDER #6 __CACHE_WRITEBACK_ORDER	@
@ 0 "" 2
	.loc 1 170 0
@ 170 "arch/arm/kernel/asm-offsets.c" 1
	
->CACHE_WRITEBACK_GRANULE #64 __CACHE_WRITEBACK_GRANULE	@
@ 0 "" 2
	.loc 1 171 0
@ 171 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 209 0
@ 209 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
	.loc 1 211 0
@ 211 "arch/arm/kernel/asm-offsets.c" 1
	
->VDSO_DATA_SIZE #4096 sizeof(union vdso_data_store)	@
@ 0 "" 2
	.loc 1 214 0
	mov	r0, #0	@,
	bx	lr	@
.LFE1870:
	.fnend
	.size	main, .-main
	.section	.debug_frame,"",%progbits
.Lframe0:
	.4byte	.LECIE0-.LSCIE0
.LSCIE0:
	.4byte	0xffffffff
	.byte	0x3
	.ascii	"\000"
	.uleb128 0x1
	.sleb128 -4
	.uleb128 0xe
	.byte	0xc
	.uleb128 0xd
	.uleb128 0
	.align	2
.LECIE0:
.LSFDE0:
	.4byte	.LEFDE0-.LASFDE0
.LASFDE0:
	.4byte	.Lframe0
	.4byte	.LFB1870
	.4byte	.LFE1870-.LFB1870
	.align	2
.LEFDE0:
	.text
.Letext0:
	.file 2 "include/linux/types.h"
	.file 3 "include/asm-generic/atomic-long.h"
	.file 4 "include/linux/mm.h"
	.file 5 "./arch/arm/include/asm/hwcap.h"
	.file 6 "include/linux/printk.h"
	.file 7 "include/linux/kernel.h"
	.file 8 "./arch/arm/include/asm/memory.h"
	.file 9 "./arch/arm/include/asm/thread_info.h"
	.file 10 "include/linux/cpumask.h"
	.file 11 "include/linux/highuid.h"
	.file 12 "include/linux/seq_file.h"
	.file 13 "include/linux/sched.h"
	.file 14 "include/asm-generic/percpu.h"
	.file 15 "include/linux/mmzone.h"
	.file 16 "include/linux/workqueue.h"
	.file 17 "include/linux/percpu_counter.h"
	.file 18 "include/linux/hrtimer.h"
	.file 19 "include/linux/debug_locks.h"
	.file 20 "./arch/arm/include/asm/proc-fns.h"
	.file 21 "./arch/arm/include/asm/tlbflush.h"
	.file 22 "include/asm-generic/pgtable.h"
	.file 23 "include/linux/vmstat.h"
	.file 24 "include/linux/dcache.h"
	.file 25 "include/linux/quota.h"
	.file 26 "include/linux/fs.h"
	.file 27 "./arch/arm/include/asm/xen/hypervisor.h"
	.file 28 "./arch/arm/include/asm/dma-mapping.h"
	.file 29 "./arch/arm/include/asm/cachetype.h"
	.file 30 "include/asm-generic/int-ll64.h"
	.file 31 "include/linux/dma-direction.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x3e0
	.2byte	0x4
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF72
	.byte	0x1
	.4byte	.LASF73
	.4byte	.LASF74
	.4byte	.Ldebug_ranges0+0
	.4byte	0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF1
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF2
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF4
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF5
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF6
	.uleb128 0x4
	.ascii	"u32\000"
	.byte	0x1e
	.byte	0x16
	.4byte	0x48
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF7
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF8
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF9
	.uleb128 0x5
	.4byte	.LASF10
	.byte	0x2
	.byte	0x1d
	.4byte	0x88
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.4byte	.LASF11
	.uleb128 0x5
	.4byte	.LASF12
	.byte	0x2
	.byte	0xa4
	.4byte	0x5d
	.uleb128 0x6
	.uleb128 0x5
	.4byte	.LASF13
	.byte	0x2
	.byte	0xb1
	.4byte	0x9a
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF14
	.uleb128 0x7
	.byte	0x4
	.4byte	0xb3
	.uleb128 0x8
	.4byte	.LASF16
	.uleb128 0x5
	.4byte	.LASF15
	.byte	0x3
	.byte	0x1e
	.4byte	0x9b
	.uleb128 0x8
	.4byte	.LASF17
	.uleb128 0x9
	.byte	0x4
	.uleb128 0xa
	.ascii	"pid\000"
	.uleb128 0x8
	.4byte	.LASF18
	.uleb128 0x7
	.byte	0x4
	.4byte	0xda
	.uleb128 0xb
	.4byte	0xc3
	.uleb128 0x8
	.4byte	.LASF19
	.uleb128 0x8
	.4byte	.LASF20
	.uleb128 0xc
	.4byte	.LASF21
	.byte	0x4
	.2byte	0x20f
	.4byte	0xf5
	.uleb128 0xd
	.4byte	0x100
	.uleb128 0xe
	.4byte	0xad
	.byte	0
	.uleb128 0x8
	.4byte	.LASF22
	.uleb128 0x8
	.4byte	.LASF23
	.uleb128 0x8
	.4byte	.LASF24
	.uleb128 0xf
	.4byte	.LASF75
	.byte	0x4
	.byte	0x1f
	.byte	0x7
	.4byte	0x134
	.uleb128 0x10
	.4byte	.LASF25
	.sleb128 0
	.uleb128 0x10
	.4byte	.LASF26
	.sleb128 1
	.uleb128 0x10
	.4byte	.LASF27
	.sleb128 2
	.uleb128 0x10
	.4byte	.LASF28
	.sleb128 3
	.byte	0
	.uleb128 0x8
	.4byte	.LASF29
	.uleb128 0x11
	.4byte	.LASF76
	.byte	0x1
	.byte	0x3b
	.4byte	0x41
	.4byte	.LFB1870
	.4byte	.LFE1870-.LFB1870
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x8
	.4byte	.LASF30
	.uleb128 0x8
	.4byte	.LASF31
	.uleb128 0x8
	.4byte	.LASF32
	.uleb128 0x12
	.4byte	.LASF33
	.byte	0x5
	.byte	0xd
	.4byte	0x48
	.uleb128 0x13
	.4byte	0x41
	.4byte	0x173
	.uleb128 0x14
	.byte	0
	.uleb128 0x12
	.4byte	.LASF34
	.byte	0x6
	.byte	0x2e
	.4byte	0x168
	.uleb128 0x15
	.4byte	.LASF35
	.byte	0x7
	.2byte	0x1ba
	.4byte	0x41
	.uleb128 0x13
	.4byte	0x76
	.4byte	0x195
	.uleb128 0x14
	.byte	0
	.uleb128 0x15
	.4byte	.LASF36
	.byte	0x7
	.2byte	0x1f2
	.4byte	0x1a1
	.uleb128 0xb
	.4byte	0x18a
	.uleb128 0x15
	.4byte	.LASF37
	.byte	0x7
	.2byte	0x1fd
	.4byte	0x1b2
	.uleb128 0xb
	.4byte	0x18a
	.uleb128 0x12
	.4byte	.LASF38
	.byte	0x8
	.byte	0xa6
	.4byte	0x6f
	.uleb128 0x16
	.4byte	0x8f
	.4byte	0x1d1
	.uleb128 0xe
	.4byte	0x6f
	.byte	0
	.uleb128 0x15
	.4byte	.LASF39
	.byte	0x8
	.2byte	0x118
	.4byte	0x1dd
	.uleb128 0x7
	.byte	0x4
	.4byte	0x1c2
	.uleb128 0x17
	.4byte	.LASF40
	.byte	0x9
	.byte	0x58
	.4byte	0x6f
	.uleb128 0x1
	.byte	0x5d
	.uleb128 0x12
	.4byte	.LASF41
	.byte	0xa
	.byte	0x25
	.4byte	0x41
	.uleb128 0x12
	.4byte	.LASF42
	.byte	0xa
	.byte	0x59
	.4byte	0x206
	.uleb128 0xb
	.4byte	0xd4
	.uleb128 0x13
	.4byte	0x6f
	.4byte	0x221
	.uleb128 0x18
	.4byte	0xa6
	.byte	0x20
	.uleb128 0x18
	.4byte	0xa6
	.byte	0
	.byte	0
	.uleb128 0x15
	.4byte	.LASF43
	.byte	0xa
	.2byte	0x2f2
	.4byte	0x22d
	.uleb128 0xb
	.4byte	0x20b
	.uleb128 0x12
	.4byte	.LASF44
	.byte	0xb
	.byte	0x22
	.4byte	0x41
	.uleb128 0x12
	.4byte	.LASF45
	.byte	0xb
	.byte	0x23
	.4byte	0x41
	.uleb128 0x12
	.4byte	.LASF46
	.byte	0xc
	.byte	0x93
	.4byte	0x14e
	.uleb128 0x15
	.4byte	.LASF47
	.byte	0xd
	.2byte	0x985
	.4byte	0x153
	.uleb128 0x13
	.4byte	0x6f
	.4byte	0x26f
	.uleb128 0x18
	.4byte	0xa6
	.byte	0x3
	.byte	0
	.uleb128 0x12
	.4byte	.LASF48
	.byte	0xe
	.byte	0x12
	.4byte	0x25f
	.uleb128 0x12
	.4byte	.LASF49
	.byte	0xf
	.byte	0x4c
	.4byte	0x41
	.uleb128 0x15
	.4byte	.LASF50
	.byte	0xf
	.2byte	0x26f
	.4byte	0xad
	.uleb128 0x15
	.4byte	.LASF51
	.byte	0x10
	.2byte	0x160
	.4byte	0x29d
	.uleb128 0x7
	.byte	0x4
	.4byte	0x158
	.uleb128 0x15
	.4byte	.LASF52
	.byte	0xf
	.2byte	0x357
	.4byte	0xcf
	.uleb128 0x12
	.4byte	.LASF53
	.byte	0x11
	.byte	0x1c
	.4byte	0x41
	.uleb128 0x15
	.4byte	.LASF54
	.byte	0x12
	.2byte	0x132
	.4byte	0x48
	.uleb128 0x15
	.4byte	.LASF55
	.byte	0xd
	.2byte	0x7f0
	.4byte	0x2d2
	.uleb128 0x7
	.byte	0x4
	.4byte	0xca
	.uleb128 0x12
	.4byte	.LASF56
	.byte	0x13
	.byte	0xa
	.4byte	0x41
	.uleb128 0x12
	.4byte	.LASF57
	.byte	0x4
	.byte	0x22
	.4byte	0x6f
	.uleb128 0x12
	.4byte	.LASF58
	.byte	0x4
	.byte	0x2d
	.4byte	0xc8
	.uleb128 0x12
	.4byte	.LASF19
	.byte	0x14
	.byte	0x4e
	.4byte	0xdf
	.uleb128 0x12
	.4byte	.LASF59
	.byte	0x15
	.byte	0xe6
	.4byte	0xe4
	.uleb128 0x19
	.4byte	0x7d
	.uleb128 0x15
	.4byte	.LASF60
	.byte	0x15
	.2byte	0x2a3
	.4byte	0x320
	.uleb128 0x7
	.byte	0x4
	.4byte	0x30f
	.uleb128 0x15
	.4byte	.LASF61
	.byte	0x16
	.2byte	0x262
	.4byte	0x6f
	.uleb128 0x13
	.4byte	0x33d
	.4byte	0x33d
	.uleb128 0x14
	.byte	0
	.uleb128 0x7
	.byte	0x4
	.4byte	0xe9
	.uleb128 0x15
	.4byte	.LASF62
	.byte	0x4
	.2byte	0x21a
	.4byte	0x34f
	.uleb128 0xb
	.4byte	0x332
	.uleb128 0x12
	.4byte	.LASF63
	.byte	0x17
	.byte	0x1c
	.4byte	0x100
	.uleb128 0x13
	.4byte	0xb8
	.4byte	0x36f
	.uleb128 0x18
	.4byte	0xa6
	.byte	0x21
	.byte	0
	.uleb128 0x12
	.4byte	.LASF64
	.byte	0x17
	.byte	0x6f
	.4byte	0x35f
	.uleb128 0x15
	.4byte	.LASF65
	.byte	0x4
	.2byte	0x6c0
	.4byte	0x18a
	.uleb128 0x15
	.4byte	.LASF66
	.byte	0x4
	.2byte	0x6c0
	.4byte	0x18a
	.uleb128 0x15
	.4byte	.LASF67
	.byte	0x18
	.2byte	0x20c
	.4byte	0x41
	.uleb128 0x15
	.4byte	.LASF23
	.byte	0x19
	.2byte	0x105
	.4byte	0x105
	.uleb128 0x15
	.4byte	.LASF68
	.byte	0x1a
	.2byte	0x900
	.4byte	0x3b6
	.uleb128 0x7
	.byte	0x4
	.4byte	0x10a
	.uleb128 0x12
	.4byte	.LASF69
	.byte	0x1b
	.byte	0x15
	.4byte	0x3c7
	.uleb128 0x7
	.byte	0x4
	.4byte	0x134
	.uleb128 0x12
	.4byte	.LASF70
	.byte	0x1c
	.byte	0x11
	.4byte	0x134
	.uleb128 0x12
	.4byte	.LASF71
	.byte	0x1d
	.byte	0xc
	.4byte	0x48
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x13
	.byte	0
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x13
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x13
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x15
	.byte	0
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.LFB1870
	.4byte	.LFE1870-.LFB1870
	.4byte	0
	.4byte	0
	.section	.debug_ranges,"",%progbits
.Ldebug_ranges0:
	.4byte	.LFB1870
	.4byte	.LFE1870
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF68:
	.ascii	"blockdev_superblock\000"
.LASF30:
	.ascii	"user_namespace\000"
.LASF66:
	.ascii	"__init_end\000"
.LASF57:
	.ascii	"max_mapnr\000"
.LASF54:
	.ascii	"hrtimer_resolution\000"
.LASF55:
	.ascii	"cad_pid\000"
.LASF35:
	.ascii	"panic_timeout\000"
.LASF67:
	.ascii	"sysctl_vfs_cache_pressure\000"
.LASF2:
	.ascii	"short int\000"
.LASF14:
	.ascii	"sizetype\000"
.LASF75:
	.ascii	"dma_data_direction\000"
.LASF60:
	.ascii	"erratum_a15_798181_handler\000"
.LASF5:
	.ascii	"long long int\000"
.LASF47:
	.ascii	"init_pid_ns\000"
.LASF72:
	.ascii	"GNU C 4.8.3 -mlittle-endian -mabi=aapcs-linux -mno-"
	.ascii	"thumb-interwork -mfpu=vfp -marm -march=armv7-a -mfl"
	.ascii	"oat-abi=soft -mtls-dialect=gnu -g -Os -std=gnu90 -f"
	.ascii	"no-strict-aliasing -fno-common -fno-PIE -fno-dwarf2"
	.ascii	"-cfi-asm -fno-ipa-sra -funwind-tables -fno-delete-n"
	.ascii	"ull-pointer-checks -fno-caller-saves -fno-stack-pro"
	.ascii	"tector -fomit-frame-pointer -fno-var-tracking-assig"
	.ascii	"nments -femit-struct-debug-baseonly -fno-var-tracki"
	.ascii	"ng -fno-strict-overflow -fconserve-stack -ffunction"
	.ascii	"-sections -fdata-sections --param allow-store-data-"
	.ascii	"races=0\000"
.LASF70:
	.ascii	"arm_dma_ops\000"
.LASF46:
	.ascii	"init_user_ns\000"
.LASF56:
	.ascii	"debug_locks\000"
.LASF31:
	.ascii	"pid_namespace\000"
.LASF34:
	.ascii	"console_printk\000"
.LASF33:
	.ascii	"elf_hwcap\000"
.LASF26:
	.ascii	"DMA_TO_DEVICE\000"
.LASF64:
	.ascii	"vm_stat\000"
.LASF10:
	.ascii	"bool\000"
.LASF38:
	.ascii	"__pv_phys_pfn_offset\000"
.LASF25:
	.ascii	"DMA_BIDIRECTIONAL\000"
.LASF51:
	.ascii	"system_wq\000"
.LASF24:
	.ascii	"super_block\000"
.LASF48:
	.ascii	"__per_cpu_offset\000"
.LASF53:
	.ascii	"percpu_counter_batch\000"
.LASF16:
	.ascii	"page\000"
.LASF7:
	.ascii	"long int\000"
.LASF63:
	.ascii	"vm_event_states\000"
.LASF65:
	.ascii	"__init_begin\000"
.LASF71:
	.ascii	"cacheid\000"
.LASF15:
	.ascii	"atomic_long_t\000"
.LASF1:
	.ascii	"unsigned char\000"
.LASF45:
	.ascii	"overflowgid\000"
.LASF39:
	.ascii	"arch_virt_to_idmap\000"
.LASF0:
	.ascii	"signed char\000"
.LASF50:
	.ascii	"mem_map\000"
.LASF6:
	.ascii	"long long unsigned int\000"
.LASF4:
	.ascii	"unsigned int\000"
.LASF42:
	.ascii	"cpu_online_mask\000"
.LASF49:
	.ascii	"page_group_by_mobility_disabled\000"
.LASF23:
	.ascii	"dqstats\000"
.LASF41:
	.ascii	"nr_cpu_ids\000"
.LASF62:
	.ascii	"compound_page_dtors\000"
.LASF3:
	.ascii	"short unsigned int\000"
.LASF18:
	.ascii	"pglist_data\000"
.LASF20:
	.ascii	"cpu_tlb_fns\000"
.LASF22:
	.ascii	"vm_event_state\000"
.LASF9:
	.ascii	"char\000"
.LASF76:
	.ascii	"main\000"
.LASF44:
	.ascii	"overflowuid\000"
.LASF74:
	.ascii	"/home/ubuntu/qsdk/qca/src/linux-4.4\000"
.LASF27:
	.ascii	"DMA_FROM_DEVICE\000"
.LASF17:
	.ascii	"cpumask\000"
.LASF11:
	.ascii	"_Bool\000"
.LASF73:
	.ascii	"arch/arm/kernel/asm-offsets.c\000"
.LASF58:
	.ascii	"high_memory\000"
.LASF8:
	.ascii	"long unsigned int\000"
.LASF29:
	.ascii	"dma_map_ops\000"
.LASF61:
	.ascii	"zero_pfn\000"
.LASF36:
	.ascii	"hex_asc\000"
.LASF21:
	.ascii	"compound_page_dtor\000"
.LASF43:
	.ascii	"cpu_bit_bitmap\000"
.LASF28:
	.ascii	"DMA_NONE\000"
.LASF69:
	.ascii	"xen_dma_ops\000"
.LASF59:
	.ascii	"cpu_tlb\000"
.LASF32:
	.ascii	"workqueue_struct\000"
.LASF19:
	.ascii	"processor\000"
.LASF40:
	.ascii	"current_stack_pointer\000"
.LASF52:
	.ascii	"contig_page_data\000"
.LASF12:
	.ascii	"phys_addr_t\000"
.LASF13:
	.ascii	"atomic_t\000"
.LASF37:
	.ascii	"hex_asc_upper\000"
	.ident	"GCC: (OpenWrt/Linaro GCC 4.8-2014.04 35ebd114e275+r49254) 4.8.3"
	.section	.note.GNU-stack,"",%progbits
