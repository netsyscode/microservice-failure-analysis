#ifndef __GEARBOX_UTILS_H__
#define __GEARBOX_UTILS_H__

#include "all.h"
#include "structs.h"
#include "maps.h"

static u32 nr_cpus = 24;
static u32 metric_num = 2;

static inline bool is_filtered_pid(u32 tgid)
{
    u32 *find_tgid = bpf_map_lookup_elem(&pid_map, &tgid);

    // For the pid that is not in the pid_map, we do not need to collect the data
    if (!find_tgid)
    {
        return true;
    }

    // TODO: This might introduce fragmentation in traces.
    // Currently, when will the tgid be 0?
    if (tgid == 0)
    {
        bpf_printk("is_filtered_pid() error: tgid is 0, which is not likely to happen\n");
        return true;
    }

    return false;
}

static inline struct tcp_sock *get_tsk_from_fd(int fd, struct task_struct *task)
{
    // Read the struct file ** pointer `fd` from task->files->fdt->fd
    struct file **file_fd;
    file_fd = BPF_CORE_READ(task, files, fdt, fd);

    // Read the file structure from the table `file_fd`
    struct file *file_ptr;
    bpf_probe_read(&file_ptr, sizeof(file_ptr), &(file_fd[fd]));

    void *private_data;
    private_data = BPF_CORE_READ(file_ptr, private_data); // Read the private data field from the file structure

    struct socket *socket = private_data;
    short int type = BPF_CORE_READ(socket, type);
    // Check if the socket type is not SOCK_STREAM
    if (type != SOCK_STREAM)
    {
        // bpf_printk("get_tsk_from_fd() error: socket type is not SOCK_STREAM\n"); //todo DEBUG 可能不是网络套接字
        return NULL;
    }

    struct sock *sk = BPF_CORE_READ(socket, sk);
    struct tcp_sock *tsk;
    tsk = (struct tcp_sock *)(sk); // Cast the sock structure pointer to a tcp_sock structure pointer
    return tsk;
}

static int get_pmu(u32 pmu_index, u32 metric_id)
{

    u32 cpu_index = bpf_get_smp_processor_id();
    u32 key = pmu_index * nr_cpus * metric_num + metric_id * nr_cpus + cpu_index; // 24个cpu
    // u32 key = metric_id;
    // u32 key = 0;
    u64 count;
    s64 error;
    bpf_printk("get_pmu: %u cpu: %u\n", key, cpu_index);
    count = bpf_perf_event_read(&pmu_map, key);
    bpf_printk("get_pmu count: %llu\n", count);
    error = (s64)count;
    if (error <= -2 && error >= -22)
        return 0;
    return count;
}

static inline void collect_data(u32 tgid, struct tcp_sock *tsk, enum syscall_name syscall_name)
{
    // struct point p = {};
    u32 key = 0;
    struct point *p = bpf_map_lookup_elem(&percpu_data_map, &key);
    if (!p)
    {
        return;
    }

    struct tag_list *tags = bpf_map_lookup_elem(&pid_tag_map, &tgid);
    if (tags == NULL)
    {
        return;
    }

    u32 sport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_sport));
    u32 dport = bpf_ntohs(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_dport));
    // u32 copied_seq = BPF_CORE_READ(tsk, copied_seq);
    u32 skc_saddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.inet_saddr));
    u32 skc_daddr = bpf_ntohl(BPF_CORE_READ(tsk, inet_conn.icsk_inet.sk.__sk_common.skc_daddr));
    u64 bytes_received = BPF_CORE_READ(tsk, bytes_received);
    u64 bytes_sent = BPF_CORE_READ(tsk, bytes_sent);

    struct task_struct *curr_task = (struct task_struct *)bpf_get_current_task();

    // 获取当前进程的内存用量
    struct mm_struct *mm = BPF_CORE_READ(curr_task, mm);

    p->tags = *tags;

    // 可以在内核获取的
    p->tags.DstIP = skc_daddr;
    p->tags.SrcIP = skc_saddr;
    p->tags.SrcPort = sport;
    p->tags.DstPort = dport;
    p->tags.Protocol = SOCK_STREAM;

    p->metrics.RX_Bytes = bytes_sent;
    p->metrics.TX_Bytes = bytes_received;
    p->metrics.Dropped_Packets = BPF_CORE_READ(tsk, lost);
    p->metrics.RTT = BPF_CORE_READ(tsk, srtt_us);
    p->metrics.Retransmissions = BPF_CORE_READ(tsk, retrans_out);
    p->metrics.Memory_Usage = BPF_CORE_READ(mm, total_vm);
    p->metrics.Page_Faults = BPF_CORE_READ(curr_task, maj_flt);
    if (syscall_name <= 4)
    {
        p->metrics.Instructions = get_pmu(tags->PmuIndex, 0);
        p->metrics.LLC_Misses = get_pmu(tags->PmuIndex, 1);
    }else{
        p->metrics.Instructions = get_pmu(tags->PmuIndex, 0) - p->metrics.Instructions;
        p->metrics.LLC_Misses = get_pmu(tags->PmuIndex, 1) - p->metrics.LLC_Misses;
    }

    bpf_ringbuf_output(&rb, p, sizeof(*p), 0);
}

static inline void process_enter_send(struct trace_event_raw_sys_enter *ctx, enum syscall_name syscall_name)
{
    u32 tgid = bpf_get_current_pid_tgid() >> 32;
    if (is_filtered_pid(tgid))
    {
        return;
    }
    u32 sockfd = ctx->args[0];

    // Retrieve the struct tcp_sock from the socket file descriptor
    struct task_struct *curr_task = (struct task_struct *)bpf_get_current_task();
    struct tcp_sock *tsk = get_tsk_from_fd(sockfd, curr_task);
    if (tsk == NULL)
    {
        return;
    }
    if (ctx->args[2] != 165)
    {
        return;
    }
    bpf_printk("Enter send: %u\n", ctx->args[2]);
    collect_data(tgid, tsk, syscall_name);
}

static inline void process_enter_recv(struct trace_event_raw_sys_enter *ctx, enum syscall_name syscall_name)
{
    u32 tgid = bpf_get_current_pid_tgid() >> 32;
    if (is_filtered_pid(tgid))
    {
        return;
    }
    u32 sockfd = ctx->args[0];

    // Retrieve the struct tcp_sock from the socket file descriptor
    struct task_struct *curr_task = (struct task_struct *)bpf_get_current_task();
    struct tcp_sock *tsk = get_tsk_from_fd(sockfd, curr_task);
    if (tsk == NULL)
    {
        return;
    }
    struct data_args read_args = {};
    read_args.tsk = tsk;
    bpf_map_update_elem(&read_args_map, &tgid, &read_args, BPF_ANY);
}

static inline void process_exit_recv(struct trace_event_raw_sys_exit *ctx, enum syscall_name type)
{
    // Filter the system call if the pid is not in the pid_map
    u32 tgid = bpf_get_current_pid_tgid() >> 32;
    if (is_filtered_pid(tgid))
    {
        return;
    }

    // Retrieve the struct tcp_sock from the read_args_map
    struct data_args *read_args = bpf_map_lookup_elem(&read_args_map, &tgid);
    if (!read_args)
    {
        // TODO: 这可能是正常情况，因为最后删掉了read_args_map
        // bpf_printk("process_exit_recv() error: Failed to get struct tcp_sock from read_args_map: %u %u\n", type, tgid);
        return;
    }
    struct tcp_sock *tsk = read_args->tsk;
    if (tsk == NULL)
    {
        // bpf_printk("process_exit_recv() error: Failed to get struct tcp_sock from read_args_map, tsk is NULL\n"); //todo DEBUG 不是网络数据包，是文件读取
        return;
    }

    u64 buf_len = (__u64)ctx->ret;
    if (buf_len <= 2 || 4294967280 <= buf_len)
    {
        // bpf_printk("process_exit_recv() error: buf_len is %u, which is not in the range (0, 4294967280)\n", buf_len); //todo DEBUG 这个检测放到后边
        return;
    }

    bpf_printk("Exit recv: %u\n", type);

    collect_data(tgid, tsk, type);

    bpf_map_delete_elem(&read_args_map, &tgid);
}

#endif // __GEARBOX_UTILS_H__