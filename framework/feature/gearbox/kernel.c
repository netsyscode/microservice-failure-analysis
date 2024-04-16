

#include "utils.h"
#define TCPHDR_SYN 0x02
#define TCPHDR_PSH 0x08
SEC("tracepoint/syscalls/sys_enter_write")
int enter_write(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_send(ctx, WRITE);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_read")
int enter_read(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_recv(ctx, READ);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_read")
int exit_read(struct trace_event_raw_sys_exit *ctx)
{
    process_exit_recv(ctx, READ);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_sendmsg")
int enter_sendmsg(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_send(ctx, SENDMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_recvmsg")
int enter_recvmsg(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_recv(ctx, RECVMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_recvmsg")
int exit_recvmsg(struct trace_event_raw_sys_exit *ctx)
{
    process_exit_recv(ctx, RECVMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_sendmmsg")
int enter_sendmmsg(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_send(ctx, SENDMMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_recvmmsg")
int enter_recvmmsg(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_recv(ctx, RECVMMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_recvmmsg")
int exit_recvmmsg(struct trace_event_raw_sys_exit *ctx)
{
    process_exit_recv(ctx, RECVMMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_writev")
int enter_writev(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_send(ctx, WRITEV);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_readv")
int enter_readv(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_recv(ctx, READV);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_readv")
int exit_readv(struct trace_event_raw_sys_exit *ctx)
{
    process_exit_recv(ctx, READV);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_sendto")
// SEC("kprobe/__x64_sys_sendto")
int enter_sendto(struct trace_event_raw_sys_enter *ctx)
{
    process_enter_send(ctx, SENDTO);
    return 0;
}

// __sys_recvfrom(int fd, void __user *ubuf, size_t size, unsigned int flags,
//      struct sockaddr __user *addr, int __user *addr_len)
SEC("tracepoint/syscalls/sys_enter_recvfrom")
int enter_recvfrom(struct trace_event_raw_sys_enter *ctx)
{

    process_enter_recv(ctx, RECVFROM);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_recvfrom")
// SEC("kretprobe/__x64_sys_recvfrom")
int exit_recvfrom(struct trace_event_raw_sys_exit *ctx)
{
    process_exit_recv(ctx, RECVMSG);
    return 0;
}


static bool skops_can_add_option(struct bpf_sock_ops *skops)
{

    if (skops->skb_len + sizeof(struct tcp_int_opt) > skops->mss_cache)
        return false;
    if (skops->skb_tcp_flags != 24)
    {
        return false;
    }

    return true;
}

static inline void tcp_int_add_tcpopt(struct bpf_sock_ops *skops)
{
    // u64 t1 = bpf_ktime_get_boot_ns();
    if (!skops_can_add_option(skops))
        return;
    struct flow_tuple tuple1 = {
        .sport = skops->local_port,
        .dport = bpf_ntohl(skops->remote_port),
        .sip = bpf_ntohl(skops->local_ip4),
        .dip = bpf_ntohl(skops->remote_ip4),
    };

    struct MsgContext *_context = bpf_map_lookup_elem(&tuple_context_map, &tuple1);
    struct tcp_int_opt iopt = {0};
    if (_context)
    {
        iopt.kind = TCP_INT_OPT_KIND;
        iopt.len = sizeof(iopt);
        iopt.senderID = _context->senderID;
        iopt.invokeID = _context->invokeID;
        iopt.traceID = _context->traceID;
        iopt.edgeNum = _context->edgeNum;
        if (0 == bpf_store_hdr_opt(skops, &iopt, sizeof(iopt), 0))
        {
            bpf_map_delete_elem(&tuple_context_map, &tuple1);

            // struct tcphdr *th = skops->skb_data;
            // u32 seq = bpf_ntohl(BPF_CORE_READ(th, seq));
            // struct option_msg msg1 = {
            //     .tuple = tuple1,
            //     .seq = seq,
            //     .tgid = 0,
            //     .timestamp = bpf_ktime_get_ns(),
            //     .type = ADD_OPT,
            //     .option = iopt,
            // };
            // bpf_ringbuf_output(&rb, &msg1, sizeof(msg1), 0);
        }
        // bpf_printk("add: %llu\n", bpf_ktime_get_boot_ns() - t1);
    }
}

static inline void tcp_int_process_tcpopt(struct bpf_sock_ops *skops)
{
    // u64 t1 = bpf_ktime_get_boot_ns();
    struct tcp_int_opt iopt = {};
    int rv;
    iopt.kind = TCP_INT_OPT_KIND;
    rv = bpf_load_hdr_opt(skops, &iopt, sizeof(iopt), 0);
    if (rv <= 0)
    {
        return;
    }

    struct MsgContext _context = {
        .senderID = iopt.senderID,
        .invokeID = iopt.invokeID,
        .traceID = iopt.traceID,
        .edgeNum = iopt.edgeNum,
    };
    struct flow_tuple f_tuple = {
        .sport = skops->local_port,
        .dport = bpf_ntohl(skops->remote_port),
        .sip = bpf_ntohl(skops->local_ip4),
        .dip = bpf_ntohl(skops->remote_ip4),
    };
    bpf_map_update_elem(&tuple_context_map, &f_tuple, &_context, BPF_ANY);
    // bpf_printk("parse: %llu\n", bpf_ktime_get_boot_ns() - t1);

    // struct tcphdr *th = skops->skb_data;
    // u32 seq = bpf_ntohl(BPF_CORE_READ(th, seq));
    // struct option_msg msg1 = {
    //     .tuple = f_tuple,
    //     .seq = seq,
    //     .tgid = 0,
    //     .timestamp = bpf_ktime_get_ns(),
    //     .type = PARSE_OPT,
    //     .option = iopt,
    // };
    // bpf_ringbuf_output(&rb, &msg1, sizeof(msg1), 0);
}

static inline void tcp_int_enable_tcp_opt_cb(struct bpf_sock_ops *skops)
{
    int cb_flags;

    cb_flags = skops->bpf_sock_ops_cb_flags |
               BPF_SOCK_OPS_WRITE_HDR_OPT_CB_FLAG |
               BPF_SOCK_OPS_PARSE_UNKNOWN_HDR_OPT_CB_FLAG |
               BPF_SOCK_OPS_PARSE_ALL_HDR_OPT_CB_FLAG;
    bpf_sock_ops_cb_flags_set(skops, cb_flags);
}

static inline void tcp_int_reserve_hdr_opt(struct bpf_sock_ops *skops, __u32 size)
{
    bpf_reserve_hdr_opt(skops, size, 0);
}

SEC("sockops")
int tcp_int(struct bpf_sock_ops *skops)
{
    
    switch (skops->op)
    {
    case BPF_SOCK_OPS_TCP_CONNECT_CB:
    case BPF_SOCK_OPS_ACTIVE_ESTABLISHED_CB:
    case BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB:
        tcp_int_enable_tcp_opt_cb(skops);
        break;
    case BPF_SOCK_OPS_HDR_OPT_LEN_CB:
        tcp_int_reserve_hdr_opt(skops, sizeof(struct tcp_int_opt));
        break;
    case BPF_SOCK_OPS_WRITE_HDR_OPT_CB:
        tcp_int_add_tcpopt(skops);
        break;
    case BPF_SOCK_OPS_PARSE_HDR_OPT_CB:
        tcp_int_process_tcpopt(skops);
        break;
    default:
        break;
    }
    return 1;
}

char LICENSE[] SEC("license") = "GPL";