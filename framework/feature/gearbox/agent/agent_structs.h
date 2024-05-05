#ifndef __AGENT_STRUCTS_H__
#define __AGENT_STRUCTS_H__

#include <linux/types.h>

typedef enum { 
    NO_DEBUG, 
    MINIMAL_DEBUG, 
    FULL_DEBUG,
} DebugLevel;

typedef struct {
    int num_managers;
    char **manager_ips;
    int *manager_ports;
    int *manager_fds;

    int num_collectors;
    char **collector_ips;
    int *collector_ports;
    int *collector_fds;
} ConfigData;

struct edge_for_path {
    __u64 trace_id;
    __u64 component_id_1;
    __u16 invoke_id_1;
    __u64 component_id_2;
    __u16 invoke_id_2;
    __u16 num;
} __attribute__((packed));

#endif // __AGENT_STRUCTS_H__