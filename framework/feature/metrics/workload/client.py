
# docker build --network=host -t flask-server .
# docker run --name flask-s -p 5000:5000 -d flask-server

import requests
import sys

response = requests.get(f'http://localhost:{sys.argv[1]}/api')
print(response.json())