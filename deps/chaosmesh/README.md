# Installation for ChaosMesh

This README is to install ChaosMesh (Version 2.6.3).

## Prerequisites

- [Helm](../helm/README.md)

## Installation

Add the Chaos Mesh repository to the Helm repository:

```bash
helm repo add chaos-mesh https://charts.chaos-mesh.org
```

To see charts that can be installed, execute the following command:

```bash
helm search repo chaos-mesh
```

It is recommended to install Chaos Mesh under the chaos-mesh namespace, or you can specify any namespace to install Chaos Mesh:

```bash
kubectl create ns chaos-mesh
```

As the daemons of different container runtimes listen on different socket paths, you need to set the appropriate values during installation. You can run the following installation commands according to different environments.
In this repo, our cri is containerd: 

```bash
helm install chaos-mesh chaos-mesh/chaos-mesh -n=chaos-mesh --set chaosDaemon.runtime=containerd --set chaosDaemon.socketPath=/run/containerd/containerd.sock --set dashboard.securityMode=false --version 2.6.3
```

**Note:** To quickly create chaos experiments, we disable authentication with `--set dashboard.securityMode=false`.

## Verify the installation

To check the running status of Chaos Mesh, execute the following command:

```bash
kubectl get po -n chaos-mesh
```

The expected output is as follows:

```
NAME                                        READY   STATUS    RESTARTS   AGE
chaos-controller-manager-69fd5c46c8-xlqpc   3/3     Running   0          2d5h
chaos-daemon-jb8xh                          1/1     Running   0          2d5h
chaos-dashboard-98c4c5f97-tx5ds             1/1     Running   0          2d5h
```

## Uninstall

Before uninstall Chaos Mesh, please make sure that all the chaos experiments are deleted. You could list chaos related objects by executing:

```bash
for i in $(kubectl api-resources | grep chaos-mesh | awk '{print $1}'); do kubectl get $i -A; done
```
Once you make sure that all the chaos experiments are deleted, you can uninstall Chaos Mesh.

You could list the installed helm release by executing:

```bash
helm list -A
```

The output should look like:

```
NAME                    NAMESPACE       REVISION        UPDATED                                 STATUS          CHART                   APP VERSION
chaos-mesh              chaos-mesh      1               2021-12-01 22:58:18.037052401 +0800 CST deployed        chaos-mesh-2.1.0        2.1.0
```

It means that Chaos Mesh has been installed as a helm release named chaos-mesh-playground in namespace chaos-mesh. So here is the target release to uninstall.

```bash
helm uninstall chaos-mesh -n chaos-mesh
```

helm uninstall would not remove the CRDs, so you could remove them manually by executing:

```bashs
kubectl delete crd $(kubectl get crd | grep 'chaos-mesh.org' | awk '{print $1}')
```

## References

[Install Chaos Mesh using Helm](https://chaos-mesh.org/zh/docs/production-installation-using-helm/)