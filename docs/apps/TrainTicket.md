# TrainTicket README

This repository contains several Ansible playbooks used to deploy and configure the `Trainticket` application suite on a Kubernetes cluster.
Below is a detailed explanation of each playbook.

---

## Playbook 1: Deploy the trainticket application in one step.

Mainly includes two sub-playbooks. `install_openebs.yaml` installs OpenEBS to create and manage persistent storage volumes (PV) in k8s.` install_tt.yaml` modifies the relevant configuration of the application and deploys the application..

### Example Usage

```sh
ansible-playbook deployment/ansible/apps/train-ticket/main.yaml 

```

### Note

1. Possibly due to insufficient system resources, there may be some SQL pods failing when executing the function `deploy_tt_mysql_each_service` during deployment. If such a situation occurs, add `sleep * `in `deploy_tt_mysql_each_service `to alleviate it.
2. The current persistent storage part of trainticket has some bugs. If the application generates database content, certain service pods may fail during the next deployment. Clear the persistent storage volume with `kubectl delete pvc --all` before the second deployment and then proceed with the deployment.
