# eBPF Program Templates

To add a new kernel eBPF program, create a folder with eBPF programs in `*.c` and a `Makefile` in it.
The `Makefile` should include the relative path of `Makefile.arch` and `Makefile.ebpf`, which looks like below:

```Makefile
# Optional: The directory to which the program will be installed
# If BUILD_DIR_NAME is not specified, it will create a `build` subdir under the agent directory
# BUILD_DIR_NAME=xxx
include ../../../bpf-kernel-infrastructure/Makefile.arch
include ../../../bpf-kernel-infrastructure/Makefile.ebpf
```

Either run `make` under the newly created folder (e.g., `./helloworld`) to just build the new program,
or run `make` under this folder (`./`) to build all kernel programs that have been updated.

Note that the directory of the eBPF programs does not have to be a subdirectory of `./`,
nor do it any position requirements. 
All you need to do is to relatively include `Makefile.arch` and `Makefile.ebpf` in the Makefile.

`make clean` works in the same way.

## Current eBPF Program Templates

- `./src/hello-world`: An example program, attached to `tracepoint/syscalls/sys_enter_execve`
