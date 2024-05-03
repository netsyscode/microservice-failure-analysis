# Overview of Instrumentation Hooks

The eBPF program utilizes a series of hooks to intercept network operations at different stages of data transmission (sending and receiving) and during socket operations. The implementation of these hooks can be found at `kernel.c`. Below is the categorization of these hooks:

## Receiving Hooks & Sending Hooks

Receiving hooks and sending hooks are implemented to conduct **metric collection** and **context propagation**.

### Receiving Hooks

Hooks in this category are triggered during the entry and exit points of system calls related to receiving network data. They allow the eBPF program to monitor and act upon data being received by the system. The following hooks are implemented:

- **Read Operations**
  - Entry: `SEC("tracepoint/syscalls/sys_enter_read")`
    - Action: `process_enter_recv(ctx, READ);`
  - Exit: `SEC("tracepoint/syscalls/sys_exit_read")`
    - Action: `process_exit_recv(ctx, READ);`
- **Message Receiving Operations**
  - For `recvmsg`, `recvmmsg`, `readv`, and `recvfrom` operations, both entry and exit points are hooked similarly to read operations, with specific actions tied to each syscall type, allowing for detailed monitoring and control over these receiving paths.

> *TODO:* Here, Gearbox reads the parameters at `process_enter_recv`, and executes the actual logic at `process_exit_recv`. This is because some bugs appeared in the previous implementation. This issue will be attempted to be fixed in future development.

### Sending Hooks

These hooks are activated at the system call entry points related to sending network data. They enable the observation and manipulation of outbound network traffic. The hooks include:

- **Write Operations**
  - Entry: `SEC("tracepoint/syscalls/sys_enter_write")`
    - Action: `process_enter_send(ctx, WRITE);`
- **Message Sending Operations**
  - For `sendmsg`, `sendmmsg`, `writev`, and `sendto` operations, entry points are hooked to invoke actions specific to each type of sending operation, facilitating precise intervention in data sending processes.

## Socket Operations Hooks

Socket operation hooks are implemented to conduct **TCP Option injection**.
This includes enabling, reserving space for, injecting, and retrieving custom TCP header options utilized by Gearbox.

The implementation is relied on `SEC("sockops")` hooks, which intercept various socket operations, allowing for a wide range of actions to be performed, from connection establishment to header option manipulation. 

The `tcp_int_*` function is designed to handle different socket operation callbacks.

> **Note:** For SEC("sockops"), we cannot capture a valid tgid. Therefore, Gearbox can only use the five-tuple of the stream as the core basis for association here.

### Enabling TCP Option Injection During Connection and Establishment Time

Actions are performed based on the operation type, such as enabling TCP options or establishing callbacks.
For `BPF_SOCK_OPS_TCP_CONNECT_CB`, `BPF_SOCK_OPS_ACTIVE_ESTABLISHED_CB`, and `BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB`, try to enable TCP Option injection by overwritting callback flags with

```C
cb_flags = skops->bpf_sock_ops_cb_flags |
            BPF_SOCK_OPS_WRITE_HDR_OPT_CB_FLAG |
            BPF_SOCK_OPS_PARSE_UNKNOWN_HDR_OPT_CB_FLAG |
            BPF_SOCK_OPS_PARSE_ALL_HDR_OPT_CB_FLAG;

bpf_sock_ops_cb_flags_set(skops, cb_flags);
```

### Reserving Header Spaces for Gearbox Header

For `BPF_SOCK_OPS_HDR_OPT_LEN_CB`, reserve space for injected structure `struct tcp_int_opt` with `bpf_reserve_hdr_opt()`.

### Inject Gearbox Header

For `BPF_SOCK_OPS_WRITE_HDR_OPT_CB`, lookup context by flow tuple in BPF map `flow_context_map`, construct `struct tcp_int_opt`, and inject it with `bpf_store_hdr_opt()`.

### Retrieve Gearbox Header

For `BPF_SOCK_OPS_PARSE_HDR_OPT_CB`, load `struct tcp_int_opt` via `bpf_load_hdr_opt` from TCP Option, parse it to construct `struct msg_context`, and store it with `struct flow_tuple` as key in BPF map `flow_context_map`.
