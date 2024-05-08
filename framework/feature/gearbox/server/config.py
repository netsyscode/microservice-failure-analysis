import json

class AggregatorConfig:
    def __init__(self, ip, loader_port, manager_port, collector_port):
        self.ip = ip
        self.loader_port = loader_port
        self.manager_port = manager_port
        self.collector_port = collector_port

    def __str__(self):
        return f"AggregatorConfig(ip={self.ip}, loader_port={self.loader_port}, manager_port={self.manager_port}, collector_port={self.collector_port})"

class ManagerConfig:
    def __init__(self, ip, loader_port, aggregator_port, collector_port):
        self.ip = ip
        self.loader_port = loader_port
        self.aggregator_port = aggregator_port
        self.collector_port = collector_port

    def __str__(self):
        return f"ManagerConfig(ip={self.ip}, loader_port={self.loader_port}, aggregator_port={self.aggregator_port}, collector_port={self.collector_port})"

class CollectorConfig:
    def __init__(self, ip, aggregator_port, manager_port):
        self.ip = ip
        self.aggregator_port = aggregator_port
        self.manager_port = manager_port

    def __str__(self):
        return f"CollectorConfig(ip={self.ip}, aggregator_port={self.aggregator_port}, manager_port={self.manager_port})"

class Config:
    def __init__(self, aggregators, managers, collectors):
        self.aggregators = [AggregatorConfig(**ag) for ag in aggregators]
        self.managers = [ManagerConfig(**m) for m in managers]
        self.collectors = [CollectorConfig(**c) for c in collectors]

    def __str__(self):
        ag_str = ', '.join(str(ag) for ag in self.aggregators)
        mgr_str = ', '.join(str(mgr) for mgr in self.managers)
        col_str = ', '.join(str(col) for col in self.collectors)
        return f"Config(aggregators=[{ag_str}], managers=[{mgr_str}], collectors=[{col_str}])"

def parse_config(filename):
    with open(filename, 'r') as file:
        data = json.load(file)
    
    return Config(data['aggregators'], data['managers'], data['collectors'])
