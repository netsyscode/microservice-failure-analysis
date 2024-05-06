import socket
import threading
import time
from queue import Queue
from ctypes import *
from path import *
import sys
import numpy as np


lock = threading.Lock()

edge_list = Queue(2000*(sizeof(Edge)+16))
agent_port = 50011
storage_address = "10.0.1.40"
storage_port = 50020
path_server_ip = "10.0.1.40" #path_server 的ip地址

transfer_size = 0
edge_num = 0
path_num = 0
path_set = set()

path_time = {}
time_list = []
pathnum_dic = {}
for x in range(1, 12):
    pathnum_dic[x] = 0

def handle_client(client_socket,client_address):
    fp = open("egde.txt", 'a+')
    # 处理客户端连接请求
    global edge_list
    global edge_num
    global transfer_size
    global path_num
    global pathnum_dic
    print('new connection from %s:%s' % client_address)
    while True:

        data = client_socket.recv(sizeof(Edge))
        if not data:
            continue
        transfer_size += len(data)
        edge = Edge()
        edge.decode(data)

        #print(edge.num)
        # fp.write(f"{edge.traceID} {edge.componentID1} {edge.invokeID1} {edge.componentID2} {edge.invokeID2}\n")
      
        try:
            edge_list.put((edge,client_address))
        except:
            print("full")

        edge_num += 1
        if edge_num % 10 == 0:
            pass
            # print(f"edge num: {edge_num} path_num: {path_num}")
        if data=="QUIT":
            break
        #client_socket.send("ACKED".encode("utf-8"))
    time.sleep(1)
    client_socket.close()


# all for send pathID back
def send_msg(udp_socket,data,server_address):
    global transfer_size
    has_send = False
    while not has_send:
        
        udp_socket.sendto(data, server_address)
        transfer_size += len(data)
        # print(f"transfer_size: {transfer_size}")
        
        try:
            data, server_address = udp_socket.recvfrom(1024)
        except:
            continue
        #print(data)
        has_send = True

def send_back(point,address,pathID):
    server_address = (address, agent_port)

    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.settimeout(2)
    
    send_msg(udp_socket,point.encode(),server_address)
    send_msg(udp_socket,pathID.to_bytes(32, 'little'),server_address)
    udp_socket.close()

def send_pathID(pathID):
    server_address = (storage_address, storage_port)

    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.settimeout(2)
    # print("PID:",pathID)
    send_msg(udp_socket,pathID.to_bytes(32, 'little'),server_address)
    udp_socket.close()

    
def edge_consumer():
    global edge_list
    path_dic = {}
    # {traceid: [(componentID,invokeID,address)]}, for send back
    point_dic = {}
    global path_num
    global transfer_size
    fp = open(f"../../evaluation/tprof/data/path10-0.5ms.txt", 'w')
    while True:
        try:
            (edge,client_address) = edge_list.get(block=False)
        except:
            time.sleep(1)
        else:
            if edge.traceID not in point_dic:
                point_dic[edge.traceID] = []
            point_dic[edge.traceID].append((edge.componentID1,edge.invokeID1,client_address))
            point_dic[edge.traceID].append((edge.componentID2,edge.invokeID2,client_address))
            if edge.traceID not in path_dic:
                path_dic[edge.traceID] = Path(mode = 1)
            path_dic[edge.traceID].addEdge(edge)

            path_dic_copy = path_dic.copy()
            for traceid in path_dic_copy:
                if path_dic_copy[traceid].complete():
                    path_id = getPathID(path_dic[traceid].__str__())
                    if path_id not in path_set:
                        if path_dic[traceid].mode == 1:
                            fp.write(f"pathid: {path_id} edges: {path_dic[traceid].Graph.edges()}\n")
                    del path_dic[traceid]
                    path_set.add(path_id)
                    with lock:
                        path_num += 1
                        # if path_num // 1000 > 0 and pathnum_dic[ path_num // 1000 ] != 1:
                        if path_num // 500 > 0:
                            print(f"pathnum: {path_num} transfer_size: {transfer_size}")
                            # pathnum_dic[ path_num // 500 ] += 1
                            path_num = 0

                    point_list = point_dic.pop(traceid)
                    for (componentID,invokeID,address) in point_list:
                        if componentID == 0:
                            continue
                        point = Point()
                        point.traceID = traceid
                        point.componentID = componentID
                        point.invokeID = invokeID
                        # print(point,address[0],path_id)
                        send_back(point,address[0],path_id)
                    send_pathID(path_id)
            path_dic_copy.clear()


def open_server(host,port):
    # 创建TCP套接字
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 绑定到本地端口
    server_socket.bind((host, port))
    # 监听连接请求
    server_socket.listen(5)

    server_thread = threading.Thread(target=edge_consumer,args=())
    server_thread.start()

    while True:
        # 等待客户端连接
        client_socket, client_address = server_socket.accept()
        # 开启新的线程处理该连接
        client_thread = threading.Thread(target=handle_client, args=(client_socket,client_address))
        client_thread.start()
            
        
if __name__ == '__main__':
    open_server(path_server_ip, int(sys.argv[1]))