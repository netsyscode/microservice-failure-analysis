# Bash Installation Scripts for microservice-failure-analysis

This repository contains scripts to automate the installation of `libbpf` and `bpftools` on Ubuntu systems. 
These tools are essential for this repo and for working with BPF (Berkeley Packet Filter) programs in Linux.

## Scripts Description

- `install.sh`: Entrance of the installation scripts. It calls `install_libbpf.sh` and `install_bpftool.sh`.
- `install_libbpf.sh`: This script installs `libbpf` from source. It checks for necessary kernel configurations, installs dependencies, creates symbolic links for LLVM tools, and builds and installs `libbpf`.
- `install_bpftool.sh`: This script installs `bpftools`, which depends on `libbpf`. It handles dependency installation, symbolic linking for `libbpf`, and compiles and installs `bpftools`.

## Usage

```bash
chmod +x ./install.sh && ./install.sh
```