# Manual Installation for libbpf

Installation for `libbpf` (v1.2.0).

## Environment Requirements

Check kernel option

```bash
cat /boot/config-$(uname -r) | grep CONFIG_BPF=y
cat /boot/config-$(uname -r) | grep CONFIG_BPF_SYSCALL=y
# [optional, for tc filters]
cat /boot/config-$(uname -r) | grep CONFIG_NET_CLS_BPF=m
# [optional, for tc actions]
cat /boot/config-$(uname -r) | grep CONFIG_NET_ACT_BPF=m
# [optional, for kprobes]
cat /boot/config-$(uname -r) | grep CONFIG_BPF_EVENTS=y
cat /boot/config-$(uname -r) | grep CONFIG_BPF_JIT=y
# [for Linux kernel versions 4.1 through 4.6]
cat /boot/config-$(uname -r) | grep CONFIG_HAVE_BPF_JIT=y
# [for Linux kernel versions 4.7 and later]
cat /boot/config-$(uname -r) | grep CONFIG_HAVE_EBPF_JIT=y
# Need kernel headers through /sys/kernel/kheaders.tar.xz
cat /boot/config-$(uname -r) | grep CONFIG_IKHEADERS=y
```

## Dependency Requirements

Installation of dependencies.

For common libraries: `libelf1`, `libelf-dev`, `zlib1g-dev`, `gcc-multilib`

```bash
sudo apt install libelf1 libelf-dev zlib1g-dev

# For error: /usr/include/linux/types.h:5:10: fatal error: 'asm/types.h' file not found
sudo apt install gcc-multilib 
```

For compiler, linker, etc.: `clang-14`, `llc-14`, `opt-14`, `llvm-dis-14`, `llvm-objdump-14`

```bash
sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)" -- 14

sudo ln -s $(which clang-14) /usr/bin/clang
sudo ln -s $(which clang++-14) /usr/bin/clang++
sudo ln -s $(which llc-14) /usr/bin/llc
sudo ln -s $(which opt-14) /usr/bin/opt
sudo ln -s $(which llvm-dis-14) /usr/bin/llvm-dis
sudo ln -s $(which llvm-objdump-14) /usr/bin/llvm-objdump
sudo ln -s $(which llvm-config-14) /usr/bin/llvm-config
sudo ln -s $(which llvm-strip-14) /usr/bin/llvm-strip

sudo ln -s /usr/include/llvm-c-14/llvm-c /usr/include/llvm-c
sudo ln -s /usr/include/llvm-14/llvm /usr/include/llvm
```

## Installation

Run the following command

```bash
# Note: Update w/o recursive option
git submodule update --init

# Install libbpf
cd ./libbpf/src && make && sudo make install && cd -
```

## Check Installation

```bash
./bpftool --version
```