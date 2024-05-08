from ctypes import *
import networkx as nx

class Point(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        #(name, ctype)
        ('trace_id', c_uint64),
        ('component_id', c_uint64),
        ('invoke_id', c_uint16),
    ]

    def __init__(self, trace_id, component_id, invoke_id) -> None:
        super().__init__()
        self.trace_id = trace_id
        self.component_id = component_id
        self.invoke_id = invoke_id

    def encode(self):
        return string_at(addressof(self), sizeof(self))
    
    def decode(self, data):
        memmove(addressof(self), data, sizeof(self))
        return len(data)

class Edge(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        #(name, ctype)
        ('trace_id', c_uint64),
        ('component_id_1', c_uint64),
        ('invoke_id_1', c_uint16),
        ('component_id_2', c_uint64),
        ('invoke_id_2', c_uint16),
        ('num', c_uint16),
    ]

    def encode(self):
        return string_at(addressof(self), sizeof(self))
    
    def decode(self, data):
        memmove(addressof(self), data, sizeof(self))
        return len(data)

class Path():
    def __init__(self):
        """Initialize the Path."""
        self.has_request = False
        self.has_response = False
        self.graph = nx.DiGraph()
        self.component_point = {}

    def __str__(self) -> str:
        """Return a string representation of the graph based on DFS preorder traversal of nodes."""
        return ''.join(str(node) for node in nx.dfs_preorder_nodes(self.graph))

    def add_edge(self, edge):
        """Add an edge to the graph, checking for special cases involving request and response."""
        if edge.component_id_1 == edge.component_id_2:
            raise ValueError("An edge cannot connect a component to itself.")

        # Handling request and response flags
        if edge.component_id_1 == 0:
            self.has_request = True
        if edge.component_id_2 == 0:
            self.has_response = True

        # Initialize component points list if not already present
        if edge.component_id_1 not in self.component_point:
            self.component_point[edge.component_id_1] = []
        if edge.component_id_2 not in self.component_point:
            self.component_point[edge.component_id_2] = []

        # Append invoke IDs to their respective components
        if edge.component_id_1 != 0:
            self.component_point[edge.component_id_1].append(int(edge.invoke_id_1))
        if edge.component_id_2 != 0:
            self.component_point[edge.component_id_2].append(int(edge.invoke_id_2))

        # Adding edges to the graph
        source = f"{edge.component_id_1}-{edge.invoke_id_1}"
        target = f"{edge.component_id_2}-{edge.invoke_id_2 if edge.component_id_2 != 0 else 1}"
        self.graph.add_edge(source, target)

    def complete(self):
        """Ensure each component has at least two interactions and create additional edges if needed."""
        if not (self.has_request and self.has_response):
            return False

        for component, points in self.component_point.items():
            if len(points) < 2:
                continue
            sorted_points = sorted(points)
            for i in range(len(sorted_points) - 1):
                self.graph.add_edge(f"{component}-{sorted_points[i]}", f"{component}-{sorted_points[i + 1]}")
        return True
    
    def calc_path_id(self):
        """Calculate a unique path identifier based on the MD5 hash of the graph's string representation."""
        hash_object = hashlib.md5()
        hash_object.update(str(self).encode('utf-8'))
        ID128 = hash_object.hexdigest()
        return int(ID128[:8], 16)
