

#include "all.h"
// #include "structs.h"

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, u32);
    __uint(max_entries, 100);
} pid_map SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, struct data_args);
    __uint(max_entries, 1000);
} read_args_map SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, struct tag_list);
    __uint(max_entries, 1000);
} pid_tag_map SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 25600 * 1024 /* 2560 KB */);
} rb SEC(".maps");



