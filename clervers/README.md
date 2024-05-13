# HTTP client/server benchmarks
Just for clarification, `clervers` stands for `clients and servers` : )

## Overview

This directory contains the source code for client/server workloads that cover the following HTTP configurations:
- HTTP/1.x
- HTTP/1.x with TLS encryption
- HTTP/2.0
- HTTP/2.0 with TLS encrpytion

## Components

- **http1**: A client/server workload using HTTP/1.1 for communication. Implemented in Go.
- **http1s**: A client/server workload using HTTP/1.1 with TLS encryption. Implemented in Python.
- **http2**: A client/server workload using HTTP/2.0 for communication. Implemented in Go.
- **http2s**: (TODO) A client/server workload using HTTP/2.0 with TLS encryption. 


## Usage

In each folder, there is a README file with specific instructions on how to run the server and client for that configuration.

Ensure you have installed all dependencies in the `ebpf_http_dependencies` directory.


## Note
At the current stage, only the workloads for HTTP/1.x, HTTP/1.x with TLS encryption, and HTTP/2.0 have been implemented. Workloads for HTTP/2.0 with TLC encryption will be added shortly.

Additionally, current workloads only feature simple logic under their given configurations. This will be updated to feature more complex logic and interactions. 

Benchmarks for measuring the performance and coverage of our proposed sniffers will also be featured in this directory. 