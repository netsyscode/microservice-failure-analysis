# Gearbox eBPF Program Documentation

This document provides an overview of the eBPF program developed for Gearbox, detailing its instrumentation hooks for monitoring and manipulating network traffic efficiently. The eBPF (Extended Berkeley Packet Filter) program is designed to hook into various system call entry and exit points, as well as socket operations, to observe and potentially alter the behavior of network communication.

## Overview of Instrumentation Hooks

The eBPF program utilizes a series of hooks to intercept network operations at different stages of data transmission (sending and receiving) and during socket operations. Below is the categorization of these hooks:

### Receiving Hooks

Hooks in this category are triggered during the entry and exit points of system calls related to receiving network data. They allow the eBPF program to monitor and act upon data being received by the system. The following hooks are implemented:

- **Read Operations**
  - Entry: `SEC("tracepoint/syscalls/sys_enter_read")`
    - Action: `process_enter_recv(ctx, READ);`
  - Exit: `SEC("tracepoint/syscalls/sys_exit_read")`
    - Action: `process_exit_recv(ctx, READ);`
- **Message Receiving Operations**
  - For `recvmsg`, `recvmmsg`, `readv`, and `recvfrom` operations, both entry and exit points are hooked similarly to read operations, with specific actions tied to each syscall type, allowing for detailed monitoring and control over these receiving paths.

### Sending Hooks

These hooks are activated at the system call entry points related to sending network data. They enable the observation and manipulation of outbound network traffic. The hooks include:

- **Write Operations**
  - Entry: `SEC("tracepoint/syscalls/sys_enter_write")`
    - Action: `process_enter_send(ctx, WRITE);`
- **Message Sending Operations**
  - For `sendmsg`, `sendmmsg`, `writev`, and `sendto` operations, entry points are hooked to invoke actions specific to each type of sending operation, facilitating precise intervention in data sending processes.

### Socket Operations Hooks

The `SEC("sockops")` hook intercepts various socket operations, allowing for a wide range of actions to be performed, from connection establishment to header option manipulation. The `tcp_int_*` function is designed to handle different socket operation callbacks:

- **Connection and Establishment Callbacks**
  - Actions are performed based on the operation type, such as enabling TCP options or establishing callbacks.
    - For `BPF_SOCK_OPS_TCP_CONNECT_CB`, `BPF_SOCK_OPS_ACTIVE_ESTABLISHED_CB`, and `BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB`

- **Header Option Callbacks**
  - These include reserving space for, adding, and processing custom TCP header options, significantly enhancing the capability to modify TCP behavior according to the program's needs.
    - For `BPF_SOCK_OPS_TCP_CONNECT_CB`, `BPF_SOCK_OPS_ACTIVE_ESTABLISHED_CB`, and `BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB`, overwrite TCP Option injection with `bpf_sock_ops_cb_flags_set()`
    - For `BPF_SOCK_OPS_HDR_OPT_LEN_CB`, reserve space for injected structure `struct tcp_int_opt` with `bpf_reserve_hdr_opt()`
    - For `BPF_SOCK_OPS_WRITE_HDR_OPT_CB`, lookup context by tuple in BPF map `flow_context_map`, construct `struct tcp_int_opt`, and inject it with `bpf_store_hdr_opt()`
    - For `BPF_SOCK_OPS_PARSE_HDR_OPT_CB`, load `struct tcp_int_opt` from TCP Option, parse it to construct `struct msg_context`, and store it with `struct flow_tuple` as key in BPF map `flow_context_map`
