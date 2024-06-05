#!/bin/bash

# Install git incase it is not installed
apt-get install -y git

# Update the git submodules
git submodule update --init

# Install the libbpf library
chmod +x ./install_libbpf.sh && ./install_libbpf.sh

if [ $? -ne 0 ]; then
  echo "An error occurred during the installation of libbpf."
  exit 1
fi

# Install the bpftool
echo "Try to install bpftool..."
chmod +x ./install_bpftool.sh && ./install_bpftool.sh

if [ $? -ne 0 ]; then
  echo "An error occurred during the installation of bpftool."
  exit 1
fi

# Install the cjson library
echo "Try to install cjson..."
chmod +x ./install_cjson.sh && ./install_cjson.sh

if [ $? -ne 0 ]; then
  echo "An error occurred during the installation of cjson."
  exit 1
fi

echo "Installation completed successfully."