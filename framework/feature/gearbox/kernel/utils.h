#ifndef __GEARBOX_UTILS_H__
#define __GEARBOX_UTILS_H__

#include "all.h"
#include "structs.h"
#include "maps.h"


#define HOST_IP         3396398369  // 202.112.237.33
// 3396398373  // 202.112.237.37

/**
 * Checks if the current component is the INGRESS component
 */
static inline bool is_ingress(u32 tgid) {
    u32 *find_tgid = bpf_map_lookup_elem(&pid_map, &tgid);
    if (!find_tgid) { 
        return false;
    }
    return *find_tgid;
}

/**
 * Retrieves the TCP socket structure from a file descriptor and a task structure.
 */
static inline struct tcp_sock *get_tsk_from_fd(int fd, struct task_struct *task) {
    // Read the struct file ** pointer `fd` from task->files->fdt->fd
    struct file **file_fd;
    file_fd = BPF_CORE_READ(task, files, fdt, fd);  

    // Read the file structure from the table `file_fd`
    struct file *file_ptr;
    bpf_probe_read(&file_ptr, sizeof(file_ptr), &(file_fd[fd]));  

    void *private_data;
    private_data = BPF_CORE_READ(file_ptr, private_data);  // Read the private data field from the file structure

    struct socket *socket = private_data;
    short int type = BPF_CORE_READ(socket, type);
    // Check if the socket type is not SOCK_STREAM
    if (type != SOCK_STREAM) {
        bpf_printk("get_tsk_from_fd() error: socket type is not SOCK_STREAM\n");
        return NULL;
    }

    struct sock *sk = BPF_CORE_READ(socket, sk);
    struct tcp_sock *tsk;
    tsk = (struct tcp_sock *)(sk);  // Cast the sock structure pointer to a tcp_sock structure pointer
    return tsk;
}

/**
 * Retrieves the metrics from struct tcp_sock.
 */
static inline void collect_metrics(struct tcp_sock* tsk, struct metrics* m) {
    m->srtt_us = BPF_CORE_READ(tsk, srtt_us);
    m->mdev_max_us = BPF_CORE_READ(tsk, mdev_max_us);
    m->rttvar_us = BPF_CORE_READ(tsk, rttvar_us);
    m->mdev_us = BPF_CORE_READ(tsk, mdev_us);
    m->bytes_sent = BPF_CORE_READ(tsk, bytes_received);
    m->bytes_received = BPF_CORE_READ(tsk, bytes_received);
    m->bytes_acked = BPF_CORE_READ(tsk, bytes_acked);
    m->delivered = BPF_CORE_READ(tsk, delivered);
    m->snd_cwnd = BPF_CORE_READ(tsk, snd_cwnd);
    m->rtt_us = BPF_CORE_READ(tsk, rcv_rtt_est.rtt_us);

    // m->packet_out = BPF_CORE_READ(tsk, packets_out);
    // m->loss_pkt_out = BPF_CORE_READ(tsk, lost_out);
    // m->lost_pkt =BPF_CORE_READ(tsk, lost);
    // m->retran_pkt_out = BPF_CORE_READ(tsk, retrans_out);
    // m->retran_pkt = BPF_CORE_READ(tsk, total_retrans);
    // m->retran_bytes = BPF_CORE_READ(tsk, bytes_retrans);
}

// TODO: 这么拷贝复制好吗？
static inline void construct_edge_at_recv(struct edge *e, struct msg_context c, struct point p2) {
    struct point p1 = {
        .trace_id = c.trace_id,
        .component_id = c.sender_id,
        .invoke_id = c.invoke_id,
    };
    e->p1 = p1;
    e->p2 = p2;
}

/**
 * Infers the I/O state for a given thread group ID and source port based on a syscall type.
 */
static inline enum IO_state infer_IO_state(u32 tgid, u16 sport, enum syscall_type flag) {
    enum IO_state current_io_s, ret_io_s = 0;

    // Check if the syscall type is neither RECV nor SEND
    if (flag != RECV_TYPE && flag != SEND_TYPE) {
        bpf_printk("infer_IO_state() error: flag value is %u, which is not one of the RECV/SEND flag\n", flag);
        ret_io_s = INFER_ERROR;
        goto return_val;
    }

    // Look up current I/O state in the IO_state_map
    // TODO: 这里更严谨的做法是用五元组
    struct tgid_sport tgid_sport_s = {.tgid = tgid, .sport = sport};
    enum IO_state *current_io_s_ptr = bpf_map_lookup_elem(&IO_state_map, &tgid_sport_s);

    // Check if the current I/O state pointer is NULL (i.e., no entry found)
    if (!current_io_s_ptr) {
        ret_io_s = (flag == RECV_TYPE) ? FIRST_RECV : FIRST_SEND;
    } else {
        // Dereference the pointer to get the current I/O state
        // 直接*_io_s == 2,会报错
        current_io_s = *current_io_s_ptr;

        // Infer the I/O state based on the stored I/O state type
        if (current_io_s == FIRST_RECV || current_io_s == NOT_FIRST_RECV) {
            ret_io_s = (flag == RECV_TYPE) ? NOT_FIRST_RECV : FIRST_SEND;
        }
        if (current_io_s == FIRST_SEND || current_io_s == NOT_FIRST_SEND) {
            ret_io_s = (flag == SEND_TYPE) ? NOT_FIRST_SEND : FIRST_RECV;
        }

        // If the current state is already an error, return an error
        if (current_io_s == INFER_ERROR) {
            bpf_printk("infer_IO_state() error: current_io_s value retrieved from IO_state_map is INFER_ERROR\n");
            ret_io_s = INFER_ERROR;
            goto return_val;
        }
        
        // Update the current I/O state in the IO_state_map
    }
    bpf_map_update_elem(&IO_state_map, &tgid_sport_s, &ret_io_s, BPF_ANY);

return_val:
    return ret_io_s;
}

/**
 * Update the invoke states
 */
static inline u16 update_invoke_state(u64 trace_id, u32 tgid, struct metrics* m) {
    struct trace_pid tp = {.trace_id = trace_id, .tgid = tgid};
    struct invoke_state invoke_state, *_invoke_state = bpf_map_lookup_elem(&invoke_state_map, &tp);

    // For the first point, set the invokeid to 1 and duration to 0
    if (!_invoke_state) {
        invoke_state.invoke_times = 1;
        m->duration = 0;
    } else {
        // For the subsequent points, increment the invokeid and calculate the duration
        bpf_probe_read(&invoke_state, sizeof(invoke_state), _invoke_state);
        invoke_state.invoke_times = invoke_state.invoke_times + 1;
        m->duration = bpf_ktime_get_boot_ns() - invoke_state.last_timestamp;
    }

    invoke_state.last_timestamp = bpf_ktime_get_boot_ns();
    bpf_map_update_elem(&invoke_state_map, &tp, &invoke_state, BPF_ANY);

    return invoke_state.invoke_times;
}

#endif // __GEARBOX_UTILS_H__