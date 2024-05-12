#!/bin/bash

set -e

install_dependencies() {
    echo "Installing dependencies for cjson..."
    apt install -y cmake || { echo "Failed to install dependencies."; exit 1; }
}

build_and_install() {
    echo "Building cjson..."
    cd ../../deps/cjson && mkdir -p build && cd build && cmake ../cJSON/ && make && make install && cd - || { echo "Failed to build and install cjson."; exit 1; }
}

check_installation() {
	echo "Checking cjson installation..."

    if ! find /usr/local/lib -name "libcjson.so"; then
		echo "Error: libcjson not found in /usr/local/lib."
		exit 1
	fi
}

install_cjson() {
    echo "Trying to install cjson..."

    install_dependencies
    build_and_install
    check_installation
}

install_cjson
echo "cjson installed successfully."
exit 0