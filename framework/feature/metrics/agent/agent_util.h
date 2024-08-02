#ifndef __AGENT_UTIL_H__
#define __AGENT_UTIL_H__

#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <cjson/cJSON.h>
#include "kernel_structs.h"

void update_tags_map(const char *filename, const char *pid_tag_map_path, struct tag_list **tags, int *count)
{

    int ret = 0;

    int map_fd = bpf_obj_get(pid_tag_map_path);
    if (map_fd < 0)
    {
        fprintf(stderr, "Failed to get BPF map: %s because %s\n", pid_tag_map_path, strerror(errno));
        ret = -1;
        return;
    }


    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("File opening failed");
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char *)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    cJSON *json = cJSON_Parse(data);
    if (!json)
    {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        free(data);
        return;
    }

    cJSON *instances = cJSON_GetObjectItem(json, "instances");
    if (!cJSON_IsArray(instances))
    {
        printf("Error: instances is not an array\n");
        cJSON_Delete(json);
        free(data);
        return;
    }

    *count = cJSON_GetArraySize(instances);
    *tags = (struct tag_list *)malloc(*count * sizeof(struct tag_list));


    for (int i = 0; i < *count; i++)
    {
        cJSON *instance = cJSON_GetArrayItem(instances, i);
        if (!cJSON_IsObject(instance))
        {
            printf("Error: instance is not an object\n");
            continue;
        }

        (*tags)[i].HostIP = cJSON_GetObjectItem(instance, "HostIP")->valueint;
        (*tags)[i].ServiceID = cJSON_GetObjectItem(instance, "ServiceID")->valueint;
        (*tags)[i].ContainerID = cJSON_GetObjectItem(instance, "ContainerID")->valuedouble;
        (*tags)[i].ProcessID = cJSON_GetObjectItem(instance, "ProcessID")->valueint;
        (*tags)[i].ComponentID = cJSON_GetObjectItem(instance, "ComponentID")->valueint;
        (*tags)[i].k8sLabel = cJSON_GetObjectItem(instance, "k8sLabel")->valueint;
        (*tags)[i].ServiceType = cJSON_GetObjectItem(instance, "ServiceType")->valueint;
        (*tags)[i].EnvironmentType = cJSON_GetObjectItem(instance, "EnvironmentType")->valueint;
        (*tags)[i].ReleaseType = cJSON_GetObjectItem(instance, "ReleaseType")->valueint;
        (*tags)[i].Versions = cJSON_GetObjectItem(instance, "Versions")->valueint;
        (*tags)[i].PathID = cJSON_GetObjectItem(instance, "PathID")->valueint;
        (*tags)[i].SrcIP = cJSON_GetObjectItem(instance, "SrcIP")->valueint;
        (*tags)[i].DstIP = cJSON_GetObjectItem(instance, "DstIP")->valueint;
        (*tags)[i].SrcPort = cJSON_GetObjectItem(instance, "SrcPort")->valueint;
        (*tags)[i].DstPort = cJSON_GetObjectItem(instance, "DstPort")->valueint;
        (*tags)[i].Protocol = cJSON_GetObjectItem(instance, "Protocol")->valueint;
        (*tags)[i].CliendID = cJSON_GetObjectItem(instance, "CliendID")->valueint;
        (*tags)[i].OperationName = cJSON_GetObjectItem(instance, "OperationName")->valueint;
        (*tags)[i].API = cJSON_GetObjectItem(instance, "API")->valueint;
        (*tags)[i].ObservationPoints = cJSON_GetObjectItem(instance, "ObservationPoints")->valueint;
        (*tags)[i].MonitoringAttributes = cJSON_GetObjectItem(instance, "MonitoringAttributes")->valueint;

        __u32 pid = (*tags)[i].ProcessID;
        printf("PID: %u %u\n", pid, (*tags)[i].ContainerID);
        if (bpf_map_update_elem(map_fd, &pid, &(*tags)[i], BPF_ANY) != 0)
        {
            fprintf(stderr, "Failed to update BPF Tags map for PID %d: %s\n", pid, strerror(errno));
            ret = -1;
            break;
        }
        else
        {
            fprintf(stderr, "PID %d added to the Tags map successfully.\n", pid);
        }
    }

    cJSON_Delete(json);
    free(data);
}

int read_pids_and_update_map(const char *pid_config_path, const char *pid_map_path)
{
    int ret = 0;
    FILE *fp;
    char line[1024];

    fp = fopen(pid_config_path, "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to open file: %s because %s\n", pid_config_path, strerror(errno));
        ret = -1;
        return ret;
    }

    int pid_map_fd = bpf_obj_get(pid_map_path);
    if (pid_map_fd < 0)
    {
        fprintf(stderr, "Failed to get BPF map for PID filtering: %s because %s\n", pid_map_path, strerror(errno));
        ret = -1;
        goto cleanup;
    }

    while (fgets(line, sizeof(line), fp))
    {
        __u32 pid, value;
        sscanf(line, "%u %u", &pid, &value);

        if (pid > 0)
        {
            if (bpf_map_update_elem(pid_map_fd, &pid, &value, BPF_ANY) != 0)
            {
                fprintf(stderr, "Failed to update BPF map for PID %d: %s\n", pid, strerror(errno));
                ret = -1;
                break;
            }
            else
            {
                fprintf(stderr, "PID %d added to the map successfully.\n", pid);
            }
        }
        else
        {
            fprintf(stderr, "Invalid PID: %s\n", line);
        }
    }

    close(pid_map_fd);

cleanup:
    fclose(fp);
    return ret;
}

// void read_syscall_counts(const char *syscall_count_map_path)
// {
//     printf("Reading syscall counts\n");
//     FILE *log_file;
//     log_file = fopen("normal_log.txt", "w");
//     int pid_syscount_map_fd = bpf_obj_get(syscall_count_map_path);
//     if (pid_syscount_map_fd < 0)
//     {
//         fprintf(stderr, "Failed to get BPF map for PID syscall count: %s because %s\n", syscall_count_map_path, strerror(errno));
//         return;
//     }

//     __u64 key, next_key;
//     __u32 value;
//     int ret = 0;

//     while (1)
//     {
//         ret = bpf_map_get_next_key(pid_syscount_map_fd, &key, &next_key);
//         if (ret)
//         {
//             if (errno == ENOENT)
//             {
//                 // No more keys
//                 break;
//             }
//             else
//             {
//                 printf("Failed to get next key: %s\n", strerror(errno));
//                 break;
//             }
//         }
//         if (bpf_map_lookup_elem(pid_syscount_map_fd, &next_key, &value) == 0)
//         {
//             if(value == 0){
//                 continue;
//             }
//             __u32 pid = next_key >> 32;
//             __u32 sysid = next_key & 0xFFFFFFFF;
//             printf("PID: %u, sysID: %u count: %u\n", pid, sysid, value);
//             fprintf(log_file, "PID: %u, sysID: %u count: %u\n", pid, sysid, value);
//             value = 0;
//             bpf_map_update_elem(pid_syscount_map_fd, &next_key, &value, BPF_ANY);
//         }
//         key = next_key;
//     }
//     close(pid_syscount_map_fd);
//     fclose(log_file);

//     printf("--------------------\n");
// }

#endif // __AGENT_UTIL_H__