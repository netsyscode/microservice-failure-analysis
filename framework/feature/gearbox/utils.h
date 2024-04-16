#ifndef __LIB_UTILS_H_
#define __LIB_UTILS_H_

#include "all.h"
#include "structs.h"
#include "map.h"
#define TCP_INT_OPT_KIND 0x88
#define INGRESS_TGID 3865781

#define INGRESS

const volatile bool filter_cgroup = true;
const volatile bool filter_sshd = false;

static __inline void collect_metrics(struct tcp_sock* tsk, struct metrics* m){
    m->srtt_us = BPF_CORE_READ(tsk, srtt_us);
    m->mdev_max_us = BPF_CORE_READ(tsk, mdev_max_us);
    m->rttvar_us = BPF_CORE_READ(tsk, rttvar_us);
    m->mdev_us = BPF_CORE_READ(tsk, mdev_us);
    m->bytes_sent = BPF_CORE_READ(tsk, bytes_received);
    m->bytes_received = BPF_CORE_READ(tsk, bytes_received);
    m->bytes_acked = BPF_CORE_READ(tsk, bytes_acked);
    m->delivered = BPF_CORE_READ(tsk,delivered);
    m->snd_cwnd = BPF_CORE_READ(tsk,snd_cwnd);
    m->rtt_us = BPF_CORE_READ(tsk,rcv_rtt_est.rtt_us);

    // m->packet_out = BPF_CORE_READ(tsk,packets_out);
    // m->loss_pkt_out = BPF_CORE_READ(tsk,lost_out);
    // m->lost_pkt =BPF_CORE_READ(tsk,lost);
    // m->retran_pkt_out = BPF_CORE_READ(tsk,retrans_out);
    // m->retran_pkt = BPF_CORE_READ(tsk,total_retrans);
    // m->retran_bytes = BPF_CORE_READ(tsk,bytes_retrans);
}

static inline struct tcp_sock *get_tsk_from_fd(int fd, struct task_struct *task)
{
    struct file **file_fd;
    file_fd = BPF_CORE_READ(task, files, fdt, fd);
    struct file *file_ptr;
    bpf_probe_read(&file_ptr, sizeof(file_ptr), &(file_fd[fd]));
    void *private_data;
    private_data = BPF_CORE_READ(file_ptr, private_data);
    struct socket *socket;
    socket = private_data;
    short int type = BPF_CORE_READ(socket, type);
    if (type != SOCK_STREAM)
    {
        return NULL;
    }
    struct sock *sk = BPF_CORE_READ(socket, sk);
    // return sk;
    struct tcp_sock *tsk;
    tsk = (struct tcp_sock *)(sk);
    return tsk;
}

struct sock *get_sk_from_fd(int fd, struct task_struct *task)
{
    struct file **file_fd;
    file_fd = BPF_CORE_READ(task, files, fdt, fd);
    struct file *file_ptr;
    bpf_probe_read(&file_ptr, sizeof(file_ptr), &(file_fd[fd]));
    void *private_data;
    private_data = BPF_CORE_READ(file_ptr, private_data);
    struct socket *socket;
    socket = private_data;
    struct sock *sk = BPF_CORE_READ(socket, sk);
    return sk;
}


static __inline void get_edge_from_recv(struct edge *e, struct MsgContext c, struct point p2)
{
    e->p2 = p2;
    struct point p1 = {
        .traceID = c.traceID,
        .componentID = c.senderID,
        .invokeID = c.invokeID,
    };
    e->p1 = p1;
}

// 推断该pid在 recv msg或send msg时,是不是第一次recv/send调用
// flag = 0代表 recv, flag = 1代表 send
static __inline enum IO_state infer_IO_state(u32 tgid, u16 sport, u32 flag)
{
    struct tgid_sport tgid_sport_s = {.tgid = tgid, .sport = sport};
    enum IO_state *_io_s = bpf_map_lookup_elem(&IO_state_map, &tgid_sport_s);
    enum IO_state io_s, io_s2 = 0;
    if (flag == 1)
    {
        if (!_io_s)
        {
            io_s2 = FIRST_WRITE;
        }
        else
        {
            io_s = *_io_s;
            if (io_s == FIRST_READ || io_s == NOT_FIRST_READ)
            {
                io_s2 = FIRST_WRITE;
            }
            else if (io_s == FIRST_WRITE)
            {
                io_s2 = NOT_FIRST_WRITE;
            }
        }
        bpf_map_update_elem(&IO_state_map, &tgid_sport_s, &io_s2, BPF_ANY);
        return io_s2;
    }
    else if (flag == 0)
    {
        if (!_io_s)
        {
            io_s2 = FIRST_READ;
        }
        else
        {
            io_s = *_io_s;
            if (io_s == FIRST_WRITE || io_s == NOT_FIRST_WRITE) // 这里不能直接*_io_s == 2,会报错
            {
                io_s2 = FIRST_READ;
            }
            else if (io_s == FIRST_READ)
            {
                io_s2 = NOT_FIRST_READ;
            }
        }
        bpf_map_update_elem(&IO_state_map, &tgid_sport_s, &io_s2, BPF_ANY);
        return io_s2;
    }
    return 0;
}

static __inline u16 update_invoke_id(u64 traceid, u32 tgid, struct metrics* m)
{
    struct TracePID tp = {.traceID = traceid, .tgid = tgid};
    struct invoke_point *invoke_p = bpf_map_lookup_elem(&invoke_seq_map, &tp);
    struct invoke_point _invoke_p;
    if (!invoke_p)
    {
        _invoke_p.invokeid = 1;
        _invoke_p.timestamp = bpf_ktime_get_boot_ns();
        bpf_map_update_elem(&invoke_seq_map, &tp, &_invoke_p, BPF_ANY);
        m->duration = 0;
        return _invoke_p.invokeid;
    }
    else
    {
        bpf_probe_read(&_invoke_p, sizeof(_invoke_p), invoke_p);
        _invoke_p.invokeid = _invoke_p.invokeid + 1;
        m->duration = bpf_ktime_get_boot_ns() - _invoke_p.timestamp;
        _invoke_p.timestamp = bpf_ktime_get_boot_ns();
        bpf_map_update_elem(&invoke_seq_map, &tp, &_invoke_p, BPF_ANY);
        return _invoke_p.invokeid;
    }
}

// 在send的时候,把当前组件最近收到的context,绑定到要发送的seq上,
// 以便在下边经过add option的时候,根据seq找到要插入的context
// 还需要判断send的dstip是不是trace的结束
static __inline void attach_context_to_tuple(u32 tgid, enum IO_state io_s, struct tcp_sock *tsk)
{
    u32 skc_daddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_daddr));
    u16 sport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_sport));
    u16 dport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_dport));
    u32 skc_saddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_saddr));
    struct MsgContext context = {0};
    struct MsgContext *_context = bpf_map_lookup_elem(&pid_context_map, &tgid);
    struct metrics m;
    collect_metrics(tsk, &m);


    if (_context)
    {
        bpf_probe_read(&context, sizeof(context), _context);

        if (io_s == FIRST_WRITE)
        {

            u16 invokeid = update_invoke_id(context.traceID, tgid, &m);
            struct point p2 = {
                .componentID = ((u64)HOST_IP << 32) + tgid,
                .invokeID = invokeid,
                .traceID = context.traceID,
            };
            struct point_msg p_msg = {.type = POINT, .p = p2, .m = m};

#ifdef INGRESS
            u32 key = 1;
            struct flow_tuple *client_tuple = bpf_map_lookup_elem(&client_ip_map, &key);
            if (client_tuple)
            {
                struct point client = {.componentID = 0, .traceID = context.traceID, .invokeID = 0};
                struct edge edge_s = {.p1 = p2, .p2 = client, .edgeNum = 0};
                struct edge_msg edge_msg_s = {.type = EDGE, .e = edge_s};
                if (client_tuple->sip == skc_saddr &&
                    client_tuple->dip == skc_daddr &&
                    client_tuple->sport == sport &&
                    client_tuple->dport == dport)
                {
                    bpf_map_delete_elem(&client_ip_map, &key);
                    bpf_ringbuf_output(&rb, &p_msg, sizeof(p_msg), 0);
                    bpf_ringbuf_output(&rb, &edge_msg_s, sizeof(edge_msg_s), 0);
                    bpf_map_delete_elem(&now_traceID_map, &key);
                }
                else
                {
                    u64 *now_traceID = bpf_map_lookup_elem(&now_traceID_map, &key);
                    if (!now_traceID)
                    {
                        return;
                    }
                    else if (*now_traceID != context.traceID)
                    {
                        return;
                    }
#endif
                    struct flow_tuple f_tuple = {
                        .sip = skc_saddr,
                        .dip = skc_daddr,
                        .sport = sport,
                        .dport = dport,
                    };
                    context.invokeID = invokeid;
                    context.senderID = ((u64)HOST_IP << 32) + tgid;
                    bpf_ringbuf_output(&rb, &p_msg, sizeof(p_msg), 0);
                    bpf_map_update_elem(&tuple_context_map, &f_tuple, &context, BPF_ANY);
#ifdef INGRESS
                }
            }
#endif
        }
    }
}

static __inline void process_enter_send(struct trace_event_raw_sys_enter *ctx, enum msg_type type)
{
    // u64 t1 = bpf_ktime_get_boot_ns();
    u32 tgid = bpf_get_current_pid_tgid() >> 32;
    u32 *find_tgid = bpf_map_lookup_elem(&pid_map, &tgid);
    if (!find_tgid)
    {
        return;
    }
    if (tgid == 0)
    {
        return;
    }

    u32 sockfd = ctx->args[0];
    struct task_struct *curr_task = (struct task_struct *)bpf_get_current_task();

    struct tcp_sock *tsk = get_tsk_from_fd(sockfd, curr_task);
    if (tsk == NULL)
    {
        return;
    }

    u16 dport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_dport));
    u16 sport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_sport));
    u32 write_seq = BPF_CORE_READ(tsk, write_seq);

    if (sport == 0 || dport == 0 || write_seq == 0)
    {
        return;
    }

    enum IO_state io_s = infer_IO_state(tgid, sport, 1);
    attach_context_to_tuple(tgid, io_s, tsk);

    // bpf_printk("enter send: %llu\n", bpf_ktime_get_boot_ns() - t1);

//     if (tgid != 1125110)
//     {
//         return;
//     }
//     u32 skc_saddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_saddr));
//     u32 skc_daddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_daddr));
//     struct flow_tuple tuple1 = {
//         .sport = sport,
//         .dport = dport,
//         .sip = skc_saddr,
//         .dip = skc_daddr,
//     };
//     struct send_write_msg msg1 = {
//         .tuple = tuple1,
//         .seq = write_seq,
//         .tgid = tgid,
//         .type = type,
//         .flag = false,
//         .buf_len = (__u64)ctx->args[2],
//         .io_s = io_s,
//     };

//     __u32 buf_size = sizeof(msg1);
//     bpf_ringbuf_output(&rb, &msg1, buf_size, 0);


}

static __inline void process_enter_recv(struct trace_event_raw_sys_enter *ctx, enum msg_type type)
{
    // u64 t1 = bpf_ktime_get_boot_ns();
    u32 tgid = bpf_get_current_pid_tgid() >> 32;
    u32 *find_tgid = bpf_map_lookup_elem(&pid_map, &tgid);
    if (!find_tgid)
    {
        return;
    }
    if (tgid == 0)
    {
        return;
    }

    u32 sockfd = ctx->args[0];
    struct task_struct *curr_task = (struct task_struct *)bpf_get_current_task();
    struct tcp_sock *tsk = get_tsk_from_fd(sockfd, curr_task);
    if (tsk == NULL)
    {
        return;
    }
    // u32 skc_saddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_saddr));
    // u32 skc_daddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_daddr));

    u32 dport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_dport));
    u32 sport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_sport));
    u32 copied_seq = BPF_CORE_READ(tsk, copied_seq);
    if (sport == 0 || dport == 0 || copied_seq == 0)
    {
        return;
    }

    struct data_args read_args = {};
    read_args.tsk = tsk;
    bpf_map_update_elem(&read_args_map, &tgid, &read_args, BPF_ANY);

    // bpf_printk("enter recv: %llu\n", bpf_ktime_get_boot_ns() - t1);

    // struct send_write_msg msg1 = {
    //     .tuple = {
    //         .sport = sport,
    //         .dport = dport,
    //         .sip = skc_saddr,
    //         .dip = skc_daddr,
    //     },
    //     .seq = copied_seq,
    //     .tgid = tgid,
    //     .type = type,
    //     .flag = true,
    // };
    // if (tgid != 1125110)
    // {
    //     return;
    // }
    // bpf_ringbuf_output(&rb, &msg1, sizeof(msg1), 0);
}

static inline void process_exit_recv(struct trace_event_raw_sys_exit *ctx, enum msg_type type)
{
    // u64 t1 = bpf_ktime_get_boot_ns();
    u32 tgid = bpf_get_current_pid_tgid() >> 32;
    u32 *find_tgid = bpf_map_lookup_elem(&pid_map, &tgid);
    if (!find_tgid || tgid == 0)
    {
        return;
    }

    struct data_args *read_args = bpf_map_lookup_elem(&read_args_map, &tgid);

    if (read_args)
    {
        u32 buf_len = (__u64)ctx->ret;
        if (0 < buf_len && buf_len < 4294967280)
        {
            struct tcp_sock *tsk = read_args->tsk;
            u32 skc_daddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_daddr));
            u16 sport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_sport));
            u32 skc_saddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_saddr));
            u32 dport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_dport));
            struct flow_tuple f_tuple = {
                .sport = sport,
                .dport = dport,
                .sip = skc_saddr,
                .dip = skc_daddr,
            };
            enum IO_state io_s = infer_IO_state(tgid, sport, 0);

            struct MsgContext context = {0};
            struct MsgContext *_context = bpf_map_lookup_elem(&tuple_context_map, &f_tuple);

            if (io_s == FIRST_READ)
            {
                struct metrics m;
                collect_metrics(tsk, &m);
#ifdef INGRESS
                if (tgid == INGRESS_TGID)
                {
                    u32 key = 1;
                    struct flow_tuple *find_client = bpf_map_lookup_elem(&client_ip_map, &key);
                    if (!find_client)
                    { // 表示上一个trace已经结束
                        bpf_map_update_elem(&client_ip_map, &key, &f_tuple, BPF_ANY);
                        context.traceID = ((u64)HOST_IP << 32) + (bpf_ktime_get_ns() & 0xFFFFFFFF);
                        context.senderID = 0;
                        context.invokeID = 0;
                        context.edgeNum = 1;

                        bpf_map_update_elem(&now_traceID_map, &key, &context.traceID, BPF_ANY);
                        bpf_map_update_elem(&pid_context_map, &tgid, &context, BPF_ANY);

                        struct point p2 = {
                            .componentID = ((u64)HOST_IP << 32) + tgid,
                            .traceID = context.traceID,
                            .invokeID = update_invoke_id(context.traceID, tgid, &m),
                        };
                        struct edge edge_s = {0};
                        get_edge_from_recv(&edge_s, context, p2);

                        edge_s.edgeNum = context.edgeNum;
                        struct edge_msg edge_msg_s = {.type = EDGE, .e = edge_s};
                        struct point_msg p_msg = {.type = POINT, .p = p2, .m = m};

                        bpf_ringbuf_output(&rb, &edge_msg_s, sizeof(edge_msg_s), 0);
                        bpf_ringbuf_output(&rb, &p_msg, sizeof(p_msg), 0);
                    }
                }
#endif
                if (_context)
                {
#ifdef INGRESS
                    u32 key = 1;
                    u64 *now_traceID = bpf_map_lookup_elem(&now_traceID_map, &key);
                    if (!now_traceID)
                    {
                        bpf_map_delete_elem(&pid_context_map, &tgid);
                        return;
                    }
#endif
                    bpf_probe_read(&context, sizeof(context), _context);
                    struct point p2 = {
                        .componentID = ((u64)HOST_IP << 32) + tgid,
                        .traceID = context.traceID,
                        .invokeID = update_invoke_id(context.traceID, tgid, &m),
                    };
                    struct edge edge_s = {0};
                    get_edge_from_recv(&edge_s, context, p2);
                    struct edge_msg edge_msg_s = {.type = EDGE, .e = edge_s};
                    struct point_msg p_msg = {.type = POINT, .p = p2, .m = m};
                    bpf_ringbuf_output(&rb, &edge_msg_s, sizeof(edge_msg_s), 0);
                    bpf_ringbuf_output(&rb, &p_msg, sizeof(p_msg), 0);
                    bpf_map_update_elem(&pid_context_map, &tgid, &context, BPF_ANY);
                }
            }
            

        //     // if (tgid == 1125110)
        //     // {
        //     //     struct send_write_msg msg1 = {
        //     //         .tuple = f_tuple,
        //     //         .tgid = tgid,
        //     //         .type = type,
        //     //         .flag = true,
        //     //         .io_s = io_s,
        //     //         // .context = context,
        //     //     };
        //     //     bpf_ringbuf_output(&rb, &msg1, sizeof(msg1), 0);
        //     // }
        }
        bpf_map_delete_elem(&read_args_map, &tgid);
    }
     // bpf_printk("exit recv: %llu\n", bpf_ktime_get_boot_ns() - t1);
    
}

#endif