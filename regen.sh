 make ARCH=arm64 vayu_defconfig savedefconfig && mv .config arch/arm64/configs/vayu_defconfig
 make ARCH=arm64 vayu-bpf_defconfig savedefconfig && mv .config arch/arm64/configs/vayu-bpf_defconfig
 make ARCH=arm64 vayu-debug_defconfig savedefconfig && mv .config arch/arm64/configs/vayu-debug_defconfig
rm defconfig
