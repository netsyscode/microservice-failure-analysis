# Kernel BPF Program Infrastructure

This folder is used for the compilation support of eBPF kernel programs.
Common users don't have to modify this folder.

## Folder structure

```bash
./
├── include
│   ├── all.h
│   ├── asm_goto_workaround.h
│   ├── bpf_core_read.h
│   ├── bpf_endian.h
│   ├── bpf_helper_defs.h
│   ├── bpf_helpers.h
│   ├── bpf_tracing.h
│   ├── kernel.h
│   ├── LICENSE.BSD-2-Clause
│   └── vmlinux.h
├── Makefile
├── Makefile.arch
├── Makefile.ebpf
├── README.md
└── update.sh
```

## Helper Commands

```bash
# Check disassembling results
llvm-objdump -s -d ./yourBuildDir/main.o
```

## Update Header Files (Optional)

`update.sh` is for updating the header files in `./include`.
These header files should be kept up with the version of libbpf and the kernel.

***If nothing is wrong, do not execute this.***