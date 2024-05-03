# Gearbox eBPF Program Documentation

This document provides an overview of the eBPF program developed for Gearbox, detailing its instrumentation hooks for monitoring network traffic efficiently. The eBPF (Extended Berkeley Packet Filter) program is designed to hook into various system call entry and exit points, as well as socket operations, to observe and potentially alter the behavior of network communication.

## Gearbox Documentation Overview

This directory contains detailed documentation for various components of the Gearbox tool. Each subdirectory focuses on different aspects of Gearbox, providing insights into its implementation and usage. Below is an overview of what each documentation file covers:

### Files

- [**Hooks.md**](./Hooks.md)
  - Description: Details the specific system hooks that Gearbox utilizes to monitor and interact with the system. This includes hooks into network events, system calls, and other critical points that Gearbox uses to extract or inject data.
- [**Maps.md**](./Maps.md)
  - Description: Outlines the various maps used by Gearbox for storing and accessing data efficiently. It describes each map's purpose, structure, and role in the overall functionality of the tool.
- [**Structures.md**](./Structures.md)
  - Description: Discusses the data structures defined and used by Gearbox. This file is crucial for understanding how data is organized, manipulated, and stored during the operation of Gearbox.
- [**Usage.md**](./Usage.md)
  - Description: Provides step-by-step instructions on how to set up and use the Gearbox tool. It includes details on configuration, deployment, and operational best practices to help users maximize the tool's capabilities.
- [**Overview.md**](./Overview.md)
  - Description: Provides an in-depth explanation of how context propagation is implemented in Gearbox. It covers the mechanisms used to track and manage context across different parts of the system.


Each document is designed to provide a comprehensive understanding of its respective topic, ensuring that developers and users alike can get the most out of Gearbox.

Please refer to each individual file for more detailed information on each topic.

## Dependencies

Gearbox requires the same dependencies of this git repo, which includes:

- libbpf v1.2.0
- bpftools v7.2.0

## Source Code Architecture

## Tested Environments

Gearbox is tested deployable and runnable on 

- Kubernetes v1.29.0, Kernel version 5.15.0-105-generic, Ubuntu 22.04.2 LTS