import socket
import threading
import time
from ctypes import *

from util import parse_arg, save_data
from structs import AggrPoint, AggrMetric

TRIGGER_THRESHOLD = 5000

class Collector:
    def __init__(self, host, port, aggr_server_list, output_path="./result.json", threshold=TRIGGER_THRESHOLD):
        self.host = host
        self.port = port
        self.threshold = threshold
        self.path_num = 0
        self.pathid_set = {}
        self.path_threshold_event = threading.Event()
        self.lock = threading.Lock()
        self.aggr_server_list = aggr_server_list
        self.output = output_path

    def start_server(self):
        threading.Thread(target=self.process).start()

        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
            udp_socket.bind((self.host, self.port))

            while True:
                data, client_address = udp_socket.recvfrom(1024)
                path_id = int.from_bytes(data, 'little')
                udp_socket.sendto(b'ACKED', client_address)

                with self.lock:
                    self.pathid_set[path_id] += 1
                    self.path_num += 1

                if self.path_num >= self.threshold:
                    self.path_threshold_event.set()

    def process(self):
        while True:
            self.path_threshold_event.wait()
            self.path_threshold_event.clear()

            print("Path threshold reached, processing data...")

            with self.lock:
                local_pathid_set = self.pathid_set.copy()
                self.pathid_set.clear()
                self.path_num = 0

            for path_id, count in local_pathid_set.items():
                path_data = self.collect_aggr_data(path_id)
                save_data({"time": time.time(), "pathnum": count, "path_id": path_data}, self.output_path)

    def collect_aggr_data(self, path_id):
        """Aggregate data from multiple servers for a given path_id."""
        aggregated_data = []
        for address, port in self.aggr_server_list:
            aggregated_data.extend(self.request_aggr_data(address, port, path_id))
        return aggregated_data

    def request_aggr_data(self, address, port, path_id):
        """Request data for a path_id from a specific aggregation server."""
        aggregated_data = []
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
            udp_socket.settimeout(3)
            request = path_id.to_bytes(32, 'little')
            udp_socket.sendto(request, (address, port))
            try:
                data, _ = udp_socket.recvfrom(1024)
                length = int.from_bytes(data, 'little')

                if length > 0:
                    for _ in range(length):
                        data, _ = udp_socket.recvfrom(1024)
                        point = AggrPoint()
                        metric = AggrMetric()
                        point.decode(data[:sizeof(AggrPoint)])
                        metric.decode(data[sizeof(AggrPoint):])
                        aggregated_data.append((point.component_id, point.invoke_id, metric.to_dict()))
            except socket.timeout:
                print("Timeout occurred while requesting data.")
        return aggregated_data

def main():
    args = parse_arg()
    collector = Collector(args.host, args.port)
    collector.start_server()

if __name__ == '__main__':
    main()
