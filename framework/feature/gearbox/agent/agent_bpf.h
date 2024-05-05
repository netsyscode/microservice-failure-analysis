#ifndef __AGENT_BPF_H__
#define __AGENT_BPF_H__

#include "agent_util.h"

int attach_to_cgroup(const char *cgroup_path, const char *tcp_int_path, const char *new_link_path);
int read_pids_and_update_map(const char *pid_config_path, const char *pid_map_path);

#endif // __AGENT_BPF_H__