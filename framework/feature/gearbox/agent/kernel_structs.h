#ifndef __KERNEL_STRUCTS_H__
#define __KERNEL_STRUCTS_H__

/** 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * Enums 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 */

enum syscall_name {
    READ,
    RECVMSG,
    RECVMMSG,
    READV,
    RECVFROM,
    WRITE,
    SENDMSG,
    SENDMMSG,
    SENDTO,
    WRITEV,
} __attribute__((packed));

enum syscall_type {
    RECV_TYPE,
    SEND_TYPE,
} __attribute__((packed));

enum msg_type {
    EDGE,
    POINT,
} __attribute__((packed));

// The state machine for inferring the I/O state
enum IO_state {
    FIRST_RECV,
    NOT_FIRST_RECV,
    FIRST_SEND,
    NOT_FIRST_SEND,
    INFER_ERROR,
} __attribute__((packed));

/** 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * General structs 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 */

// struct tcp_int_opt is used to store the struct msg_context in TCP Option
// Compared with struct msg_context, struct tcp_int_opt contains additional 
// info: .kind = TCP_INT_OPT_KIND; .len = sizeof(iopt);
struct tcp_int_opt {
    __u8 kind;
    __u8 len;
    __u64 trace_id;
    __u64 sender_id;
    __u16 invoke_id;
} __attribute__((packed));

// struct msg_context stores the nessary info to construct correlation
struct msg_context {
    __u64 trace_id;
    __u64 sender_id;
    __u16 invoke_id;
} __attribute__((packed));

struct flow_tuple {
    __u32 sport;
    __u32 dport;
    __u32 sip;
    __u32 dip;
} __attribute__((packed));

// struct data_args is used to wrap and store the struct tcp_sock,
// which is retrieved from the socket file descriptor via get_tsk_from_fd()
struct data_args {
    struct tcp_sock *tsk;
} __attribute__((packed));

// Metrics collected by Gearbox
struct metrics {
    __u32 srtt_us;
    __u32 mdev_max_us;      // maximal mdev for the last rtt period
	__u32 rttvar_us;        // smoothed mdev_max
    __u32 mdev_us;          // medium deviation of rtt, reflect shake of rtt
    __u64 bytes_sent;       // total bytes sent
    __u64 bytes_received;   // total bytes recieved
    __u64 bytes_acked;      // total bytes acked
    __u32 delivered;        // Total data packets delivered incl. rexmits
    __u32 snd_cwnd;         // Sending congestion window		*/
    __u32 rtt_us;           // Receiver side RTT estimation */
    __u64 duration;

    // __u32 packet_out; // unacked packet
    // __u32 loss_pkt_out; // loss packet
    // __u32 lost_pkt; // total lost packet
    // __u32 retran_pkt_out; // unacked retransport packet
    // __u32 retran_pkt; // total retransport packet
    // __u32 retran_bytes; // total retransport bytes
} __attribute__((packed));

// TODO: 这个结构干啥的来着忘了
struct trace_pid {
    __u64 trace_id;
    __u32 tgid;
} __attribute__((packed));

// This struct maintains the invoke times and last timestamp of points from the same trace
struct invoke_state {
    __u16 invoke_times;
    __u64 last_timestamp;
}__attribute__((packed));

struct point {
    __u64 trace_id;
    __u64 component_id;
    __u16 invoke_id;
} __attribute__((packed));

struct edge {
    struct point p1;
    struct point p2;
} __attribute__((packed));

struct tgid_sport {
    __u32 tgid;
    __u16 sport;
} __attribute__((packed));

/** 
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * Structs for outputting monitoring results
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 */

// Wrap struct point with struct point_msg
struct point_msg {
    enum msg_type type;
    struct point p;
    struct metrics m;
} __attribute__((packed));

// Wrap struct edge with struct edge_msg
struct edge_msg {
    enum msg_type type;
    struct edge e;
} __attribute__((packed));

#endif // __KERNEL_STRUCTS_H__