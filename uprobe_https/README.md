## Run sniffer
FOR CURRENT VERSION: start the http1s server before running the sniffer
```bash
sudo go run ./sniffer/main.go ./sniffer/ebpf_src.c --pid $(pgrep -f "python3 ./server/https_server.py 43421")
```

