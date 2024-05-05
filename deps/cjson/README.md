# Installation for cJSON

This guide provides detailed steps on how to compile and install cJSON from [DaveGamble/cJSON](https://github.com/DaveGamble/cJSON). It is a lightweight JSON library written in C.

## Dependency Requirements

Installation of dependencies.

```bash
sudo apt install cmake
```

## Installation

Run the following command

```bash
git submodule update --init

mkdir build

cd build && cmake ../cJSON/
make && make install
```