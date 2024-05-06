import socket
import threading
import time
from queue import Queue
from ctypes import *
from data import AGG_Point, AGG_Metric
import json
import copy

lock = threading.Lock()

path_num = 0
transfer_size = 0
pathid_list = Queue(1000 * 4)
pathid_set = {}
pathnum_dic = {}
file_name = '../../evaluation/tprof/data/error10-0.5ms.json'
for x in range(1, 21):
    pathnum_dic[x] = 0

# agent_list = [("127.0.0.1",50012)] 
# agent_list = [("10.0.1.132",50012), ("10.0.1.130", 50012)]
agent_list = [("10.0.1.132",50012)]
storage_server_ip = "10.0.1.40"

def request(address,port,pathid):
    # print(pathid)
    global transfer_size
    global file_name
    global path_num
    server_address = (address, port)

    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.settimeout(3)
    has_send = False
    data = pathid.to_bytes(32, 'little')
    lenth = 0
    data_list = []
    while not has_send:
        udp_socket.sendto(data, server_address)
        transfer_size += len(data)
        try:
            data, server_address = udp_socket.recvfrom(1024)
            transfer_size += len(data)
        except:
            continue
        else:
            lenth = int.from_bytes(data,'little')
            has_send = True
            # print(lenth)
    if lenth > 0:
        for i in range(0,lenth):
            try:
                data, server_address = udp_socket.recvfrom(1024)
                transfer_size += len(data)
            except:
                continue
            else:
                data1 = data[0:sizeof(AGG_Point):]
                data2 = data[sizeof(AGG_Point):sizeof(AGG_Point)+sizeof(AGG_Metric)]
                point = AGG_Point()
                metric = AGG_Metric()
                point.decode(data1)
                metric.decode(data2)
                # print(point.componentID,point.invokeID,metric.srtt_us)
                #data_list.append((point.componentID,point.invokeID,metric.srtt_us))
                data_list.append((point.componentID,point.invokeID,metric.to_dic()))
                #udp_socket.sendto(b'ACKED', server_address)


    udp_socket.close()
    return data_list

def get_data():
    global pathid_list
    global pathid_set
    global path_num
    global file_name
    global transfer_size
    global pathnum_dic
    path_list_local = []
    data = []
    flag = 1
    last_query_time = time.time()
    query_interval = 10
    
    while True:
        # if path_num // 1000 > 0 and pathnum_dic[ path_num // 1000 ] != 1:
        if path_num // 5000 > 0:
            t = path_num
            path_num = 0
            with lock:
                path_list_local = list(pathid_set.keys())
                pathid_set_copy = copy.deepcopy(pathid_set)
                pathid_set.clear()
            print("query on",time.time())
            # data_save = {'time': time.time()}
            for pathID in path_list_local:
                data = []
                for (address,port) in agent_list:
                    data += request(address,port,pathID)
                save({"time": time.time(), "pathnum": pathid_set_copy[pathID], pathID:data })
                data = []

            print(f"pathnum: {t} transfer_size: {transfer_size} name: {file_name}")


