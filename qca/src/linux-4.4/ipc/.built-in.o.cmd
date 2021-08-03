cmd_ipc/built-in.o :=  arm-openwrt-linux-uclibcgnueabi-ld -EL    -r -o ipc/built-in.o ipc/util.o ipc/msgutil.o ipc/msg.o ipc/sem.o ipc/shm.o ipc/syscall.o ipc/ipc_sysctl.o 
