#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "agent_bpf.h"
#include "agent_defs.h"
#include "agent_util.h"
#include "agent_structs.h"
#include "kernel_structs.h"

unsigned int cur_pbuffer_sz = 0;
unsigned int cur_mbuffer_sz = 0;
unsigned int cur_ebuffer_sz = 0;

char point_buffer[POINT_BUFFER_SIZE];
char metric_buffer[METRIC_BUFFER_SIZE];
char edge_buffer[EDGE_BUFFER_SIZE];

int collect_rb(void *ctx, void *data, size_t data_sz) {
    enum msg_type type = *((enum msg_type *)data);

    switch (type) {
        case EDGE: {
            struct edge_msg *e_msg = (struct edge_msg *)data;
            struct edge_for_path e = {
                .trace_id = e_msg->e.p1.trace_id,
                .component_id_1 = e_msg->e.p1.component_id,
                .invoke_id_1 = e_msg->e.p1.invoke_id,
                .component_id_2 = e_msg->e.p2.component_id,
                .invoke_id_2 = e_msg->e.p2.invoke_id,
            };

            if (cur_ebuffer_sz + sizeof(struct edge_for_path) <= EDGE_BUFFER_SIZE) {
                memcpy(edge_buffer + cur_ebuffer_sz, &e, sizeof(struct edge_for_path));
                cur_ebuffer_sz += sizeof(struct edge_for_path);
            } else {
                fprintf(stderr, "Edge buffer full\n");
            }       
            break;
        }
        case POINT: {
            struct point_msg *p_msg = (struct point_msg *)data;
            struct point p = {
                .trace_id = p_msg->p.trace_id,
                .component_id = p_msg->p.component_id,
                .invoke_id = p_msg->p.invoke_id,
            };
            struct metrics m = {
                .srtt_us = p_msg->m.srtt_us,
                .mdev_max_us = p_msg->m.mdev_max_us,
                .rttvar_us = p_msg->m.rttvar_us,
                .mdev_us = p_msg->m.mdev_us,
                .bytes_sent = p_msg->m.bytes_sent,
                .bytes_received = p_msg->m.bytes_received,
                .bytes_acked = p_msg->m.bytes_acked,
                .delivered = p_msg->m.delivered,
                .snd_cwnd = p_msg->m.snd_cwnd,
                .rtt_us = p_msg->m.rtt_us,
                .duration = p_msg->m.duration,
            };

            if (cur_pbuffer_sz + sizeof(struct point) <= POINT_BUFFER_SIZE &&
                cur_mbuffer_sz + sizeof(struct metrics) <= METRIC_BUFFER_SIZE) {
                memcpy(point_buffer + cur_pbuffer_sz, &p, sizeof(struct point));
                cur_pbuffer_sz += sizeof(struct point);

                memcpy(metric_buffer + cur_mbuffer_sz, &m, sizeof(struct metrics));
                cur_mbuffer_sz += sizeof(struct metrics);
            } else {
                fprintf(stderr, "Point or metric buffer full\n");
            }            
            break;
        }
        default:
            fprintf(stderr, "Unknown message type: %d\n", type);
            break;
    }

    return 0;
}

int run(ConfigData *config, struct ring_buffer **rb) {
    int ret = 0;

    while (1) {
        ret = ring_buffer__poll(*rb, RING_BUFFER_POLL_TIMEOUT);
        if (ret) {
            if (ret == -EINTR) {
                fprintf(stdout, "Interrupted, exiting gearbox agent\n");
                ret = 0;
                break;
            } else {
                fprintf(stderr, "Error polling ring buffer: %s\n", strerror(errno));
                break;
            }
        }

        if (cur_pbuffer_sz > 0) {
            for (int i = 0; i < cur_pbuffer_sz / sizeof(struct point); i ++) {
                struct point *p = (struct point *)(point_buffer + i * sizeof(struct point));
                int id = p->trace_id % config->num_managers;

                ret = write(config->manager_fds[id], point_buffer + i * sizeof(struct point), sizeof(struct point));
                DEBUG("write point to manager %d\n", id);
                if (ret < 0) {
                    fprintf(stderr, "Failed to write point to manager %d: %s\n", id, strerror(errno));
                    return ret;
                }

                ret = write(config->manager_fds[id], metric_buffer + i * sizeof(struct metrics), sizeof(struct metrics));
                DEBUG("write metric to manager %d\n", id);
                if (ret < 0) {
                    fprintf(stderr, "Failed to write metric to manager %d: %s\n", id, strerror(errno));
                    return ret;
                }
            }

            cur_pbuffer_sz = 0;
            cur_mbuffer_sz = 0;
        }

        if (cur_ebuffer_sz > 0) {
            for (int i = 0; i < cur_ebuffer_sz / sizeof(struct edge_for_path); i ++) {
                struct edge_for_path *e = (struct edge_for_path *)(edge_buffer + i * sizeof(struct edge_for_path));
                int id = e->trace_id % config->num_collectors;

                ret = write(config->collector_fds[id], (void *)e, sizeof(struct edge_for_path));
                DEBUG("write edge to collector %d\n", id);
                if (ret < 0) {
                    fprintf(stderr, "Failed to write to collector %d: %s\n", id, strerror(errno));
                    return ret;
                }
            }

            cur_ebuffer_sz = 0;
        }
    }

    return ret;
}

int init(ConfigData *config, struct ring_buffer **rb) {
    // Attach cgroup map for tcp_int
    if (attach_to_cgroup(CGROUP_PATH, BPF_TCP_INT_PATH, BPF_TCP_INT_NEW_PATH) < 0) {
        fprintf(stderr, "Failed to attach to cgroup\n");
        return -1; // TODO: Change to error code
    }

    // Update pid_map for pid filtering
    if (read_pids_and_update_map(PID_CONFIG_PATH, PID_MAP_PATH) < 0) {
        fprintf(stderr, "Failed to update pid map\n");
        return -1; // TODO: Change to error code
    }

    // Open new ring buffer
    *rb = ring_buffer__new(bpf_obj_get(RING_BUFFER_PATH), collect_rb, NULL, NULL);
    if (!(*rb)) {
        fprintf(stderr, "Failed to open ring buffer\n");
        return -1; // TODO: Change to error code
    }

    // Parse config file to get manager and collector settings
    if (parse_config_file(config_path, config) < 0) {
        fprintf(stderr, "Failed to parse config file\n");
        return -1; // TODO: Change to error code
    }
    
    // Connect to manager
    for (int i = 0; i < config->num_managers; i++) {
        config->manager_fds[i] = open_client(config->manager_ips[i], config->manager_ports[i]);
        if (config->manager_fds[i] < 0) {
            fprintf(stderr, "Failed to connect to manager %d\n", i);
            return -1; // TODO: Change to error code
        }
    }

    // Connect to collectors
    for (int i = 0; i < config->num_collectors; i++) {
        config->collector_fds[i] = open_client(config->collector_ips[i], config->collector_ports[i]);
        if (config->collector_fds[i] < 0) {
            fprintf(stderr, "Failed to connect to collector %d\n", i);
            return -1; // TODO: Change to error code
        }
    }

    return 0;
}

void cleanup(ConfigData *config, struct ring_buffer **rb) {
    // For config
    if (config->num_managers > 0) {
        for (int i = 0; i < config->num_managers; i++) {
            close(config->manager_fds[i]);
            free(config->manager_ips[i]);
        }
        free(config->manager_ips);
        free(config->manager_ports);
        free(config->manager_fds);
    }

    if (config->num_collectors > 0) {
        for (int i = 0; i < config->num_collectors; i++) {
            close(config->collector_fds[i]);
            free(config->collector_ips[i]);
        }
        free(config->collector_ips);
        free(config->collector_ports);
        free(config->collector_fds);
    }

    // For ring buffer
    if (*rb) {
        ring_buffer__free(*rb);
    }
}

int main(int argc, char **argv) {
    struct ring_buffer *rb = NULL;
    ConfigData config = {0};
    int ret = 0;

    INFO("Gearbox agent started\n");
    parse_args(argc, argv);
    INFO("Parse args done\n");

    ret = init(&config, &rb);
    INFO("Init done\n");

    // Run the event processing loop
    if (ret == 0) {
        run(&config, &rb);
        INFO("Run done\n");
    } else {
        INFO("Failed to run\n");
    }

    cleanup(&config, &rb);
    INFO("Cleanup done\n");

    return 0;
}
