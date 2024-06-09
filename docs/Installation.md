# Detailed Deployment and Installation Instructions

This project uses Ansible to automate the installation and configuration of various components like Kubernetes, LibBPF, BPFTool, and OpenEBS.

## Directory Structure

The deployment scripts are under `./deployment/ansible/`, including

```
./deployment/ansible/
├── apps/
├── tools/
├── cleanup.yaml
├── install_bpftool.yaml
├── install_k8s.yaml
├── install_libbpf.yaml
├── inventory.ini
├── main.yaml
├── README.md
├── set_env.yaml
├── shared_vars.yaml
└── update_repo.yaml
```

## Inventory File

Your [`inventory.ini`](../deployment/ansible/inventory.ini) file should look like this:

```ini
[controller]
main ansible_host=CONTROLLER_HOST ansible_user=CONTROLLER_USER ansible_become_pass=USER_PASSWD ssh_key=CONTROLLER_SSH_KEY proxy=CONTROLLER_PROXY

[workers]
worker1 ansible_host=WORKER1_HOST ansible_user=WORKER_USER ansible_become_pass=USER_PASSWD proxy=WORKER1_PROXY
worker2 ansible_host=WORKER2_HOST ansible_user=WORKER_USER ansible_become_pass=USER_PASSWD proxy=WORKER2_PROXY
```

## Main Playbook

`main.yaml` is the main playbook that sequentially imports other playbooks:

```yaml
---
- import_playbook: set_env.yaml
- import_playbook: install_k8s.yaml
- import_playbook: update_repo.yaml
- import_playbook: install_libbpf.yaml
- import_playbook: install_bpftool.yaml
...
```

## Playbook Descriptions

- **set_env.yaml**: Configures environment variables.
- **install_k8s.yaml**: Installs Sealos (v4.3.7), Kubernetes (v1.24.17) with Flannel (v0.19.2), and Helm (v3.13.2).
- **update_repo.yaml**: Updates and syncs this github repository to worker nodes.
- **install_libbpf.yaml**: Installs LibBPF (v1.2.0).
- **install_bpftool.yaml**: Installs BPFTool (v7.2.0).
- **cleanup.yaml**: Cleanup the kubernetes cluster.

## Usage

1. Configure the `inventory.ini` file according to your environment.
2. Execute the main playbook under the root directory of this repo:
    ```sh
    ansible-playbook ./deployment/ansible/main.yaml
    ```
3. Cleanup with
    ```sh
    ansible-playbook ./deployment/ansible/cleanup.yaml
    ```

### Advanced Installation

We also provide: 

- **apps/train-ticket/main.yaml**: Install application demo [Train Ticket](https://github.com/FudanSELab/train-ticket).
  - For cleanups, use **apps/train-ticket/cleanup.yaml**.
- **apps/deathstar/main.yaml**: Install application demo [DeathStarBench](https://github.com/delimitrou/DeathStarBench/).
  - For hotelReservation, run with `-e "ds_type=hotelReservation"`.
  - For mediaMicroservices, run with `-e "ds_type=mediaMicroservices"`.
  - For socialNetwork, run with `-e "ds_type=socialNetwork"`.
  - For cleanups, use **apps/deathstar/main.yaml**.
- **tools/chaosmesh.yaml**: Installs [Chaos Mesh (v2.6.3)](https://chaos-mesh.org/docs/production-installation-using-helm/).
  - For cleanups, use **tools/cleanup_chaosmesh.yaml**.

**Notes for advanced installation:**

1. If not specified, all ansible scripts should be run under the root dir of this repo (for ansible to obtain the [ansible.cfg](../ansible.cfg)).
2. If not specified, no parameters are required.

## Notes

- Ensure that all hosts have been properly configured for SSH key access before running these playbooks.
- If you encounter any issues while executing any playbook, please refer to the relevant Ansible documentation or look at the comments within the playbook files for more information.
