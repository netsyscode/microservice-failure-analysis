# Manual Installation for bpftools

Installation for `bpftools` (v7.2.0).

## Environment Requirements

Same as `libbpf`.

## Dependency Requirements

Same as `libbpf`.
Also, `libbpf` needs to be installed first.

```bash
# For Feature libbfd
sudo apt install binutils-dev

# For Feature libcap
sudo apt install libcap-dev
```

## Installation

Run the following command

```bash
# Note: Update w/o recursive option
git submodule update --init

# Create symbolic link instead of download submodule again
rm -r ./bpftools/libbpf/ && ln -s $(realpath ../libbpf/libbpf/) $(realpath ./bpftools)

cd ./bpftools/src && make && sudo make install && cd -
```

## Check Installation

```bash
dpkg -l | grep libbpf
ldconfig -p | grep libbpf
```

## Generate vmlinux.h (Optional)

This requires the installation of `bpftools` (v7.2.0 recommended).

```bash
bpftool btf dump file /sys/kernel/btf/vmlinux format c > ./kernel/include/vmlinux.h
```