#!/bin/bash

SERVICE_NAME="service1"
NAMESPACE="default"

# 设置默认循环次数
DEFAULT_LOOP_COUNT=100

# 从命令行参数获取循环次数，如果没有提供参数，则使用默认值
LOOP_COUNT="${1:-$DEFAULT_LOOP_COUNT}"

CLUSTER_IP=$(kubectl get svc $SERVICE_NAME -n $NAMESPACE -o=jsonpath='{.spec.clusterIP}')

PORT=$(kubectl get svc $SERVICE_NAME -n $NAMESPACE -o=jsonpath='{.spec.ports[0].port}')

if [ -z "$CLUSTER_IP" ] || [ -z "$PORT" ]; then
    echo "Failed to get the ClusterIP or port of the service: $SERVICE_NAME"
    exit 1
fi

URL="http://${CLUSTER_IP}:${PORT}/"
echo "URL: $URL"

echo "Example response:"
curl "$URL"
echo ""
echo "Example response end."

for i in $(seq 1 $LOOP_COUNT); do
  response=$(curl -s "$URL")
  echo "Response: $response"
done
