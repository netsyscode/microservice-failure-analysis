import json

class ControllerConfig:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
    
    def __str__(self):
        return f"ControllerConfig(ip={self.ip}, port={self.port})"

class Config:
    def __init__(self, controllers):
        self.controllers = [ControllerConfig(**c) for c in controllers]

    def __str__(self):
        output_str = ','.join([str(c) for c in self.controllers])
        return f"Config(controller=[{output_str}])"

def parse_config(filename):
    with open(filename, 'r') as file:
        data = json.load(file)
    
    return Config(data['controller'])
