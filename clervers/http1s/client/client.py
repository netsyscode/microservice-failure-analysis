import requests
body = {"Hello World": "I'm testing my HTTPS sniffer", "Results=": "Good"}
resp = requests.post("https://127.0.0.1:43421/example", json=body, verify=False)