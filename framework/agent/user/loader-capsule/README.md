# Loader Capsule and Debug Utility

## Overview

This directory contains the source code for a loader utility designed to facilitate loading and attaching eBPF (Extended Berkeley Packet Filters) programs, with optional debug capabilities. 
It leverages `bpftool` and BPF skeletons to manage lifecycle events of eBPF programs. 
Additionally, a debug utility is included to print `trace_pipe` outputs via the Linux tracefs.

## Components

- **loader.c**: The main loader utility that handles the initialization, loading, attaching, and optionally detaching of eBPF programs defined by a BPF skeleton. It utilizes C macro to abstract the specifics of the eBPF program it operates on.
- **debug.c**: Offers debugging support by intercepting SIGINT signals and reading `trace_pipe` outputs from the Linux trace filesystem (tracefs). It is conditionally compiled into the loader utility based on the `DEBUG` flag.
- **Makefile**: Provides build instructions for compiling the loader and debug utility, with conditional debug support. It also outlines the dependencies on external libraries such as libbpf and libelf.

## Building

To build the loader capsule:

1. Ensure the `SKEL` and `SKEL_DIR` variables are set, pointing to the eBPF program skeleton and its directory, respectively.
2. `BUILD_DIR_NAME`: Specifies the directory where the compiled binaries will be placed. This allows for organized storage of build artifacts, separate from the source code. If not set, a default path within a build subdirectory under `./user` is used, based on the SKEL name. This variable can be customized to change the output directory of the build process.
3. Run `make` to compile the loader and debug utility. The `DEBUG` flag can be set to 1 to enable debug support.

Example:

```bash
make SKEL=my_ebpf_program SKEL_DIR=/path/to/skel DEBUG=1

# Or you can specify BUILD_DIR_NAME
make SKEL=my_ebpf_program SKEL_DIR=/path/to/skel BUILD_DIR_NAME=/custom/build/directory DEBUG=1
```

## Usage

Run the loader capsule. 
If compiled with DEBUG=1, the program can be interrupted with SIGINT to trigger debug output, with skel struct detached and destroyed gracefully.

```bash
./path/to/build/loader-capsule
```

Ensure that you have the necessary permissions (i.e., sudo) to load eBPF programs and access tracefs.

## Cleaning up

To remove the built binaries and clean up the build directory, run:

```bash
make SKEL=my_ebpf_program SKEL_DIR=/path/to/skel clean

# Or
make SKEL=my_ebpf_program SKEL_DIR=/path/to/skel BUILD_DIR_NAME=/custom/build/directory clean
```

## Note

At the current stage, this loader capsule requires several parameters to be filled in.
This will be upgraded for better functionality after the basic features stabilize.