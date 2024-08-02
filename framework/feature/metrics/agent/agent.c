#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <fcntl.h>
#include "agent_util.h"
#include "kernel_structs.h"

#define RING_BUFFER_POLL_TIMEOUT    100 
#define DEBUG_FS "/sys/kernel/debug/tracing/trace_pipe"
#define PID_MAP_PATH     "/sys/fs/bpf/metrics/pid_map"
#define PID_TAGS_MAP_PATH     "/sys/fs/bpf/metrics/pid_tag_map"
#define RING_BUFFER_PATH            "/sys/fs/bpf/metrics/rb"
char *pid_filter_file = NULL;
char *tags_config_file = NULL;


void read_trace_pipe(void) {
    int trace_fd;
    trace_fd = open(DEBUG_FS, O_RDONLY, 0);
    if (trace_fd < 0) {
        printf("Failed to open %s\n", DEBUG_FS);  // DEBUGFS is a macro defined in debug.h
        return;
    }

    while (1) {
        static char buf[4096];
        ssize_t sz;

        sz = read(trace_fd, buf, sizeof(buf) - 1);
        if (sz > 0) {
            buf[sz] = 0;
            puts(buf);
        }
    }

    close(trace_fd);
}


int collect_rb(void *ctx, void *data, size_t data_sz) {

    struct point *p = (struct point *)data;
    printf("Tag List:\n");
    printf("  HostIP: %u\n", p->tags.HostIP);
    printf("  ServiceID: %u\n", p->tags.ServiceID);
    printf("  ContainerID: %u\n", p->tags.ContainerID);
    printf("  ProcessID: %u\n", p->tags.ProcessID);
    printf("  ComponentID: %u\n", p->tags.ComponentID);
    printf("  k8sLabel: %u\n", p->tags.k8sLabel);
    printf("  ServiceType: %u\n", p->tags.ServiceType);
    printf("  EnvironmentType: %u\n", p->tags.EnvironmentType);
    printf("  ReleaseType: %u\n", p->tags.ReleaseType);
    printf("  Versions: %u\n", p->tags.Versions);
    printf("  PathID: %u\n", p->tags.PathID);
    printf("  SrcIP: %u\n", p->tags.SrcIP);
    printf("  DstIP: %u\n", p->tags.DstIP);
    printf("  SrcPort: %u\n", p->tags.SrcPort);
    printf("  DstPort: %u\n", p->tags.DstPort);
    printf("  Protocol: %u\n", p->tags.Protocol);
    printf("  CliendID: %u\n", p->tags.CliendID);
    printf("  OperationName: %u\n", p->tags.OperationName);
    printf("  API: %u\n", p->tags.API);
    printf("  ObservationPoints: %u\n", p->tags.ObservationPoints);
    printf("  MonitoringAttributes: %u\n", p->tags.MonitoringAttributes);
    printf("Metric List:\n");
    printf("  Execution_Duration: %u\n", p->metrics.Execution_Duration);
    printf("  CPU_Utilization: %u\n", p->metrics.CPU_Utilization);
    printf("  Instructions_per_Cycle: %u\n", p->metrics.Instructions_per_Cycle);
    printf("  LLC_Misses: %u\n", p->metrics.LLC_Misses);
    printf("  LLC_Hit_Rate: %u\n", p->metrics.LLC_Hit_Rate);
    printf("  LLC_Occupancy: %u\n", p->metrics.LLC_Occupancy);
    printf("  Memory_Usage: %u\n", p->metrics.Memory_Usage);
    printf("  Memory_Bandwidth: %u\n", p->metrics.Memory_Bandwidth);
    printf("  Page_Faults: %u\n", p->metrics.Page_Faults);
    printf("  TX_Bytes: %u\n", p->metrics.TX_Bytes);
    printf("  RX_Bytes: %u\n", p->metrics.RX_Bytes);
    printf("  Dropped_Packets: %u\n", p->metrics.Dropped_Packets);
    printf("  RTT: %u\n", p->metrics.RTT);
    printf("  Retransmissions: %u\n", p->metrics.Retransmissions);
    printf("  File_System_Usage: %u\n", p->metrics.File_System_Usage);
    printf("  File_Write_Speed: %u\n", p->metrics.File_Write_Speed);
    printf("  File_Read_Speed: %u\n", p->metrics.File_Read_Speed);
    printf("  Context_Switches: %u\n", p->metrics.Context_Switches);
    printf("  Software_Interrupts: %u\n", p->metrics.Software_Interrupts);
    printf("  Lock_Contentions: %u\n", p->metrics.Lock_Contentions);
    printf("  Application_Metrics: %u\n", p->metrics.Application_Metrics);
    printf("-----------------------------\n");
    printf("\n");



    return 0;
}


void parse_args(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "c:t:")) != -1) {
        switch (opt) {
            case 'c':
                pid_filter_file = optarg;
                printf("PID filter file: %s\n", pid_filter_file);
                break;
            case 't':
                tags_config_file = optarg;
                printf("Tags config file: %s\n", tags_config_file);
                break;
            default:
                fprintf(stderr, "Usage: %s -c <pid_filter_file>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}



int run(struct ring_buffer **rb) {
    int ret = 0;

    while (1) {
        ret = ring_buffer__poll(*rb, RING_BUFFER_POLL_TIMEOUT);

        if (ret == -EINTR) {
            fprintf(stdout, "Interrupted, exiting gearbox agent\n");
            ret = 0;
            break;
        }
        
        if (ret < 0) {
            fprintf(stderr, "Error polling ring buffer: %s\n", strerror(errno));
            break;
        }
    }

    return ret;
}


int main(int argc, char **argv) {
    struct ring_buffer *rb = NULL;
    struct tag_list *tags;
    int count;

    parse_args(argc, argv);
    update_tags_map(tags_config_file, PID_TAGS_MAP_PATH, &tags, &count);

    read_pids_and_update_map(pid_filter_file, PID_MAP_PATH);

    // read_trace_pipe();

    rb = ring_buffer__new(bpf_obj_get(RING_BUFFER_PATH), collect_rb, NULL, NULL);
    if (!rb) {
        fprintf(stderr, "Failed to open ring buffer\n");
        return -1; // TODO: Change to error code
    }

    run(&rb);

    if (rb) {
        ring_buffer__free(rb);
    }
    return 0;
}
