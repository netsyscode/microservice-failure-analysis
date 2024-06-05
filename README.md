# Microservice Failure Analysis

## Quick Start

### Prerequisites

A cluster with more than 3 nodes, each with `python` and [ansible](https://www.ansible.com/) installed.

### Basic Installation

For non-root users, clone this repo under `/home/<user-name>` at `<user-name>@<main-host-ip>`.
Modify [inventory.ini](./deployment/ansible/inventory.ini) run 

```bash
ansible-playbook ./deployment/ansible/main.yaml
```

For detailed instructions, see [Detailed Deployment and Installation Instructions](./docs/Installation.md).

### Feature Installation

## Currently Supported Environments

This repo is tested deployable and runnable with 

- Ansible v2.16.7
- Python v3.10.12
- Sealos v4.3.7
- Kubernetes v1.24.17 with 
  - Flannel v0.19.2
- Helm v3.13.2

... and on: 

- Kernel version 5.15.0-105-generic
- Ubuntu 22.04.2 LTS
