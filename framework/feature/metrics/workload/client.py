
# docker build --network=host -t flask-server .
# docker run --name flask-s -p 5000:5000 -d flask-server

import requests

response = requests.get('http://localhost:5000/api')
print(response.json())