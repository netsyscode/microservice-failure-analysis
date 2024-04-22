# Workloads for HTTP/1.x + TLS

## Overview
This directory contains code for a client/server workload that uses HTTP/1.1 + TLS/SSL encryption.

### Generate certs
This command generates a certificate and private key used for TLS/SSL encryption.
```bash
cd server/certs && openssl req -new -x509 -days 365 -nodes -out cert.pem -keyout key.pem
```

### Run server
This command runs the server and sets its listening port to 43421.
```bash
python3 ./server/https_server.py 43421
```

### Run client
This command makes a POST request to the server.
```bash
python3 ./client/client.py
```
