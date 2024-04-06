## Run server
```bash
sudo go build -o /tmp/grpc_server server/main.go && /tmp/grpc_server
```

## Run client
```bash
sudo go build -o ./tmp/grpc_client client/main.go && ./tmp/grpc_client --count 1
```