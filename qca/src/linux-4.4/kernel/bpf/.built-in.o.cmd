cmd_kernel/bpf/built-in.o :=  arm-openwrt-linux-uclibcgnueabi-ld -EL    -r -o kernel/bpf/built-in.o kernel/bpf/core.o kernel/bpf/syscall.o kernel/bpf/verifier.o kernel/bpf/inode.o kernel/bpf/helpers.o kernel/bpf/hashtab.o kernel/bpf/arraymap.o 
