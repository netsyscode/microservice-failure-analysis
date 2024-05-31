# Installation for OpenEBS (2.12.x)

Even though OpenEBS has been upgraded to 4.x version, this README installs 2.12.x for simplicity and compatibility for Train-Ticket.

## Presequisites

- [helm 3.2 or more](../helm/README.md)

## Installation

```bash
kubectl apply -f https://openebs.github.io/charts/openebs-operator.yaml
```

## Verifying OpenEBS installation

List the pods in `<openebs>` namespace with `kubectl get pods -n openebs`:

```
NAME                                            READY   STATUS    RESTARTS   AGE
openebs-localpv-provisioner-6787b599b9-nhh95    1/1     Running   0          3m5s
openebs-ndm-cluster-exporter-7bfd5746f4-k5xng   1/1     Running   0          3m5s
openebs-ndm-k2xmr                               1/1     Running   0          3m5s
openebs-ndm-node-exporter-hjhn4                 1/1     Running   0          3m5s
openebs-ndm-node-exporter-lx8vz                 1/1     Running   0          3m5s
openebs-ndm-operator-845b8858db-qwprc           1/1     Running   0          3m5s
openebs-ndm-rw2b6                               1/1     Running   0          3m5s
```

Verify StorageClasses with `kubectl get sc`:

```
NAME               PROVISIONER        RECLAIMPOLICY   VOLUMEBINDINGMODE      ALLOWVOLUMEEXPANSION   AGE
openebs-device     openebs.io/local   Delete          WaitForFirstConsumer   false                  3m32s
openebs-hostpath   openebs.io/local   Delete          WaitForFirstConsumer   false                  3m32s
```

## Test by deploying an application

Create the PersistentVolumeClaim: 
```bash
kubectl apply -f local-hostpath-pvc.yaml
```

Look at the PersistentVolumeClaim:

```bash
kubectl get pvc local-hostpath-pvc
```

The output shows that the `STATUS` is `Pending`. This means PVC has not yet been used by an application pod. The next step is to create a Pod that uses your PersistentVolumeClaim as a volume.

```bash
kubectl apply -f local-hostpath-pod.yaml
```

Verify

```bash
# that the container in the Pod is running
kubectl get pod hello-local-hostpath-pod

# that the data is being written to the volume
kubectl exec hello-local-hostpath-pod -- cat /mnt/store/greet.txt

# that the container is using the Local PV Hostpath
kubectl describe pod hello-local-hostpath-pod

# Look at the PersistentVolumeClaim again to see the details about the dynamically provisioned Local PersistentVolume
kubectl get pvc local-hostpath-pvc

# Look at the PersistentVolume details to see where the data is stored. Replace the PVC name with the one that was displayed in the previous step.
kubectl get pv <pvc-name> -o yaml
```

### Cleanup

```bash
kubectl delete -f local-hostpath-pod.yaml
kubectl delete -f local-hostpath-pvc.yaml
```