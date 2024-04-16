#ifndef __LIB_MAP_H_
#define __LIB_MAP_H_

#include "all.h"
#include "structs.h"


struct
{
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 25600 * 1024 /* 2560 KB */);
} rb SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_CGROUP_ARRAY);
    __type(key, u32);
    __type(value, u32);
    __uint(max_entries, 1);
} cgroup_map SEC(".maps");



// BPF_MAP_TYPE_LRU_HASH

struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, struct MsgContext);
    __uint(max_entries, 100);
} pid_context_map SEC(".maps");


struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, struct flow_tuple);
    __type(value, struct MsgContext);
    __uint(max_entries, 10000);
} tuple_context_map SEC(".maps");

// 保存某个trace中,某个pid的上一次recv和上一次send的invoke id;
// value位(recv invoke id) << 16 + send invoke id
struct
{
    __uint(type, BPF_MAP_TYPE_LRU_HASH); // LRU map的大小似乎有点问题,就是还没满的时候,老得表项会被莫名删除
    __type(key, struct TracePID);
    __type(value, struct invoke_point);
    __uint(max_entries, 10000);
} invoke_seq_map SEC(".maps");



struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, struct flow_tuple);
    __uint(max_entries, 10);
} client_ip_map SEC(".maps");



struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, struct data_args);
    __uint(max_entries, 1000);
} read_args_map SEC(".maps");


struct
{
    __uint(type, BPF_MAP_TYPE_LRU_HASH);
    __type(key, struct tgid_sport);
    __type(value, enum IO_state);  // 0: first read 1: not first read 2: first write 3:not first write
    __uint(max_entries, 20000);
} IO_state_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, __u32);
    __type(value, __u32);
    __uint(max_entries, 100);
} pid_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, __u32);
    __type(value, __u32);
    __uint(max_entries, 1);
} ingress_pid SEC(".maps"); //第一跳的组件的pid

struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, __u32);
    __type(value, __u64);
    __uint(max_entries, 1);
} now_traceID_map SEC(".maps"); //第一跳的组件的pid




#endif