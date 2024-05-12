#ifndef __AGENT_STRUCTS_H__
#define __AGENT_STRUCTS_H__

#include <linux/types.h>

typedef enum { 
    NO_DEBUG, 
    MINIMAL_DEBUG, 
    FULL_DEBUG,
} DebugLevel;

typedef struct {
    int num_aggregators;
    char **aggregator_ips;
    int *aggregator_ports;

    int num_managers;
    char **manager_ips;
    int *manager_ports; 

    int *aggregator_fds;
    int *manager_fds;
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