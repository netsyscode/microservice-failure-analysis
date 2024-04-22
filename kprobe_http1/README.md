# Kprobe Sniffer for HTTP/1.x Observability

## Overview

This directory contains the source code for an HTTP/1.x packet sniffer leveraging eBPF programs attached to kprobes in the kernel. This allows for non-intrusive observability for HTTP/1.x protocols while also offering high performance under minimal maintainence effort.

We run a user-space agent that attaches eBPF programs to syscalls that are integral for HTTP/1.x communication, handles the transfer of data between user-space and kernel-space, and outputs the captured data to the terminal.

## Components

- **module_src**: Contains Golang implementations of attaching kprobes, structs for communication between user-space and kernel-space, read/write buffers etc.
- **sniffer**: Contains source code for the user-space agent and eBPF programs.

## Dependencies
Please refer to the `ebpf_http_dependencies` directory.

## Usage
Run the sniffer using the following command
```bash
sudo go run ./sniffer/main.go ./sniffer/kprobe_http1_src.c
```
Ensure that you have the necessary packages installed and the correct version of bcc (v0.24.0) compiled. Exit the sniffer using Ctrl-C.

## Note
At the current stage, this sniffer only supports the observation of HTTP/1.x through kprobes. This will be upgraded later to support uprobes as well as observability for HTTP/2.0 and HTTPS protocols.

We also plan to use Grafana to provide a better interface for users, replacing the current mode of outputting to the terminal.
