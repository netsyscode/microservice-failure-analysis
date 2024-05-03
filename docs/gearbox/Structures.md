# Structures Documentation for Gearbox

Gearbox employs a variety of data structures to manage complex interactions and maintain state within the system efficiently. These structures are pivotal for storing context, managing network flows, and tracking performance metrics. Below is a detailed overview of each structure used in Gearbox.

## Enumeration Types

Enumeration types in Gearbox define sets of named constants, which are used to represent various states or types across the system. They are essential for readability and maintenance of the code.

### `enum syscall_name`

- **Values**:
  - `READ`
  - `RECVMSG`
  - `RECVMMSG`
  - `READV`
  - `RECVFROM`
  - `WRITE`
  - `SENDMSG`
  - `SENDMMSG`
  - `SENDTO`
  - `WRITEV`
- **Description**: Represents the names of system calls that Gearbox hooks into for processing. These identifiers help in mapping the intercepted system call data to appropriate handling functions. This enum type is mainly used for debugging.

### `enum syscall_type`

- **Values**:
  - `RECV_TYPE`
  - `SEND_TYPE`
- **Description**: Categorizes the types of system calls into receive and send operations. This distinction is crucial for context propagation logic that depends on the direction of data flow.

### `enum msg_type`

- **Values**:
  - `EDGE`
  - `POINT`
- **Description**: Used in messages (transmitted to the user space) to distinguish between different types of trace elements being processed or transmitted, such as points in the execution trace or edges that represent connections between these points.

### `enum IO_state`

- **Values**:
  - `FIRST_RECV`
  - `NOT_FIRST_RECV`
  - `FIRST_SEND`
  - `NOT_FIRST_SEND`
  - `INFER_ERROR`
- **Description**: Describes the I/O state of a network connection, indicating whether a data segment is part of an ongoing consecutive system calls of the same type (which, is likely that these sysytem calls belong to the same application-layer message) or new. This is vital for correctly applying trace context in dynamic network environments.

## Key Structures

### `struct flow_tuple`

- **Fields**:
  - `__u32 sport`: Source port
  - `__u32 dport`: Destination port
  - `__u32 sip`: Source IP address
  - `__u32 dip`: Destination IP address
- **Description**: Represents the 4-tuple key used for identifying network flows. This structure is critical for mapping network packets to their corresponding context in various maps.

### `struct msg_context`

- **Fields**:
  - `__u64 trace_id`: Unique identifier for the trace
  - `__u64 sender_id`: Identifier of the sender component
  - `__u16 invoke_id`: Invocation identifier within the trace, it specifies a specific `struct point` together with `sender_id`
- **Description**: Stores essential context for message tracing, including identifiers necessary to track and propagate the context through different stages of network communication.

### `struct tcp_int_opt`

- **Fields**:
  - `__u8 kind`: The type of the TCP option; in this case, a custom kind defined by the Gearbox.
  - `__u8 len`: Length of the TCP option.
  - `__u64 trace_id`: Trace identifier, used to correlate network events and data across the system.
  - `__u64 sender_id`: Identifier for the sender of the packet, aiding in the trace of source-destination relationships.
  - `__u16 invoke_id`: Invocation identifier, used to track specific calls or actions within a larger trace.
- **Description**: This structure is used to encapsulate `struct msg_context` information in TCP options, enabling the injection and extraction of tracing data directly within TCP headers. It is critical for maintaining state and context across network boundaries, especially in distributed systems.

### `struct data_args`

- **Fields**:
  - `struct tcp_sock *tsk`: Pointer to the TCP socket structure
- **Description**: Used to wrap the TCP socket structure retrieved during system call handling, facilitating easy access and manipulation of socket-specific data.

### `struct metrics`

- **Fields**:
  - `__u32 srtt_us`: Smoothed round-trip time
  - `__u32 mdev_max_us`: Maximum deviation of RTT during the last measurement period
  - `__u32 rttvar_us`: Smoothed mean deviation of RTT
  - `__u32 mdev_us`: Median deviation of RTT
  - `__u64 bytes_sent`: Total bytes sent
  - `__u64 bytes_received`: Total bytes received
  - `__u64 bytes_acked`: Total bytes acknowledged
  - `__u32 delivered`: Total packets delivered, including retransmissions
  - `__u32 snd_cwnd`: Sender's congestion window size
  - `__u32 rtt_us`: Receiver side RTT estimation
  - `__u64 duration`: Duration between events in a trace
- **Description**: Collects various metrics from the TCP stack, providing detailed insights into network performance and behavior for each tracked connection.

### `struct point`

- **Fields**:
  - `__u64 trace_id`: Trace identifier, calculated by `((u64)HOST_IP << 32) + (bpf_ktime_get_ns() & 0xFFFFFFFF)`
  - `__u64 component_id`: Component identifier, calculated by `((u64)HOST_IP << 32) + tgid`
  - `__u16 invoke_id`: Invocation identifier, for each trace, its invoke id in each component start with 0.
- **Description**: Represents a point in the trace, marking significant events or stages in data processing, used in conjunction with edges to form a trace graph.

### `struct edge`

- **Fields**:
  - `struct point p1`: Start point of the edge
  - `struct point p2`: End point of the edge
- **Description**: Defines a directional relationship between two points in a trace, crucial for constructing the path and flow of execution across components.

### `struct tgid_sport`

- **Fields**:
  - `__u32 tgid`: Thread group ID
  - `__u16 sport`: Source port
- **Description**: Used as a key in maps for tracking I/O states, linking network events to specific processes and ports.

### `struct invoke_state`

- **Fields**:
  - `__u16 invoke_times`: Number of invocations
  - `__u64 last_timestamp`: Timestamp of the last invocation
- **Description**: Tracks invocation states, helping to manage and coordinate the timing and sequence of `struct point` within a trace.

### `struct point_msg` and `struct edge_msg`

- **Fields**:
  - `enum msg_type type`: Type of message (POINT or EDGE)
  - `struct point p` or `struct edge e`: The data structure representing the point or edge
  - `struct metrics m`: Metrics associated with a point (only in `point_msg`)
- **Description**: These structures wrap points and edges with additional metadata, such as type and associated metrics, facilitating their transmission and processing.

### `struct trace_pid`

- **Fields**:
  - `__u64 trace_id`: Trace identifier which corresponds to a specific execution trace across components.
  - `__u32 tgid`: Thread Group ID, used to link the trace to a specific process or group of threads.
- **Description**: Used as key to maintain a mapping between ongoing traces and invocation states (i.e., points and point sequences). 