# Workload Generator

This guide explains how to build the workload benchmarking tool Docker image from the provided Dockerfile, and deploy it to a Kubernetes cluster using the supplied YAML file.

## Prerequisites

- Docker installed on your local machine. (In this case, we use Docker version 24.0.7)
- Access to a Kubernetes cluster with kubectl configured to communicate with your cluster.

## Installation w/o Modification to the Dockerfile

```bash
kubectl apply -f loadgen-deployment.yml
```

## Attach to the Workload Generator

```bash
kubectl exec -it -n loadgen $(kubectl -n loadgen get pods -o name) -- /bin/sh
```

## Modification of the Dockerfile

```bash
docker build -t <your-dockerhub-username>/loadgen .
docker push <your-dockerhub-username>/loadgen
```

## Cleaning Up

```bash
kubectl delete -f loadgen-deployment.yml
```