#include "all.h"

SEC("tracepoint/syscalls/sys_enter_execve")
int bpf_prog(void *ctx) {
  //char msg[] = "Hello, BPF World!";
  //bpf_trace_printk(msg, sizeof(msg));

  bpf_printk("execve enter (tracepoint)\n");
  return 0;
}

char _license[] SEC("license") = "GPL";