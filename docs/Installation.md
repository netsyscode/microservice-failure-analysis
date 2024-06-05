# Detailed Deployment and Installation Instructions

This project uses Ansible to automate the installation and configuration of various components like Kubernetes, LibBPF, BPFTool, and OpenEBS.

## Directory Structure

The deployment scripts are under `./deployment/ansible/`, including

```
./deployment/ansible/
├── cleanup.yaml
├── install_bpftool.yaml
├── install_k8s.yaml
├── install_libbpf.yaml
├── install_openebs.yaml
├── inventory.ini
├── main.yaml
├── README.md
├── set_env.yaml
└── update_repo.yaml
```

## Inventory File

Your [`inventory.ini`](../deployment/ansible/inventory.ini) file should look like this:

```ini
[controller]
main ansible_host=CONTROLLER_HOST ansible_user=CONTROLLER_USER ssh_key=CONTROLLER_SSH_KEY proxy=CONTROLLER_PROXY

[workers]
worker1 ansible_host=WORKER1_HOST ansible_user=WORKER_USER proxy=WORKER_PROXY
worker2 ansible_host=WORKER2_HOST ansible_user=WORKER_USER proxy=WORKER_PROXY
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

- **set_env.yaml**: Configures environment variables
- **install_k8s.yaml**: Installs Sealos (v4.3.7), Kubernetes (v1.24.17) with Flannel (v0.19.2), and Helm (v3.13.2)
- **update_repo.yaml**: Updates and syncs this github repository to worker nodes
- **install_libbpf.yaml**: Installs LibBPF (v1.2.0)
- **install_bpftool.yaml**: Installs BPFTool (v7.2.0)
- **install_openebs.yaml**: Installs OpenEBS (v3.9.0)
- **cleanup.yaml**: Cleanup script

## Usage

1. Configure the `inventory.ini` file according to your environment.
2. Execute the main playbook under the root directory of this repo:
    ```sh
    ansible-playbook ./deployment/ansible/main.yaml
    ```

## Notes

- Ensure that all hosts have been properly configured for SSH key access before running these playbooks.
- If you encounter any issues while executing any playbook, please refer to the relevant Ansible documentation or look at the comments within the playbook files for more information.
