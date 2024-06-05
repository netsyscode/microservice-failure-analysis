#!/bin/bash

set -e

check_kernel_configs() {
	echo "Checking required kernel configurations..."

	if ! cat /boot/config-$(uname -r) | grep -q CONFIG_BPF=y; then
		echo "Error: CONFIG_BPF=y not found in kernel config."
		exit 1
	fi    

	if ! cat /boot/config-$(uname -r) | grep -q CONFIG_BPF_SYSCALL=y; then
		echo "Error: CONFIG_BPF=y not found in kernel config."
		exit 1
	fi    

	cat /boot/config-$(uname -r) | grep CONFIG_NET_CLS_BPF=m || echo "Optional config CONFIG_NET_CLS_BPF=m not found"
	cat /boot/config-$(uname -r) | grep CONFIG_NET_ACT_BPF=m || echo "Optional config CONFIG_NET_ACT_BPF=m not found"
	cat /boot/config-$(uname -r) | grep CONFIG_BPF_EVENTS=y || echo "Optional config CONFIG_BPF_EVENTS=y not found"
	cat /boot/config-$(uname -r) | grep CONFIG_BPF_JIT=y || echo "Optional config CONFIG_BPF_JIT=y not found"
	cat /boot/config-$(uname -r) | grep CONFIG_HAVE_BPF_JIT=y || echo "CONFIG_HAVE_BPF_JIT=y not found for kernel versions 4.1 through 4.6"
	cat /boot/config-$(uname -r) | grep CONFIG_HAVE_EBPF_JIT=y || echo "CONFIG_HAVE_EBPF_JIT=y not found for kernel versions 4.7 and later"
	cat /boot/config-$(uname -r) | grep CONFIG_IKHEADERS=y || echo "CONFIG_IKHEADERS=y not found, required for /sys/kernel/kheaders.tar.xz"
}

install_dependencies() {
	echo "Installing common libraries..."
	apt install -y libelf1 libelf-dev zlib1g-dev gcc-multilib wget build-essential lsb-release software-properties-common gnupg || exit 1

	echo "Installing LLVM and related tools..."
	bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)" -- 14 || exit 1
}

create_symbolic_link() {
	echo "Creating symbolic links for LLVM tools..."

	ln -sf $(which clang-14) /usr/bin/clang || { echo "Failed to create link for clang"; exit 1; }
	ln -sf $(which clang++-14) /usr/bin/clang++ || { echo "Failed to create link for clang++"; exit 1; }
	ln -sf $(which llc-14) /usr/bin/llc || { echo "Failed to create link for llc"; exit 1; }
	ln -sf $(which opt-14) /usr/bin/opt || { echo "Failed to create link for opt"; exit 1; }
	ln -sf $(which llvm-dis-14) /usr/bin/llvm-dis || { echo "Failed to create link for llvm-dis"; exit 1; }
	ln -sf $(which llvm-objdump-14) /usr/bin/llvm-objdump || { echo "Failed to create link for llvm-objdump"; exit 1; }
	ln -sf $(which llvm-config-14) /usr/bin/llvm-config || { echo "Failed to create link for llvm-config"; exit 1; }
	ln -sf $(which llvm-strip-14) /usr/bin/llvm-strip || { echo "Failed to create link for llvm-strip"; exit 1; }

	ln -sf /usr/include/llvm-c-14/llvm-c /usr/include/llvm-c || { echo "Failed to create link for llvm-c headers"; exit 1; }
	ln -sf /usr/include/llvm-14/llvm /usr/include/llvm || { echo "Failed to create link for llvm headers"; exit 1; }
}

build_and_install() {
	echo "Installing libbpf..."
	cd ../../deps/libbpf/libbpf/src && make && make install && cd - || { echo "Failed to build and install libbpf"; exit 1; }
}

check_installation() {
	echo "Checking libbpf installation..."

    if ! find /usr/lib64 -name "libbpf.so"; then
		echo "Error: libbpf not found in /usr/lib64."
		exit 1
	fi
}

install_libbpf() {
	echo "Trying to install libbpf..."

	check_kernel_configs
	install_dependencies
	create_symbolic_link
	build_and_install
	check_installation
}

install_libbpf
echo "Libbpf installed successfully."
exit 0
