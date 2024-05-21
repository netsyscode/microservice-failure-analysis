import socket
import threading
from ctypes import *
import argparse



class Edge(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        #(name, ctype)
        ('traceID',c_uint64),
        ('componentID1',c_uint64),
        ('invokeID1',c_uint16),
        ('componentID2',c_uint64),
        ('invokeID2',c_uint16),
        ('num',c_uint16)
    ]

    def encode(self):
        return string_at(addressof(self),sizeof(self))
    
    def decode(self,data):
        memmove(addressof(self),data,sizeof(self))
        return len(data)

def handle_client(conn, addr):
    print('Connected by', addr)
    while True:
        data = conn.recv(sizeof(Edge))
        edge = Edge()
        edge.decode(data)
        if not data:
            break
        print(f"Received edge: {edge.traceID} {edge.componentID1} {edge.invokeID1} {edge.componentID2} {edge.invokeID2} {edge.num}")
    conn.close()

def start_server(host='10.0.1.132', port=18080):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()
        print(f'Server is listening on {host}:{port}')
        while True:
            conn, addr = s.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr))
            thread.start()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process some integers.")
    parser.add_argument("--ip", help="The IP address to connect to")
    parser.add_argument("--port", type=int, help="The port to connect to")

    args = parser.parse_args()

    start_server(args.ip, args.port)