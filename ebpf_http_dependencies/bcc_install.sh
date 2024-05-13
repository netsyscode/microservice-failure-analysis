sudo apt install -y zip bison build-essential cmake flex git libedit-dev libllvm14 llvm-14-dev libclang-14-dev python3 zlib1g-dev libelf-dev libfl-dev python3-setuptools liblzma-dev libdebuginfod-dev arping netperf iperf

git clone --branch v0.24.0 https://github.com/iovisor/bcc.git
mkdir bcc/build; cd bcc/build
cmake ..
make
sudo make install
# cmake -DPYTHON_CMD=python3 .. # build python3 binding
cmake -DPYTHON_CMD=python3 .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DENABLE_EXAMPLES=0 -DENABLE_TESTS=0 -DENABLE_MAN=0 -DENABLE_LLVM_SHARED=1
pushd src/python/
make
sudo make install
popd