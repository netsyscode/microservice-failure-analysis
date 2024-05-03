# Maps Documentation for Gearbox

Gearbox uses several eBPF maps (implemented in `maps.h`) to manage and propagate context across different system components efficiently. These maps are crucial for maintaining state, managing data, and ensuring that context is propagated correctly through network events and system calls. Below is a detailed description of each map used in the Gearbox implementation.

## Overview of Maps

Gearbox employs a variety of maps, each serving specific roles in 

- Storing kernel information and communication
- Context propagation
- Metrics collection.

### `flow_context_map`

Used for inter-component context propagation via TCP Option injection. It maps a 4-tuple key (source IP, destination IP, source port, and destination port) to a message context which includes trace IDs, sender IDs, and invocation IDs.

- **Type**: BPF_MAP_TYPE_HASH
- **Key**: `struct flow_tuple`
- **Value**: `struct msg_context`
- **Max Entries**: 10000

### `pid_context_map`

Utilized for intra-component context propagation. It associates a process identifier (TGID) with its corresponding message context to manage context within the same component.

- **Type**: BPF_MAP_TYPE_HASH
- **Key**: `u32` (TGID)
- **Value**: `struct msg_context`
- **Max Entries**: 100

### `read_args_map`

Stores arguments from read system calls, facilitating efficient data retrieval during system call exits. It maps a TGID to `data_args` which includes references to relevant TCP sockets structure.

- **Type**: BPF_MAP_TYPE_HASH
- **Key**: `u32` (TGID)
- **Value**: `struct data_args`
- **Max Entries**: 1000

### `IO_state_map`

Manages the I/O state of connections (FIRST_RECV, NOT_FIRST_RECV, FIRST_SEND, NOT_FIRST_SEND, INFER_ERROR) to determine if data that is currently handling by this syscall is part of an ongoing application-level message or new. For the first recv/send system call, Gearbox will conduct metric collection and context propagation.

- **Type**: BPF_MAP_TYPE_LRU_HASH
- **Key**: `struct tgid_sport`
- **Value**: `enum IO_state`
- **Max Entries**: 20000

### `invoke_state_map`

Tracks invocation states for points within the same trace, helping maintain the order and consistency of trace points across the system.

- **Type**: BPF_MAP_TYPE_LRU_HASH
- **Key**: `struct trace_pid`
- **Value**: `struct invoke_state`
- **Max Entries**: 10000

### `pid_map`

Filters system call hooks based on process identifiers generated in user space, ensuring that only relevant processes are monitored. This is engineer implementation.

- **Type**: BPF_MAP_TYPE_HASH
- **Key**: `u32`
- **Value**: `u32`
- **Max Entries**: 100

### `rb`

Utilized for sending data to user space. It facilitates efficient data transfer between kernel space and user space, crucial for real-time monitoring.

- **Type**: BPF_MAP_TYPE_RINGBUF
- **Max Entries**: 25600 * 1024 (2560 KB)

### `client_ip_map`

Stores (currently only 1) client IP data for tracking ingress connections, used primarily to manage new incoming requests and their context propagation.

- **Type**: BPF_MAP_TYPE_HASH
- **Key**: `u32`
- **Value**: `struct flow_tuple`
- **Max Entries**: 10

> *TODO:* The existing implementation is quite adhoc because there are some bugs that have not yet been fixed.

### `now_trace_id_map`

Tracks the current trace ID to ensure consistency across propagated contexts, crucial for maintaining trace integrity throughout the system.

- **Type**: BPF_MAP_TYPE_HASH
- **Key**: `__u32`
- **Value**: `__u64`
- **Max Entries**: 1

> *TODO:* With the `client_ip_map`, this map is should not be needed. However, since the trace id might remain unchanged now, for consistency, it needs to be retained here until the bug is fixed.