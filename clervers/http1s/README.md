## Generate certs
```bash
cd main_src/server/certs
openssl req -new -x509 -days 365 -nodes -out cert.pem -keyout key.pem
```

## Run server
```bash
python3 ./server/https_server.py 43421
```

## Run client
```bash
python3 ./client/client.py
```
