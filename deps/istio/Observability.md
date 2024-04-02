# Observability

## Set the Sampling Rate

When you enable tracing, you can set the sampling rate that Istio uses for tracing. 
Use the `meshConfig.defaultConfig.tracing.sampling` option during installation to set the sampling rate. 
*The default sampling rate is 1%.*

You can modify the `./tracing.yaml` in this folder and run:

```text
istioctl install -f ./tracing.yml
```

**Note:** Remember to redeploy Istio BookInfo demo applications after resetting sampling rates.

## Jaeger

For quick installation:

```text
kubectl apply -f ./istio-1.21.0/samples/addons/jaeger.yaml
```

Accessing the dashboard:

```text
istioctl dashboard jaeger
```

Cleanup:

```text
kubectl delete -f ./istio-1.21.0/samples/addons/jaeger.yaml
```

## Zipkin

For quick installation:

```text
kubectl apply -f ./istio-1.21.0/samples/addons/extras/zipkin.yaml
```

Accessing the dashboard:

```text
istioctl dashboard zipkin
```

Cleanup:

```text
kubectl delete -f ./istio-1.21.0/samples/addons/extras/zipkin.yaml
```

## References

- [Jaeger Installation](https://istio.io/latest/docs/ops/integrations/jaeger/#installation)
- [Jaeger Tasks](https://istio.io/latest/docs/tasks/observability/distributed-tracing/jaeger/)
- [Setting the sampling rate](https://istio.io/latest/docs/tasks/observability/distributed-tracing/mesh-and-proxy-config/#customizing-trace-sampling)