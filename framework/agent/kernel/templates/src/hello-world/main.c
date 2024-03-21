#include "all.h"

SEC("tracepoint/syscalls/sys_enter_execve")
int bpf_prog(void *ctx) {
  //char msg[] = "Hello, BPF World!";
  //bpf_trace_printk(msg, sizeof(msg));

  u64 pid_tgid = bpf_get_current_pid_tgid();
  u32 pid = pid_tgid >> 32;
  // u32 tgid = (u32) pid_tgid;

  char comm[32];
  bpf_get_current_comm(&comm, sizeof(comm));

  bpf_printk("execve enter (tracepoint) - PID: %u, Comm: %s\n", pid, comm);
  return 0;
}

char _license[] SEC("license") = "GPL";