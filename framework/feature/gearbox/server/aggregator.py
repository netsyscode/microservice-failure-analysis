import socket
import threading
import time

from queue import Queue
from ctypes import sizeof

from config import Config
from structs import Point, Metric, AggrPoint, AggrMetric
from util import parse_args, do_aggregation, retrieve_aggregated_metric

QUEUE_ELEM_CNT = 2000
CLIENT_ADDR_SPACE = 16
PATH_ID_SZ = 4
SIGNAL_PROCESS_DONE_SZ = 4

class Aggregator:
    def __init__(self, config: Config, idx):
        self.config = config
        self.idx = idx
        self.point_metric_buffer = Queue(QUEUE_ELEM_CNT * (sizeof(Point) + sizeof(Metric)))
        self.pathid_point_buffer = Queue(QUEUE_ELEM_CNT * (PATH_ID_SZ + sizeof(Point)))
        self.query_list = Queue(QUEUE_ELEM_CNT * PATH_ID_SZ)
        self.process_done_signal = Queue(SIGNAL_PROCESS_DONE_SZ)
        self.aggr_buffer = Queue(QUEUE_ELEM_CNT * (sizeof(AggrPoint) + sizeof(AggrMetric)))

    def start_server(self):
        threading.Thread(target=self.start_loader_comm).start()
        threading.Thread(target=self.start_manager_comm).start()
        threading.Thread(target=self.start_collector_comm).start()
        threading.Thread(target=self.aggregation).start()

    def start_loader_comm(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as tcp_socket:
            tcp_socket.bind((self.config.aggregators[self.idx].ip, self.config.aggregators[self.idx].loader_port))
            tcp_socket.listen(5)

            while True:
                loader_socket, loader_address = tcp_socket.accept()
                print(f"New connection from {loader_address}")
                loader_thread = threading.Thread(target=self.handle_loader, args=(loader_socket))
                loader_thread.start()
    
    def handle_loader(self, loader_socket):
        while True:
            data = loader_socket.recv(sizeof(Point))
            assert(data)

            point = Point()
            point.decode(data)

            data = loader_socket.recv(sizeof(Metric))
            assert(data)

            metric = Metric()
            metric.decode(data)

            try:
                self.point_metric_buffer.put((point, metric))
            except:
                print("full")

            if data=="QUIT":
                break

        time.sleep(1)
        loader_socket.close()

    def start_manager_comm(self):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
            udp_socket.bind((self.config.aggregators[self.idx].ip, self.config.aggregators[self.idx].manager_port))

            while True:
                # Receive point from manager
                data, manager_address = udp_socket.recvfrom(1024)
                point = Point()
                point.decode(data)
                udp_socket.sendto(b'ACKED', manager_address)

                # Receive path_id from manager
                data, manager_address = udp_socket.recvfrom(1024)
                path_id = int.from_bytes(data,'little')
                udp_socket.sendto(b'ACKED', manager_address)

                # Put path_id and point in buffer queue
                self.pathid_point_buffer.put((path_id, point))

    def start_collector_comm(self):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
            udp_socket.bind((self.config.aggregators[self.idx].ip, self.config.aggregators[self.idx].collector_port))

            while True:
                data, collector_address = udp_socket.recvfrom(1024)
                assert(data)

                path_id = int.from_bytes(data,'little')
                print(f"Received path_id: {path_id}")

                self.query_list.put(path_id)

                while True:
                    try:
                        signal = self.process_done_signal.get(block=False)
                    except:
                        continue
                    else:
                        break

                send_data = []
                while True:
                    try:
                        aggregated_data = self.aggr_buffer.get(block=False)
                    except:
                        break
                    else:
                        send_data.append(aggregated_data)
                print("len", len(send_data))

                udp_socket.sendto(len(send_data).to_bytes(32,'little'), collector_address)
                for (p, m) in send_data:
                    udp_socket.sendto(p.encode() + m.encode(), collector_address)

    def aggregation(self):
        raw_metric_buffer, aggr_metric_buffer = {}, {}

        has_new_loader_data = False
        has_new_path_id = False
        has_storage_query = False

        while True:
            # Get new point and new metric from loader
            try:
                point_1, metric = self.point_metric_buffer.get(block=False)
            except:
                has_new_loader_data = False
            else:
                has_new_loader_data = True
                raw_metric_buffer[point_1.encode()] = metric

            # Get path_id and point from manager
            try:
                path_id, point_2 = self.pathid_point_buffer.get(block=False)
            except:
                has_new_path_id = False
            else:
                has_new_path_id = True
                if (point_2.component_id, point_2.invoke_id) != (0, 0) and point_2.encode() in raw_metric_buffer: 
                    if point_2.component_id not in aggr_metric_buffer:
                        aggr_metric_buffer[point_2.component_id] = {}
                    do_aggregation(aggr_metric_buffer[point_2.component_id], 
                                   point_2.invoke_id,
                                   path_id,
                                   raw_metric_buffer[point_2.encode()])
                    raw_metric_buffer.pop(point_2.encode())
            
            # Get metric query request from collector
            try:
                pathid = self.query_list.get(block=False)
            except:
                has_storage_query = False
            else:
                has_storage_query = True
                for _component_id in aggr_metric_buffer.keys():
                    if pathid not in aggr_metric_buffer[_component_id]:
                        continue
                    tmp_metric_dic = aggr_metric_buffer[_component_id].pop(pathid)
                    for _invoke_id, _metric in tmp_metric_dic.items():
                        agg_point = AggrPoint()
                        agg_point.component_id = _component_id
                        agg_point.invoke_id = _invoke_id
                        agg_metric = retrieve_aggregated_metric(_metric)
                        self.aggr_buffer.put((agg_point, agg_metric))
                self.process_done_signal.put(True)

            if not has_new_loader_data and not has_new_path_id and not has_storage_query:
                time.sleep(0.1)
                continue

def main():
    gearbox_config, idx = parse_args()
    aggregator = Aggregator(gearbox_config, idx)
    aggregator.start_server()

if __name__ == '__main__':
    main()
