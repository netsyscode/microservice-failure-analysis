
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "agent_bpf.h"

int attach_to_cgroup(const char *cgroup_path, const char *tcp_int_path, const char *new_link_path) {
    int bpf_cgroup_fd, bpf_prog_fd, err;
    struct bpf_object *obj = NULL;
    struct bpf_program *prog = NULL;
    struct bpf_link *link = NULL;

    bpf_cgroup_fd = open(cgroup_path, O_RDONLY);
    if (bpf_cgroup_fd < 0) {
        fprintf(stderr, "Failed to open cgroup path: %s because %s\n", cgroup_path, strerror(errno));
        return -1;
    }
    DEBUG("Opened cgroup path: %s\n", cgroup_path);

    obj = bpf_object__open(tcp_int_path);
    if (!obj) {
        fprintf(stderr, "Failed to open BPF object from path: %s\n", tcp_int_path);
        close(bpf_cgroup_fd);
        return -1;
    }
    DEBUG("Opened BPF object from path: %s\n", tcp_int_path);
    return 0;

    prog = bpf_object__find_program_by_name(obj, "tcp_int");
    if (!prog) {
        fprintf(stderr, "Failed to find BPF program: tcp_int\n");
        close(bpf_cgroup_fd);
        bpf_object__close(obj);
        return -1;
    }

    link = bpf_program__attach_cgroup(prog, bpf_cgroup_fd);
    if (!link) {
        fprintf(stderr, "Failed to attach BPF program to cgroup\n");
        close(bpf_cgroup_fd);
        bpf_object__close(obj);
        return -1;
    }

    // Pin link
    err = bpf_link__pin(link, new_link_path);
    if (err) {
        fprintf(stderr, "Failed to pin BPF link: %s\n", strerror(err));
        close(bpf_cgroup_fd);
        bpf_object__close(obj);
        bpf_link__destroy(link);
        bpf_object__close(obj);
        return -1;
    }

    bpf_object__close(obj);
    bpf_link__destroy(link);
    bpf_object__close(obj);
    return 0;
}

int read_pids_and_update_map(const char *pid_config_path, const char *pid_map_path) {
    int ret = 0;
    FILE *fp;
    char line[1024];

    fp = fopen(pid_config_path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open file: %s because %s\n", pid_config_path, strerror(errno));
        ret = -1;
        return ret;
    }

    int pid_map_fd = bpf_obj_get(pid_map_path);
    if (pid_map_fd < 0) {
        fprintf(stderr, "Failed to get BPF map for PID filtering: %s because %s\n", pid_map_path, strerror(errno));
        ret = -1;
        goto cleanup;
    }

    while (fgets(line, sizeof(line), fp)) {
        __u32 pid = atoi(line);
        __u32 value = 1;

        if (pid > 0) {
            if (bpf_map_update_elem(pid_map_fd, &pid, &value, BPF_ANY) != 0) {
                fprintf(stderr, "Failed to update BPF map for PID %d: %s\n", pid, strerror(errno));
                ret = -1;
                break;
            } else {
                INFO("PID %d added to the map successfully.\n", pid);
            }
        } else {
            fprintf(stderr, "Invalid PID: %s\n", line);
        }
    }

    close(pid_map_fd);

cleanup:
    fclose(fp);
    return ret;
}