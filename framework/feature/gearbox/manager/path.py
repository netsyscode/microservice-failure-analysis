from ctypes import *
import hashlib
from sortedcontainers import SortedList
import networkx as nx
import graphviz as gv
import time

service_name = {}


class Point(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        #(name, ctype)
        ('traceID',c_uint64),
        ('componentID',c_uint64),
        ('invokeID',c_uint16),
    ]

    def encode(self):
        return string_at(addressof(self),sizeof(self))
    
    def decode(self,data):
        memmove(addressof(self),data,sizeof(self))
        return len(data)

class Edge(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        #(name, ctype)
        ('traceID',c_uint64),
        ('componentID1',c_uint64),
        ('invokeID1',c_uint16),
        ('componentID2',c_uint64),
        ('invokeID2',c_uint16),
        ('num',c_uint16),
    ]

    def encode(self):
        return string_at(addressof(self),sizeof(self))
    
    def decode(self,data):
        memmove(addressof(self),data,sizeof(self))
        return len(data)

class PathNode():
    def __init__(self,componentID,invokeID) -> None:
        self.componentID = componentID
        self.invokeID = invokeID
        self.childList = SortedList()
    def __str__(self) -> str:
        return "{}:{}".format(self.componentID,self.invokeID)
    def __eq__(self, other) -> bool:
        return self.componentID == other.componentID and self.invokeID == other.invokeID
    def __le__(self,other):
        if self.componentID < other.componentID:
            return True
        if self.componentID == other.componentID and self.invokeID <= other.invokeID:
            return True
        return False
    def __gt__(self,other):
        if self.componentID > other.componentID:
            return True
        if self.componentID == other.componentID and self.invokeID > other.invokeID:
            return True
        return False
    def setChild(self,node):
        self.childList.add(node)

class PathNode():
    def __init__(self) -> None:
        self.componentID = None
        self.invokeID = None
    def setChild(self,componentID,invokeID):
        self.componentID = componentID
        self.invokeID = invokeID
    def __str__(self) -> str:
        if self.componentID == None or self.invokeID == None:
            return ""
        # return str((self.componentID,self.invokeID))
        return str(self.componentID)

class Path():
    # remember that invokeID is item in componentDic[componentID] plus 1
    def __init__(self, mode) -> None:
        self.componentDic = {}
        self.componentDic[0] = []
        self.totalNum = 0
        self.maxNum = 0
        self.request = False
        self.response = False
        self.edge_list = {}
        self.Graph = nx.DiGraph()
        self.gvGraph = gv.Digraph()
        self.component_point = {}
        self.point_list = []
        self.last_update = time.time()
        self.mode = mode


    
    def addPoint1(self,componentID1,invokeID1,componentID2,invokeID2):
        if componentID1 == 0:
            p = PathNode()
            p.setChild(componentID2,invokeID2)
            self.componentDic[0].append(p)
            self.request = True
            return
        if componentID1 not in self.componentDic:
            self.componentDic[componentID1] = []

        lenth = len(self.componentDic[componentID1])
        for _ in range(lenth,invokeID1):
            self.componentDic[componentID1].append(PathNode())
        self.componentDic[componentID1][invokeID1 - 1].setChild(componentID2,invokeID2)

    def addPoint2(self,componentID,invokeID):
        if componentID == 0:
            self.response = True
            return
        if componentID not in self.componentDic:
            self.componentDic[componentID] = []
        lenth = len(self.componentDic[componentID])
        for _ in range(lenth,invokeID):
            self.componentDic[componentID].append(PathNode())

    def addEdge(self,edge):
        self.last_update = time.time()
        global service_name
        if self.mode == 1:
            if edge.componentID1 not in self.component_point:
                self.component_point[edge.componentID1] = []
            if edge.componentID2 not in self.component_point:
                self.component_point[edge.componentID2] = []
            if int(edge.componentID2) != 0:
                self.component_point[edge.componentID2].append(int(edge.invokeID2))
            if int(edge.componentID1) != 0:
                self.component_point[edge.componentID1].append(int(edge.invokeID1))
            if edge.componentID1 != edge.componentID2:
                if int(edge.componentID2) == 0:
                    self.Graph.add_edge(f"{str(edge.componentID1)}-{edge.invokeID1}", f"{str(edge.componentID2)}-{1}")
                else:
                    self.Graph.add_edge(f"{str(edge.componentID1)}-{edge.invokeID1}", f"{str(edge.componentID2)}-{edge.invokeID2}")
    
        if self.mode == 0:
            if edge.componentID1 not in self.component_point:
                self.component_point[edge.componentID1] = []
            if edge.componentID2 not in self.component_point:
                self.component_point[edge.componentID2] = []
            if int(edge.componentID2) != 0:
                self.component_point[edge.componentID2].append(int(edge.invokeID2))
            if int(edge.componentID1) != 0:
                self.component_point[edge.componentID1].append(int(edge.invokeID1))
            if edge.componentID1 != edge.componentID2:
                if int(edge.componentID2) == 0:
                    self.gvGraph.edge(f"{service_name[edge.componentID1]}-{edge.invokeID1}", f"{service_name[edge.componentID2]}-{1}")
                else:
                    self.gvGraph.edge(f"{service_name[edge.componentID1]}-{edge.invokeID1}", f"{service_name[edge.componentID2]}-{edge.invokeID2}")

        # TraceGraph
        # if edge.componentID1 != edge.componentID2:
        #     if int(edge.componentID1) not in self.edge_list:
        #         self.edge_list[int(edge.componentID1)] = SortedList()
        #     if int(edge.componentID2) not in self.edge_list[int(edge.componentID1)]:
        #         self.edge_list[int(edge.componentID1)].add(int(edge.componentID2))
        #     if int(edge.componentID2) not in self.edge_list:
        #         self.edge_list[int(edge.componentID2)] = SortedList()
        #     if int(edge.componentID1) not in self.edge_list[int(edge.componentID2)]:
        #         self.edge_list[int(edge.componentID2)].add(int(edge.componentID1))



        self.addPoint1(edge.componentID1,edge.invokeID1,edge.componentID2,edge.invokeID2)
        self.addPoint2(edge.componentID2,edge.invokeID2)
        self.totalNum += 1
        self.maxNum = max(self.maxNum,edge.num)

    def complete(self):
        if not self.request or not self.response:
            return False
        # if time.time() - self.last_update < 0.2:
        #     return False
        # if self.totalNum != self.maxNum:
        #     return False

        if self.mode == 0:
            for com in self.component_point:
                if len(self.component_point[com]) < 2:
                    continue
                sorted_point =sorted(self.component_point[com])
                for i in range(len(self.component_point[com]) - 1):
                    # self.Graph.add_edge(f"{str(com)}-{sorted_point[i]}", f"{str(com)}-{sorted_point[i + 1]}")
                    # print(f"{str(com)}-{sorted_point[i]}", f"{str(com)}-{sorted_point[i + 1]}")
                    self.gvGraph.edge(f"{service_name[com]}-{sorted_point[i]}", f"{service_name[com]}-{sorted_point[i + 1]}")
        if self.mode == 1:
            for com in self.component_point:
                if len(self.component_point[com]) < 2:
                    continue
                sorted_point =sorted(self.component_point[com])
                for i in range(len(self.component_point[com]) - 1):
                    self.Graph.add_edge(f"{str(com)}-{sorted_point[i]}", f"{str(com)}-{sorted_point[i + 1]}")
                    # print(f"{str(com)}-{sorted_point[i]}", f"{str(com)}-{sorted_point[i + 1]}")
                    # self.gvGraph.edge(f"{service_name[com]}-{sorted_point[i]}", f"{service_name[com]}-{sorted_point[i + 1]}")
        return True
    
    def __str__(self) -> str:
        # if not self.complete():
        #     return None
        # print(self.edge_list)
        str1 = ''
        for node in nx.dfs_preorder_nodes(self.Graph):
            str1 += str(node)
        return str1
        # str1 = ''
        # for cID, pointList in sorted(self.edge_list.items()):
        #     # print(cID, pointList)
        #     str1 += str(cID)
        #     for p in pointList:
        #         str1 += str(p)
        # return str1

        # s_dic = {}
        # for cID, pointList in sorted(self.componentDic.items()):
        #     s_dic[cID] = []
        #     for p in pointList:
        #         if p.__str__() == '':
        #             continue
        #         if p.__str__() == str(cID):
        #         # print(p.__str__(), str(cID))
        #             continue
        #         s_dic[cID].append(p.__str__())
        # s_dic2 = {}
        # for key in s_dic:
        #     if len(s_dic[key]) != 0:
        #         s_dic2[key] = s_dic[key]
        # return str(s_dic2)
    
def getPathID(path_str):
    hash_object = hashlib.md5()
    bytes_list = bytes(path_str,'utf-8')
    hash_object.update(bytes_list)
    ID128 = hash_object.hexdigest()
    ID32 = ID128[0:8:]
    return int(ID32,16)

if __name__ == '__main__': 
    # import re
    # path = Path()
    # fp = open("test2.txt", 'r')
    # now_tid = ''
    # path_num = 0
    # pathid_set = set()
    # edge_num = 0
    # edge_num_dic = {}
    # for text in fp.readlines():
    #     cid = re.findall(r"componentID:(\d+)", text)
    #     inkid = re.findall(r"invokeID:(\d+)", text)
    #     try:
    #         tid = re.findall(r"traceid:(\d+)", text)[0]
    #     except:
    #         print(text)
    #     edge = Edge()
    #     if cid[0] == '0':
    #         edge_num = 0
    #         path_num += 1
    #         path = Path()
    #         if path_num == 7:
    #         #     plt.figure(figsize=(15, 15))
    #         #     pos = nx.fruchterman_reingold_layout(path.Graph,weight=None,scale=1.0)
    #         #     nx.draw_networkx_nodes(path.Graph, pos, node_size=40)
    #         #     nx.draw_networkx_edges(path.Graph, pos)
    #         #     nx.draw_networkx_labels(path.Graph, pos)
    #         #     # ax.set_axis_off()
    #         #     plt.show()

    #         #     plt.savefig("1.png")
                
    #             break
    #         now_tid = tid
    #     if tid != now_tid:
    #         continue
    #     edge.traceID = int(tid)
    #     edge.componentID1 = int(cid[0])
    #     edge.invokeID1 = int(inkid[0]) 
    #     edge.componentID2 = int(cid[1])
    #     edge.invokeID2 = int(inkid[1])  
    #     edge_num += 1
    #     path.addEdge(edge)   
    #     if path.complete() and edge_num > 10:
    #         if path_num == 1:
    #             path.gvGraph.render('graph', format='png')
    #         if edge_num not in edge_num_dic:
    #             edge_num_dic[edge_num] = 1
    #         else:
    #             edge_num_dic[edge_num] += 1
    #         # print(path.__str__())
    #         # print(path.edge_list)
    #         pid = getPathID(path.__str__())
    #         # print(pid, edge_num)
    #         # if pid not in pathid_set:
    #         #     print(path.edge_list)
    #         pathid_set.add(pid)     
    # print(path_num, len(pathid_set))

    import re
    # global service_name
    fp2 = open("service.txt", 'r')
    service_name[0] = '0'
    for line in fp2.readlines():
        name = re.search(r'socialNetwork_base_([^\.]+)\.', line).group(1)
        id = re.search(r", ID: (\d+)", line).group(1)
        service_name[int(id)] = name
    # print(service_name, len(service_name))
    fp = open("edge.txt", 'r')
    path_dic = {}
    path_num = 0
    path_num2 = 0
    pathid_set = set()
    edge_num = 0
    edge_num_dic = {}
    path_delete = []
    first_pid = ''
    for line in fp.readlines():
        line_ = line.strip().split()
        tid = int(line_[0])
        if tid not in path_dic:
            path_dic[tid] = Path(mode = 1)
            path_num2 += 1
        edge = Edge()
        edge.traceID = int(line_[0])
        edge.componentID1 = int(line_[1])
        edge.invokeID1 = int(line_[2])
        edge.componentID2 = int(line_[3])
        edge.invokeID2 = int(line_[4])
        path_dic[tid].addEdge(edge)
        path_dic_copy = path_dic.copy()
        for traceid in path_dic_copy:
            if path_dic_copy[traceid].complete():
                pathid = getPathID(path_dic[traceid].__str__())
                path_num += 1
                pathid_set.add(pathid)
                path_delete.append(traceid)
                del path_dic[traceid]
        path_dic_copy.clear()



    # for traceid in path_dic:
    #     path_dic[traceid].complete()
    #     path_num += 1
    #     pathid = getPathID(path_dic[traceid].__str__())
    #     if pathid not in pathid_set and path_num < 50:
    #         path_dic[traceid].gvGraph.render(f'./data/{pathid}', format='png')
    #     pathid_set.add(pathid)

    

    print(path_num, len(pathid_set), len(path_dic), path_num2)
    # print(edge_num_dic)

    # test_list = [[0, 0, 1, 1, 1],[1, 2, 2, 1, 2],[2, 2, 3, 1, 3],[3, 2, 2, 3, 4],[3, 4, 2, 5, 6],[2, 4, 3, 3, 5],[2, 6, 1, 3, 7],[1, 4, 0, 0, 8]]
    # path = Path()
    # # test_list = [[2,2,1,4,3],[0,0,1,1,1],[1,2,2,1,2],[1,3,3,1,1],[3,3,1,5,2],[1,6,0,0,6]]
    # for line in test_list:
    #     edge = Edge()
    #     edge.traceID = 0
    #     edge.componentID1 = line[0]
    #     edge.invokeID1 = line[1]   
    #     edge.componentID2 = line[2]
    #     edge.invokeID2 = line[3]               
    #     edge.num = line[4]
    #     path.addEdge(edge)
    #     if path.complete():
    #         plt.figure(figsize=(15, 15))
    #         pos = nx.fruchterman_reingold_layout(path.Graph,weight=None,scale=1.0)
    #         nx.draw_networkx_nodes(path.Graph, pos, node_size=40)
    #         nx.draw_networkx_edges(path.Graph, pos)
    #         nx.draw_networkx_labels(path.Graph, pos)
    #         # ax.set_axis_off()
    #         plt.show()

    #         plt.savefig("2.png")
    #         print(path)
    #         print(getPathID(path.__str__()))