import socket
import threading
import textwrap

from ctypes import sizeof

from util import parse_args
from structs import Edge
from config import Config

def handle_client(conn, addr):
    print('Connected by', addr)
    while True:
        data = conn.recv(sizeof(Edge))
        edge = Edge()
        edge.decode(data)
        if not data:
            break
        print("Received edge:\t"
            f"{str(edge.trace_id).ljust(24)}"  
            f"{str(edge.component_id_1).ljust(24)}"
            f"{str(edge.invoke_id_1).ljust(24)}"
            f"{str(edge.component_id_2).ljust(24)}"
            f"{str(edge.invoke_id_2).ljust(24)}"
            f"{str(edge.num).ljust(24)}")
    conn.close()

def start_server(config: Config):
    host, port = config.controllers[0].ip, config.controllers[0].port    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()
        print(f'Server is listening on {host}:{port}')
        print("Received edge:\t"
            f"{str('Trace ID').ljust(24)}"  
            f"{str('Component ID 1').ljust(24)}"
            f"{str('Invoke ID 1').ljust(24)}"
            f"{str('Component ID 2').ljust(24)}"
            f"{str('Invoke ID 2').ljust(24)}"
            f"{str('Edge number').ljust(24)}")
        while True:
            conn, addr = s.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr))
            thread.start()

def main():
    gearbox_config = parse_args()
    print(str(gearbox_config))

    start_server(gearbox_config)

if __name__ == "__main__":
    main()
