import json

class ControllerConfig:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
    
    def __str__(self):
        return f"ControllerConfig(ip={self.ip}, port={self.port})"

class Config:
    def __init__(self, controller):
        self.controller = ControllerConfig(**controller)

    def __str__(self):
        return f"Config(controller=[{str(self.controller)}])"

def parse_config(filename):
    with open(filename, 'r') as file:
        data = json.load(file)
    
    return Config(data['controller'])
