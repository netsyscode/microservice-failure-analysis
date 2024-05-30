#!/bin/bash

set -e

install_dependencies() {
    echo "Installing dependencies for bpftools..."
    apt install -y binutils-dev libcap-dev || { echo "Failed to install dependencies."; exit 1; }
}

create_symbolic_link() {
    echo "Creating symbolic link for libbpf..."
    if [ -d "../../deps/bpftools/bpftools/libbpf/" ]; then
        rm -r ../../deps/bpftools/bpftools/libbpf/ || { echo "Failed to remove old libbpf directory."; exit 1; }
    fi
    ln -s $(realpath ../../deps/libbpf/libbpf/) $(realpath ../../deps/bpftools/bpftools) || { echo "Failed to create symbolic link for libbpf."; exit 1; }
    git config submodule.deps/bpftools/bpftools.ignore all
}

build_and_install() {
    echo "Building bpftools..."
    cd ../../deps/bpftools/bpftools/src && make && make install && cd - || { echo "Failed to build and install bpftools."; exit 1; }
}

check_installation() {
    echo "Checking bpftool installation..."
    if ! bpftool --version; then
        echo "Error: bpftools not correctly installed."
        exit 1
    fi
}

install_bpftools() {
    echo "Trying to install bpftool..."

    install_dependencies
    create_symbolic_link
    build_and_install
    check_installation
}

install_bpftools
echo "bpftools installed successfully."
exit 0