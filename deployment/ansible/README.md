# Installation Scripts for K8S

This repository contains scripts for installing and removing k8s clusters with `ansible` in the Ubuntu system.

## Scripts Description

-`install_k8s.yaml`: Change the hostname of the host, install containerd, sealos, k8s, helm, etc.

-`clear_k8s.yaml`: Clean up the k8s cluster.

-`inventory.ini`: Need to specify the IP, username, password, proxy port, etc. of the cluster in this file.

## Usage  

Before installing k8s, you need to complete the configuration of the environment in `inventory.ini`, including the IP ( `ansible_host` ), username ( `ansible_user` ), and password of the machines ( `ansible_become_pass` ) in the cluster. At the same time, since installing sealos and pulling images require a proxy, you need to specify the IP and port number of the `proxy`.

Then execute the following command to install the k8s cluster.

```bash

ansible-playbook main.yaml

```

Clean up the cluster with the following command.

```bash
ansible-playbook -i h
```