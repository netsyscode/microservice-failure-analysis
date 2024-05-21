
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
    int cgroup_fd, prog_fd, link_fd, err = 0;
    struct bpf_link *link = NULL;
    enum bpf_attach_type attach_type;

    cgroup_fd = open(cgroup_path, O_RDONLY);
    if (cgroup_fd < 0) {
        fprintf(stderr, "Failed to open cgroup path: %s because %s\n", cgroup_path, strerror(errno));
        err = -1;
        goto ret;
    }
    DEBUG("Opened cgroup path: %s\n", cgroup_path);

    prog_fd = bpf_obj_get(tcp_int_path);
    if (prog_fd < 0) {
        fprintf(stderr, "Failed to open BPF object from path: %s\n", tcp_int_path);
        err = -1;
        goto close_cgroup;
    }
    DEBUG("Opened BPF object from path: %s\n", tcp_int_path);

    // The attach type is derived from the output of loader-capsule
    attach_type = BPF_CGROUP_SOCK_OPS;
    link_fd = bpf_link_create(prog_fd, cgroup_fd, attach_type, NULL);
    if (link_fd < 0) {
        fprintf(stderr, "Failed to create BPF link: %s\n", strerror(errno));
        err = -1;
        goto close_prog;
    }
    DEBUG("Created BPF link\n");

    // Pin link
    err = bpf_obj_pin(link_fd, new_link_path);
    if (err) {
        fprintf(stderr, "Failed to pin BPF link: %s\n", strerror(err));
        err = -1;
        goto close_link;
    }
    DEBUG("Pinned BPF link to path: %s\n", new_link_path);

close_link:
    close(link_fd);

close_prog:
    close(prog_fd);

close_cgroup:
    close(cgroup_fd);

ret:
    return err;
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
        __u32 pid, value;
        sscanf(line, "%u %u", &pid, &value);

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