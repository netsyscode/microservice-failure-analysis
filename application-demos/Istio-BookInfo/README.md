# Deployment of Istio BookInfo Demo

This example deploys a sample application composed of four separate microservices used to demonstrate various Istio features.
This README is mainly derived from [https://istio.io/latest/docs/examples/bookinfo](https://istio.io/latest/docs/examples/bookinfo).

## Overview

The Bookinfo application is broken into four separate microservices:

- productpage. The productpage microservice calls the details and reviews microservices to populate the page.
- details. The details microservice contains book information.
- reviews. The reviews microservice contains book reviews. It also calls the ratings microservice.
- ratings. The ratings microservice contains book ranking information that accompanies a book review.

There are 3 versions of the reviews microservice:

- Version v1 doesn’t call the ratings service.
- Version v2 calls the ratings service, and displays each rating as 1 to 5 black stars.
- Version v3 calls the ratings service, and displays each rating as 1 to 5 red stars.

## Prerequisites

Ensure Istio is installed on your system. 
(In this example, we use Istio v1.21.0).

## Installation

```bash
kubectl apply -f ../../deps/istio/istio-1.21.0/samples/bookinfo/platform/kube/bookinfo.yaml
```

If you disabled automatic sidecar injection during installation and rely on manual sidecar injection, use the istioctl kube-inject command to modify the bookinfo.yaml file before deploying your application.
This part is not contained in this README.

## Confirmation

Confirm all services and pods are correctly defined and running:

```bash
$ kubectl get services
NAME          TYPE        CLUSTER-IP       EXTERNAL-IP   PORT(S)    AGE
details       ClusterIP   10.108.170.218   <none>        9080/TCP   89s
kubernetes    ClusterIP   10.96.0.1        <none>        443/TCP    93d
productpage   ClusterIP   10.98.32.9       <none>        9080/TCP   89s
ratings       ClusterIP   10.100.28.103    <none>        9080/TCP   89s
reviews       ClusterIP   10.103.51.253    <none>        9080/TCP   89s
```

and

```bash
$ kubectl get pods
NAME                             READY   STATUS    RESTARTS   AGE
details-v1-698d88b-28k6z         2/2     Running   0          105s
productpage-v1-675fc69cf-rkcz8   2/2     Running   0          105s
ratings-v1-6484c4d9bb-6l7vj      2/2     Running   0          105s
reviews-v1-5b5d6494f4-2zr9p      2/2     Running   0          105s
reviews-v2-5b667bcbf8-srwc4      2/2     Running   0          105s
reviews-v3-5b9bd44f4-5rj4w       2/2     Running   0          105s
```

To confirm that the Bookinfo application is running, send a request to it by a curl command from some pod, for example from ratings:

```bash
$ kubectl exec "$(kubectl get pod -l app=ratings -o jsonpath='{.items[0].metadata.name}')" -c ratings -- curl -sS productpage:9080/productpage | grep -o "<title>.*</title>"
<title>Simple Bookstore App</title>
```