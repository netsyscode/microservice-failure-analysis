# Uprobe Sniffer for HTTPS Observability

## Overview

This directory contains the source code for an HTTPS packet sniffer leveraging eBPF programs attached to uprobes in user-space. This allows for non-intrusive observability for HTTPS protocols under minimal maintainence effort.

We run a user-space agent that attaches eBPF programs to SSL_read, SSL_write, transport.(*loopyWriter).writeHeader and transport.(*http2Server).operateHeaders.  The agent also handles the transfer of data between user-space and kernel-space, and outputs the captured data to the terminal.

## Components

- **module_src**: Contains Golang implementations of attaching uprobes, structs for communication between user-space and kernel-space, read/write buffers etc.
- **sniffer**: Contains source code for the user-space agent and eBPF programs.

## Dependencies
Please refer to the `ebpf_http_dependencies` directory.

## Usage
FOR CURRENT VERSION: start the http1s server before running the sniffer
```bash
sudo go run ./sniffer/main.go ./sniffer/ebpf_src.c --pid $(pgrep -f "python3 ./server/https_server.py 43421")
```
Ensure that you have the necessary packages installed and the correct version of bcc (v0.24.0) compiled. Exit the sniffer using Ctrl-C.

## Note
At the current stage, this sniffer only supports the observation of HTTPS through uprobes. This will be upgraded later to support the attachment of kprobes as well as HTTP/1.x and HTTP/2.0 observability.

We also plan to use Grafana to provide a better interface for users, replacing the current mode of outputting to the terminal.
