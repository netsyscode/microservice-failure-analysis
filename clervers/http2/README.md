# Workloads for HTTP/2.0

## Overview
This directory contains code for an HTTP/2.0 client/server workload that is implemented using gRPC.

### Run server
This command builds and runs the gRPC server.
```bash
sudo go build -o /tmp/grpc_server server/main.go && /tmp/grpc_server
```

### Run client
This command makes a request to the server and awaits for its reply, for a total of 2 times.
```bash
sudo go build -o /tmp/grpc_client client/main.go && /tmp/grpc_client --count 2
```

## Note
The commands in this README file stores the compiled executables in the `/tmp` folder. Ensure you have the necessary priviliges to access and run the workload.

Will add scripts to remove the compiled files to preserve tidiness if deemed necessary. Users can also manually change the output location to the current directory.