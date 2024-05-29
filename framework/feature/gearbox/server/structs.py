from ctypes import *

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
