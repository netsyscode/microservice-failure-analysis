# Workloads for HTTP/1.1

## Overview
This directory contains code for an HTTP/1.x client/server workload that is implemented in Go.

### Run server
This command runs the server; the port it listens to is 8080.
```bash
sudo go run server/main.go
```

### Run client
This command makes a POST request to the server.
```bash
bash ./client/client.sh
```