#ifndef __GEARBOX_MAPS_H__
#define __GEARBOX_MAPS_H__

#include "all.h"
#include "structs.h"

/** 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * Maps for kernel information and communication 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 */

// Use per-flow 5 tuple key to store message context
// Mainly used by inter-component context propagation via TCP Option injection
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, struct flow_tuple);
    __type(value, struct msg_context);
    __uint(max_entries, 10000);
} flow_context_map SEC(".maps");

// Use per-flow 5 tuple key to store message context
// Mainly used by intra-component context propagation
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, struct msg_context);
    __uint(max_entries, 100);
} pid_context_map SEC(".maps");

// Use tgid key to store read system call arguments
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, struct data_args);
    __uint(max_entries, 1000);
} read_args_map SEC(".maps");

// Use tgid and sport key to store IO state (FIRST_RECV, NOT_FIRST_RECV, FIRST_SEND, NOT_FIRST_SEND, INFER_ERROR)
struct {
    __uint(type, BPF_MAP_TYPE_LRU_HASH);
    __type(key, struct tgid_sport);
    __type(value, enum IO_state);
    __uint(max_entries, 20000);
} IO_state_map SEC(".maps");

// Use trace_pid to store invoke states of the points from the same trace
struct {
    __uint(type, BPF_MAP_TYPE_LRU_HASH); // TODO: LRU map的大小似乎有点问题,就是还没满的时候,老得表项会被莫名删除
    __type(key, struct trace_pid);
    __type(value, struct invoke_state);
    __uint(max_entries, 10000);
} invoke_state_map SEC(".maps");

/** 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * Maps for filtering 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 */ 

// Filter the trigger of system call hooks by pid generated at user space
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, u32);
    __uint(max_entries, 100);
} pid_map SEC(".maps");

/** 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * Maps for output 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 */ 
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 25600 * 1024 /* 2560 KB */);
} rb SEC(".maps");

/** 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * Maps for hardcode implementation  
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 */ 

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, struct flow_tuple);
    __uint(max_entries, 10);
} client_ip_map SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, u64);
    __uint(max_entries, 1);
} now_trace_id_map SEC(".maps");

#endif // __GEARBOX_MAPS_H__