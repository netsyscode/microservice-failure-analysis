from ctypes import *

class AggrPoint(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        #(name, ctype)
        ('component_id', c_uint64),
        ('invoke_id', c_uint16),
    ]

    def encode(self):
        return string_at(addressof(self), sizeof(self))
    
    def decode(self,data):
        memmove(addressof(self), data, sizeof(self))
        return len(data)

class AggrMetric(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        #(name, ctype)
        ('srtt_us', c_uint32),
        ('mdev_max_us', c_uint32),
        ('rttvar_us', c_uint32),
        ('mdev_us', c_uint32),
        ('bytes_sent', c_uint64),
        ('bytes_received', c_uint64),
        ('bytes_acked', c_uint64),
        ('delivered', c_uint32),
        ('snd_cwnd', c_uint32),
        ('rtt_us', c_uint32),
        ('duration_45', c_uint64),
        ('duration_95', c_uint64),
    ]

    def encode(self):
        return string_at(addressof(self), sizeof(self))
    
    def decode(self,data):
        memmove(addressof(self), data, sizeof(self))
        return len(data)    
    
    def to_dic(self):
        return {
            'srtt_us': self.srtt_us,
            'mdev_max_us': self.mdev_max_us,
            'rttvar_us': self.rttvar_us,
            'mdev_us': self.mdev_us,
            'bytes_sent': self.bytes_sent,
            'bytes_received': self.bytes_received,
            'bytes_acked': self.bytes_acked,
            'delivered': self.delivered,
            'snd_cwnd': self.snd_cwnd,
            'rtt_us': self.rtt_us,
            'duration_45': self.duration_45,
            'duration_95': self.duration_95,
        } 