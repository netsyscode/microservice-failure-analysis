# DeathStarBench README

This repository contains several Ansible playbooks used to deploy and configure the `DeathStarBench` application suite on a Kubernetes cluster. 
Below is a detailed explanation of each playbook.

---

## Playbook 1: Helm Install DeathStar Based on Parameter

This playbook installs the DeathStarBench application using Helm charts based on the provided parameter.

### Variables

- `ds_type`: Determines which Helm chart to install. Valid options are `hotel`, `media`, and `social`.

### Example Usage

```sh
ansible-playbook deployment/ansible/apps/deathstar/main.yaml -e "ds_type=hotel"

# or
ansible-playbook deployment/ansible/apps/deathstar/main.yaml -e "ds_type=media"

# or
ansible-playbook deployment/ansible/apps/deathstar/main.yaml -e "ds_type=social"
```

---

## Playbook 2: Register Users and Construct Social Graphs

This playbook init the necessary infrastructure to register users and construct social graphs for the `DeathStarBench` social network.

**Note:** Currently we only support socialNetwork init.

### Variables

- `graph`: The graph size option provided by the user (one of `s`, `m`, `l`).

### Example Usage

```sh
ansible-playbook deployment/ansible/apps/deathstar/init_sn.yaml -e "graph=s"

# or
ansible-playbook deployment/ansible/apps/deathstar/init_sn.yaml -e "graph=m"

# or
ansible-playbook deployment/ansible/apps/deathstar/init_sn.yaml -e "graph=l"
```

---

## Playbook 3: Register Users and Construct Social Graphs

This playbook build wrk2 and deploy workload pod for the `DeathStarBench` social network.

### Example Usage

```sh
ansible-playbook deployment/ansible/apps/deathstar/workload.yaml

# Then run the command output by ansible:
kubectl exec -it deathstar-workload-job-6975f5b68d-5kqf8 -- /bin/sh

# Compose posts
./wrk -D exp -t 8 -c 16 -d 10 -L -s ../social-network/compose-post.lua http://10.96.1.24:8080/wrk2-api/post/compose -R 100

# Read home timelines
./wrk -D exp -t 8 -c 16 -d 10 -L -s ../social-network/read-home-timeline.lua http://10.96.1.24:8080/wrk2-api/home-timeline/read -R 100

# Read user timelines
./wrk -D exp -t 8 -c 16 -d 10 -L -s ../social-network/read-user-timeline.lua http://10.96.1.24:8080/wrk2-api/user-timeline/read -R 100


```

---

## Playbook 4: Cleanup Helm Installations

This playbook is used for cleaning up specific Helm releases. It attempts to uninstall the Helm releases and outputs the result for debugging purposes. Each uninstallation step ignores errors to ensure other steps proceed even if one fails.

### Helm Releases to Uninstall

- hotelReservation: Helm release for the hotel reservation service.
- mediaMicroservices: Helm release for the media microservices.
- socialNetwork: Helm release for the social network service.

### Example Usage

```sh
ansible-playbook deployment/ansible/apps/deathstar/cleanup.yaml
```