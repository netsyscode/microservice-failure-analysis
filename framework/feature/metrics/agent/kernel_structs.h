#ifndef __KERNEL_STRUCTS_H__
#define __KERNEL_STRUCTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef uint32_t u32;

struct tag_list {
    u32 HostIP;
    u32 ServiceID;
    u32 ContainerID;
    u32 ProcessID;
    u32 ComponentID;
    u32 k8sLabel;
    u32 ServiceType;
    u32 EnvironmentType;
    u32 ReleaseType;
    u32 Versions;
    u32 PathID;
    u32 SrcIP;
    u32 DstIP;
    u32 SrcPort;
    u32 DstPort;
    u32 Protocol;
    u32 CliendID;
    u32 OperationName;
    u32 API;
    u32 ObservationPoints;
    u32 MonitoringAttributes;
    u32 PmuIndex;
} __attribute__((packed));

struct metric_list {
    u32 Execution_Duration;
    u32 CPU_Utilization;
    u32 Instructions;
    u32 LLC_Misses;
    u32 LLC_Hit_Rate;
    u32 LLC_Occupancy;
    u32 Memory_Usage;
    u32 Memory_Bandwidth;
    u32 Page_Faults;
    u32 TX_Bytes;
    u32 RX_Bytes;
    u32 Dropped_Packets;
    u32 RTT;
    u32 Retransmissions;
    u32 File_System_Usage;
    u32 File_Write_Speed;
    u32 File_Read_Speed;
    u32 Context_Switches;
    u32 Software_Interrupts;
    u32 Lock_Contentions;
    u32 Application_Metrics;
} __attribute__((packed));

struct point {
    struct tag_list tags;
    struct metric_list metrics;
} __attribute__((packed));

#endif // __KERNEL_STRUCTS_H__