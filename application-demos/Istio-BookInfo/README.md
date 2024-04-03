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

- Ensure Istio is installed on your system. (In this example, we use Istio v1.21.0). Check [installation instructions](../../deps/istio/README.md).
- If you are to deploy this demo with Jaeger, please make sure [jaeger and jaeger tasks are installed via istio](../../deps/istio/Observability.md).

## Installation

```text
kubectl apply -f ../../deps/istio/istio-1.21.0/samples/bookinfo/platform/kube/bookinfo.yaml
```

If you disabled automatic sidecar injection during installation and rely on manual sidecar injection, use the istioctl kube-inject command to modify the bookinfo.yaml file before deploying your application.
This part is not contained in this README.

## Confirmation

Confirm all services and pods are correctly defined and running:

```text
$ kubectl get services
NAME          TYPE        CLUSTER-IP       EXTERNAL-IP   PORT(S)    AGE
details       ClusterIP   10.108.170.218   <none>        9080/TCP   89s
kubernetes    ClusterIP   10.96.0.1        <none>        443/TCP    93d
productpage   ClusterIP   10.98.32.9       <none>        9080/TCP   89s
ratings       ClusterIP   10.100.28.103    <none>        9080/TCP   89s
reviews       ClusterIP   10.103.51.253    <none>        9080/TCP   89s
```

and

```text
$ kubectl get pods
NAME                             READY   STATUS    RESTARTS   AGE
details-v1-698d88b-28k6z         2/2     Running   0          105s
productpage-v1-675fc69cf-rkcz8   2/2     Running   0          105s
ratings-v1-6484c4d9bb-6l7vj      2/2     Running   0          105s
reviews-v1-5b5d6494f4-2zr9p      2/2     Running   0          105s
reviews-v2-5b667bcbf8-srwc4      2/2     Running   0          105s
reviews-v3-5b9bd44f4-5rj4w       2/2     Running   0          105s
```

## Test and Evaluation

To confirm that the Bookinfo application is running, send a request to it by a curl command from some pod, for example from ratings:

```text
$ kubectl exec "$(kubectl get pod -l app=ratings -o jsonpath='{.items[0].metadata.name}')" -c ratings -- curl -sS productpage:9080/productpage | grep -o "<title>.*</title>"
<title>Simple Bookstore App</title>
```

or run `wrk` in the `loadgen` pod in `loadgen` namespace created by `./evaluation/worload-generator`:

```text
/ # wrk -t16 -c64 -d30s -R100 http://productpage.default:9080/productpage
Running 30s test @ http://productpage.default:9080/productpage
  16 threads and 64 connections
  Thread calibration: mean lat.: 423.266ms, rate sampling interval: 1035ms
  Thread calibration: mean lat.: 417.916ms, rate sampling interval: 1036ms
  Thread calibration: mean lat.: 412.958ms, rate sampling interval: 1036ms
  Thread calibration: mean lat.: 397.127ms, rate sampling interval: 1035ms
  Thread calibration: mean lat.: 401.314ms, rate sampling interval: 1031ms
  Thread calibration: mean lat.: 430.609ms, rate sampling interval: 1041ms
  Thread calibration: mean lat.: 413.601ms, rate sampling interval: 1037ms
  Thread calibration: mean lat.: 399.915ms, rate sampling interval: 1036ms
  Thread calibration: mean lat.: 464.588ms, rate sampling interval: 1044ms
  Thread calibration: mean lat.: 427.173ms, rate sampling interval: 1037ms
  Thread calibration: mean lat.: 442.646ms, rate sampling interval: 1036ms
  Thread calibration: mean lat.: 461.184ms, rate sampling interval: 1043ms
  Thread calibration: mean lat.: 459.603ms, rate sampling interval: 1053ms
  Thread calibration: mean lat.: 454.717ms, rate sampling interval: 1050ms
  Thread calibration: mean lat.: 457.592ms, rate sampling interval: 1060ms
  Thread calibration: mean lat.: 464.297ms, rate sampling interval: 1041ms
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   703.71ms  508.48ms   2.35s    78.79%
    Req/Sec     5.19      1.66     7.00     94.70%
  2883 requests in 30.03s, 14.23MB read
Requests/sec:     96.00
Transfer/sec:    485.18KB
```

or run the little test script inside the cluster:

```text
chmod +x ./test.sh && ./test.sh
```

### Groundtruth trace extraction

This leverages the python program located at `evaluation/trace-extraction/jaeger`:

```text
cd ../../evaluation/trace-extraction/jaeger/ && python3 jaeger_query.py --jaeger-address $(kubectl -n istio-system get svc tracing -o=jsonpath='{.spec.clusterIP}') --jaeger-port 16685 --service-name productpage.default --start-time "1d ago" && mv ./jaeger_query_results.json ../../../application-demos/Istio-BookInfo/ && cd -
```

## Cleanup

```text
$ ../../deps/istio/istio-1.21.0/samples/bookinfo/platform/kube/cleanup.sh
namespace ? [default]
using NAMESPACE=default
Application cleanup may take up to one minute
service "details" deleted
serviceaccount "bookinfo-details" deleted
deployment.apps "details-v1" deleted
service "ratings" deleted
serviceaccount "bookinfo-ratings" deleted
deployment.apps "ratings-v1" deleted
service "reviews" deleted
serviceaccount "bookinfo-reviews" deleted
deployment.apps "reviews-v1" deleted
deployment.apps "reviews-v2" deleted
deployment.apps "reviews-v3" deleted
service "productpage" deleted
serviceaccount "bookinfo-productpage" deleted
deployment.apps "productpage-v1" deleted
Application cleanup successful
```

You might need to [uninstall Istio](../../deps/istio/README.md#uninstallation) too.

## References

- [https://istio.io/latest/docs/examples/bookinfo/](https://istio.io/latest/docs/examples/bookinfo/)