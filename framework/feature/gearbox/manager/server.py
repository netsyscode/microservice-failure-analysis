from queue import Queue

from util import *
from structs import Edge, Point, Path
from config import *


def client_handler(client_socket, client_address, output_queue):
    """Handle the connection from a client, receiving data and processing it."""
    print(f"New connection from {client_address}")

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
                output_queue.put((edge, client_address))
            except queue.Full:
                print("Edge list is full, unable to add more edges.")

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        # Ensure the client socket is closed after operations
        client_socket.close()
        print(f"Connection closed for {client_address}")

def process_edge(edge, client_address, point_dic, path_dic):
    """Add edge to point dictionary and path dictionary."""
    trace_id = edge.trace_id

    if trace_id not in point_dic:
        point_dic[trace_id] = []
    point_dic[trace_id].extend([
        (edge.component_id_1, edge.invoke_id_1, client_address),
        (edge.component_id_2, edge.invoke_id_2, client_address)
    ])

    if trace_id not in path_dic:
        path_dic[trace_id] = Path()
    path_dic[trace_id].addEdge(edge)

def check_and_process_complete_paths(point_dic, path_dic):
    """Check if paths are complete and process them if they are, then remove them."""
    completed_trace_ids = []  # List to store trace IDs of completed paths

    # Check and process each path, record trace IDs of completed paths
    for trace_id, path in path_dic.items():
        if path.complete():
            process_complete_path(trace_id, path_dic, point_dic)
            completed_trace_ids.append(trace_id)

    # Remove processed paths from the dictionary using exception handling
    for trace_id in completed_trace_ids:
        try:
            del path_dic[trace_id]
        except KeyError:
            print(f"Failed to delete path with trace ID {trace_id}: Not found in path_dic")

def process_complete_path(trace_id, path_dic, point_dic):
    """Handle complete paths by aggregating data and sending it to aggregation and storage."""
    try:
        path = path_dic[trace_id]
        point_list = point_dic.pop(trace_id)
    except KeyError as e:
        print(f"Key error encountered: {e}")
        return

    path_id = path.calc_path_id()  # Returns a unique ID for a path

    for component_id, invoke_id, aggr_address in point_list:
        if component_id == 0:
            continue

        point = Point(trace_id, component_id, invoke_id)
        send_pathid_to_aggregation(point, path_id, aggr_address, AGGREGATION_PORT)
    
    send_pathid_to_storage(path_id, STORAGE_ADDR, STORAGE_PORT)

def edge_consumer(input_queue, point_dic, path_dic):
    """Process edges from an input queue and manage paths and points dictionaries."""
    while True:
        try:
            edge, client_address = input_queue.get(block=False)
        except Empty:
            time.sleep(1)  # Sleep when queue is empty
            continue

        process_edge(edge, client_address, point_dic, path_dic)
        check_and_process_complete_paths(point_dic, path_dic)

def main():
    """Main function to setup and start the path assembly server (gearbox manager)."""
    # Buffers for storing points and paths
    point_buffer = {}
    path_buffer = {}

    # Parse command line arguments
    args = parse_args()

    # Queue for incoming edges with a calculated size based on Edge structure and client address space
    edge_queue = Queue(QUEUE_ELEM_CNT * (sizeof(Edge) + CLIENT_ADDR_SPACE))

    # Start the server with the specified settings and handlers
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(5)

    server_thread = threading.Thread(target=edge_consumer, args=(edge_queue, point_buffer, path_buffer))
    server_thread.start()

    while True:
        client_socket, client_address = server_socket.accept()
        client_thread = threading.Thread(target=client_func, args=(client_socket, client_address, edge_queue))
        client_thread.start()

if __name__ == "__main__":
    main()
