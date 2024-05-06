import socket
import threading
from queue import Queue, Empty
from ctypes import sizeof
import time

from util import parse_args, send_udp_msg
from structs import Edge, Point, Path

QUEUE_ELEM_CNT = 2000
CLIENT_ADDR_SPACE = 16

class Manager:
    def __init__(self, host, port):
        """Initialize the server with host and port, and setup necessary data structures."""
        self.host = host
        self.port = port
        self.edge_queue = Queue(QUEUE_ELEM_CNT * (sizeof(Edge) + CLIENT_ADDR_SPACE))
        self.point_dic = {}
        self.path_dic = {}

    def start_server(self):
        """Bind the server socket, listen and start the server."""
        # Initialize server socket
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
            server_socket.bind((self.host, self.port))
            server_socket.listen(5)
            print(f"Server listening on {self.host}:{self.port}")

            # Start a thread to process edges
            threading.Thread(target=self.process_edges).start()

            # Accept incoming client connections and start a new thread for each client
            try:
                while True:
                    client_socket, client_address = server_socket.accept()
                    print(f"Accepted connection from {client_address}")
                    threading.Thread(target=self.client_handler, args=(client_socket, client_address)).start()
            except KeyboardInterrupt:
                print("Server shutting down.")
    
    def client_handler(self, client_socket, client_address):
        """Handle the connection from a client, receiving data and processing it."""
        try:
            while True:
                # Receive data from the client
                data = client_socket.recv(sizeof(Edge))
                if not data:
                    continue

                # Check for quit command
                if data == b"QUIT":
                    break

                # Decode data to Edge object
                edge = Edge()
                edge.decode(data)

                # Attempt to store the received edge in the edge list
                try:
                    self.edge_queue.put((edge, client_address))
                except queue.Full:
                    print("Edge list is full, unable to add more edges.")
        except Exception as e:
            print(f"An error occurred: {e}")
        finally:
            client_socket.close()
            print(f"Connection closed for {client_address}")

    def process_edges(self):
        """Continuously get edges from the queue and process them."""
        while True:
            try:
                edge, client_address = self.edge_queue.get(block=False)
                time.sleep(1)  # Sleep when queue is empty
            except Empty:
                continue

        self.process_edge(edge, client_address)
        self.check_and_process_complete_paths()

    def process_edge(self, edge, client_address):
        """Add edge to point dictionary and path dictionary."""
        trace_id = edge.trace_id

        if trace_id not in self.point_dic:
            self.point_dic[trace_id] = []
        self.point_dic[trace_id].extend([
            (edge.component_id_1, edge.invoke_id_1, client_address),
            (edge.component_id_2, edge.invoke_id_2, client_address)
        ])

        if trace_id not in self.path_dic:
            self.path_dic[trace_id] = Path()
        self.path_dic[trace_id].addEdge(edge)

    def check_and_process_complete_paths(self):
        """Check if paths are complete and process them if they are, then remove them."""
        completed_trace_ids = []  # List to store trace IDs of completed paths

        # Check and process each path, record trace IDs of completed paths
        for trace_id, path in self.path_dic.items():
            if path.complete():
                self.process_complete_path(trace_id)
                completed_trace_ids.append(trace_id)

        # Remove processed paths from the dictionary using exception handling
        for trace_id in completed_trace_ids:
            try:
                del self.path_dic[trace_id]
            except KeyError:
                print(f"Failed to delete path with trace ID {trace_id}: Not found in path_dic")

    def process_complete_path(self, trace_id):
        """Handle complete paths by aggregating data and sending it to aggregation and storage."""
        try:
            path = self.path_dic[trace_id]
            point_list = self.point_dic.pop(trace_id)
        except KeyError as e:
            print(f"Key error encountered: {e}")
            return

        path_id = path.calc_path_id()  # Returns a unique ID for a path

        for component_id, invoke_id, aggr_address in point_list:
            if component_id == 0:
                continue

            point = Point(trace_id, component_id, invoke_id)
            self.send_to_aggregation(point, path_id, aggr_address, AGGREGATION_PORT)

        self.send_to_storage(path_id, STORAGE_ADDR, STORAGE_PORT)

    def send_pathid_to_aggregation(self, point, path_id, target_addr, target_port):
        """Creates a UDP socket, sends point and path_id to target aggregation server."""
        aggr_server = (target_addr, target_port)
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
            udp_socket.settimeout(2)  # Set timeout for the socket

            # Prepare data
            point_encoded = point.encode()
            path_id_encoded = path_id.to_bytes(32, 'little')

            # Send data
            send_udp_msg(udp_socket, point_encoded, aggr_server)
            send_udp_msg(udp_socket, path_id_encoded, aggr_server)

    def send_pathid_to_storage(self, path_id, target_addr, target_port):
        """Creates a UDP socket, sends path_id to target storage server."""
        storage_server = (target_addr, target_port)
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
            udp_socket.settimeout(2)  # Set timeout for the socket

            # Prepare data
            path_id_encoded = path_id.to_bytes(32, 'little')

            # Send data
            send_udp_msg(udp_socket, path_id_encoded, storage_server)

def main():
    args = parse_args()
    manager = Manager(args.host, args.port)
    manager.start_server()

if __name__ == "__main__":
    STORAGE_ADDR = "NOT_SET"
    STORAGE_PORT = 50020

    AGGREGATION_PORT = 50011

    main()
