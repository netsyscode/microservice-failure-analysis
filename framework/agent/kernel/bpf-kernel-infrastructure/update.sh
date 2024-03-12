#!/usr/bin/env bash

# Version of libbpf to fetch headers from
LIBBPF_VERSION=1.2.0

# The headers we want
prefix=libbpf-"$LIBBPF_VERSION"
headers=(
    "$prefix"/LICENSE.BSD-2-Clause
    "$prefix"/src/bpf_endian.h
    "$prefix"/src/bpf_helper_defs.h
    "$prefix"/src/bpf_helpers.h
    "$prefix"/src/bpf_tracing.h
    "$prefix"/src/bpf_core_read.h
)

# Fetch libbpf release and extract the desired headers
curl -sL "https://github.com/libbpf/libbpf/archive/refs/tags/v${LIBBPF_VERSION}.tar.gz" | \
    tar -xz --xform='s#.*/##' -C ./include/ "${headers[@]}"

# Generating vmlinux.h, this works for kernel 5.15.0
# Reference: https://github.com/libbpf/libbpf/blob/master/docs/libbpf_overview.rst
bpftool btf dump file /sys/kernel/btf/vmlinux format c > ./include/vmlinux.h
