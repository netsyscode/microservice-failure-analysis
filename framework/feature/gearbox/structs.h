
#ifndef __LIB_STRUCTS_H_
#define __LIB_STRUCTS_H_

// __u32 HOST_IP = 3396398373; // 202.112.237.37
__u32 HOST_IP = 3396398369; // 202.112.237.33

enum traffic_protocol
{
    PROTO_UNKNOWN,
    PROTO_ORTHER,
    PROTO_HTTP1,
    PROTO_HTTP2,
    PROTO_TLS_HTTP1,
    PROTO_TLS_HTTP2,
    PROTO_DUBBO,
    PROTO_SOFARPC,
    PROTO_MYSQL,
    PROTO_POSTGRESQL,
    PROTO_REDIS,
    PROTO_KAFKA,
    PROTO_MQTT,
    PROTO_DNS,
    PROTO_NUM,
};

enum msg_direction
{
    REQUEST,
    RESPONSE,
};

enum msg_type
{
    WRITE,
    READ,
    ADD_OPT,
    PARSE_OPT,
    SENDTO,
    RECVFROM,
    SENDMSG,
    RECVMSG,
    SENDMMSG,
    RECVMMSG,
    WRITEV,
    READV,
    EDGE,
    POINT,
} __attribute__((packed));

enum IO_state
{
    FIRST_READ,
    NOT_FIRST_READ,
    FIRST_WRITE,
    NOT_FIRST_WRITE,
} __attribute__((packed));

struct flow_tuple
{
    __u32 sport;
    __u32 dport;
    __u32 sip;
    __u32 dip;
} __attribute__((packed));

struct tcp_int_opt
{
    __u8 kind;
    __u8 len;
    __u64 traceID;
    __u64 senderID;
    __u16 invokeID;
    __u16 edgeNum;
} __attribute__((packed));

struct MsgContext
{
    __u64 traceID;
    __u64 senderID;
    __u16 invokeID;
    __u16 edgeNum;
} __attribute__((packed));

struct send_write_msg
{
    enum msg_type type;
    struct flow_tuple tuple;
    enum traffic_protocol L7_proto;
    enum msg_direction direction;
    __u32 spanID;
    __u32 tgid;
    __u64 timestamp;
    __u32 seq;
    bool flag; // Used to determine if sys_enter_read is triggered
    __u64 buf_len;
    enum IO_state io_s;
    struct MsgContext context;
    __u32 srtt_us;
    char *buf_addr;
    char buf[300];
} __attribute__((packed));

struct option_msg
{
    enum msg_type type;
    struct flow_tuple tuple;
    __u32 tgid;
    __u64 timestamp;
    __u32 seq;                 // read write
    struct tcp_int_opt option; // option
    __u64 sk_cookie;
} __attribute__((packed));

struct point
{
    __u64 traceID;
    __u64 componentID;
    __u16 invokeID;
} __attribute__((packed));

struct edge
{
    struct point p1;
    struct point p2;
    __u16 edgeNum;
} __attribute__((packed));

struct TracePID
{
    __u64 traceID;
    __u32 tgid;
} __attribute__((packed));



struct metrics
{
    __u32 srtt_us;
    __u32 mdev_max_us;	/* maximal mdev for the last rtt period	*/
	__u32 rttvar_us;	/* smoothed mdev_max			*/
    __u32 mdev_us; //medium deviation of rtt, reflect shake of rtt
    __u64 bytes_sent; //total bytes sent
    __u64 bytes_received; //total bytes recieved
    __u64 bytes_acked; //total bytes acked
    __u32 delivered;	/* Total data packets delivered incl. rexmits */
    __u32 snd_cwnd;	/* Sending congestion window		*/
    __u32 rtt_us; /* Receiver side RTT estimation */
    __u64 duration;

    // __u32 packet_out; // unacked packet
    // __u32 loss_pkt_out; // loss packet
    // __u32 lost_pkt; // total lost packet
    // __u32 retran_pkt_out; // unacked retransport packet
    // __u32 retran_pkt; // total retransport packet
    // __u32 retran_bytes; // total retransport bytes
} __attribute__((packed));

struct edge_msg
{
    enum msg_type type;
    struct edge e;
} __attribute__((packed));

struct point_msg
{
    enum msg_type type;
    struct point p;
    struct metrics m;
} __attribute__((packed));

struct data_args
{
    struct tcp_sock *tsk;
} __attribute__((packed));

struct invoke_point
{
    __u16 invokeid;
    __u64 timestamp;
}__attribute__((packed));

struct tgid_sport
{
    __u32 tgid;
    __u16 sport;
} __attribute__((packed));

struct edge_for_path
{
    __u64 traceID;
    __u64 componentID1;
    __u16 invokeID1;
    __u64 componentID2;
    __u16 invokeID2;
    __u16 num;
} __attribute__((packed));


#endif