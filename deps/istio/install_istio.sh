#!/bin/bash

# Enter the Istio directory
ISTIO_DIR=$(find . -maxdepth 1  -name 'istio-*' -type d | sort -r | head -n1)
if [ -z "$ISTIO_DIR" ]; then
    echo "Failed to download or extract Istio."
    exit 1
fi

cd "$ISTIO_DIR"

# Add Istio's bin directory to user's PATH
echo "Adding Istio bin directory to user's PATH..."
CURRENT_BIN_PATH="$(pwd)/bin"
if ! echo $PATH | grep -qE "(^|:)$CURRENT_BIN_PATH($|:)"; then
  echo "export PATH=$CURRENT_BIN_PATH:\$PATH" >> ~/.bashrc
else
  echo "Istio bin directory exist in user's PATH"
fi
export PATH=$PWD/bin:$PATH

# Install Istio
echo "Installing Istio..."
istioctl install --set profile=default -y

# Enable Istio injection for the default namespace
echo "Enabling Istio injection for the default namespace..."
kubectl label namespace default istio-injection=enabled --overwrite

LABELS=$(kubectl get namespace default -o jsonpath='{.metadata.labels}')
if [[ $LABELS == *"istio-injection"* ]]; then
  echo "The 'istio-injection' label is successfully set on the 'default' namespace."
  ISTIO_INJECTION_LABEL_VALUE=$(kubectl get namespace default -o=jsonpath='{.metadata.labels.istio-injection}')
  if [ "$ISTIO_INJECTION_LABEL_VALUE" == "enabled" ]; then
    echo "The 'istio-injection' label is successfully set to 'enabled' on the 'default' namespace."
  else
    echo "Failed: The 'istio-injection' label is not 'enabled' on the 'default' namespace."
  fi
else
  echo "Failed: The 'default' namespace does not have the 'istio-injection' label."
fi

echo "Istio installation complete."
