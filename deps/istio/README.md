# Manual Installation for Istio

This installation guide is mainly copied from [https://istio.io/latest/docs/](https://istio.io/latest/docs/) and [https://istio.io/latest/docs/setup/install/istioctl](https://istio.io/latest/docs/setup/install/istioctl).
Among the current installation methods, we choose to install it with istioctl.

## Prerequisites

- Ensure `curl` is installed on your system.
- Ensure `kubectl` is installed and you are connected to a Kubernetes cluster.
- This installation requires to access [www.github.com](www.github.com)

## Usage

Simply run the `install_istio.sh`.
This script automatically downloads and installs the latest version of Istio on your current Linux system. 
It also adds Istio's `bin` directory to the current user's `PATH`. 
Additionally, it enables Istio injection for the `default` namespace.

```bash
chmod +x install_istio.sh && ./install_istio.sh
```

which mainly conducts the following instructions:

```bash
# Download
curl -L https://istio.io/downloadIstio | sh -
cd "$ISTIO_DIR"
echo "export PATH=\$PWD/bin:\$PATH" >> ~/.bashrc

# Installation
istioctl install --set profile=demo -y
kubectl label namespace default istio-injection=enabled
```

## Additional Information

By the time this documetation was written, it installs Istio 1.21.0.

## Uninstallation

To completely uninstall Istio from a cluster, run the following command:

```bash
istioctl uninstall --purge
```

The optional --purge flag will remove all Istio resources, including cluster-scoped resources that may be shared with other Istio control planes.

## References

- [https://istio.io/latest/docs/](https://istio.io/latest/docs/)
- [https://istio.io/latest/docs/setup/install/istioctl](https://istio.io/latest/docs/setup/install/istioctl)