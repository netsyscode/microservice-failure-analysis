import socket
import threading

from ctypes import sizeof

from util import parse_args
from structs import Edge
from config import Config

# Create a lock for thread-safe file writing
file_lock = threading.Lock()

def handle_client(conn, addr, log_file_path):
    print('Connected by', addr)
    while True:
        data = conn.recv(sizeof(Edge))
        edge = Edge()
        edge.decode(data)
        if not data:
            break
        with file_lock:
            with open(log_file_path, 'a') as log_file:
                log_file.write(f"{str(edge.trace_id).ljust(24)}"  
                               f"{str(edge.component_id_1).ljust(24)}"
                               f"{str(edge.invoke_id_1).ljust(24)}"
                               f"{str(edge.component_id_2).ljust(24)}"
                               f"{str(edge.invoke_id_2).ljust(24)}"
                               f"{str(edge.num).ljust(24)}\n")
    conn.close()

def start_server(config: Config, log_file_path="../output/raw_data.log"):
    host, port = config.controllers[0].ip, config.controllers[0].port    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()
        print(f'Server is listening on {host}:{port}')
        with file_lock:
            with open(log_file_path, 'a') as log_file:
                log_file.write(f"{str('Trace_ID').ljust(24)}"  
                               f"{str('Component_ID_1').ljust(24)}"
                               f"{str('Invoke_ID_1').ljust(24)}"
                               f"{str('Component_ID_2').ljust(24)}"
                               f"{str('Invoke_ID_2').ljust(24)}"
                               f"{str('Edge_Number').ljust(24)}\n")
        while True:
            conn, addr = s.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr, log_file_path))
            thread.start()

def main():
    gearbox_config = parse_args()
    print(str(gearbox_config))

    start_server(gearbox_config)

if __name__ == "__main__":
    main()
