# Kprobe Sniffer for HTTP/2.0 Observability

## Overview

This directory contains the source code for an HTTP/2.0 packet sniffer leveraging eBPF programs attached to kprobes in the kernel. This allows for non-intrusive observability for HTTP/2.0 protocols while also offering high performance under minimal maintainence effort.

We run a user-space agent that attaches eBPF programs to syscalls and handles the transfer of data between user-space and kernel-space. Using the documentation provided by the HTTP/2.0 and HPACK RFC, the agent is able to parse inbound and outbound traffic to observe header metadata.

## Components

- **module_src**: Contains Golang implementations of attaching kprobes, structs for communication between user-space and kernel-space, read/write buffers etc.
- **sniffer**: Contains source code for the user-space agent and eBPF programs.

## Dependencies
Please refer to the `ebpf_http_dependencies` directory.

## Usage
Run the sniffer using the following command
```bash
sudo go run ./sniffer/main.go ./sniffer/kprobe_http2_src.c
```
Ensure that you have the necessary packages installed and the correct version of bcc (v0.24.0) compiled. Exit the sniffer using Ctrl-C.

## Note
This sniffer supports the observation of HTTP/2.0 through kprobes. In tandem with kprobe support for HTTP/1.x and uprobe support for HTTPS, this framework allows for the thorough observation of the HTTP protocol while remaining non-intrusive and achieving high performance.

We also plan to use Grafana to provide a better interface for users, replacing the current mode of outputting to the terminal.
