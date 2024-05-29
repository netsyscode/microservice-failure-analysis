import socket
import threading

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
        print(f"Received edge: \
              {edge.trace_id} \
              {edge.component_id_1} \
              {edge.invoke_id_1} \
              {edge.component_id_2} \
              {edge.invoke_id2} \
              {edge.num}")
    conn.close()

def start_server(config: Config):
    host, port = config.controllers[0].ip, config.controllers[0].port    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()
        print(f'Server is listening on {host}:{port}')
        while True:
            conn, addr = s.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr))
            thread.start()

def main():
    gearbox_config = parse_args()
    start_server(gearbox_config)

if __name__ == "__main__":
    main()
