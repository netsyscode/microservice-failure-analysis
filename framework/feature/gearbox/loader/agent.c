#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "./kernel/build/kernel.skel.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/perf_event.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include "./kernel/structs.h"
#include "./kernel/structs.h"
// sudo cgexec -g cpu:bpfgroup myapp

#define DEBUGFS "/sys/kernel/debug/tracing/"
#define MAX_ARGS 10
#define SERVER_PATH_CNT 1
#define HOST_METRIC "10.0.1.132" // agent的ip地址
#define PORT_METRIC 50010       // loader向agent传输point和指标的端口
#define MAX_EDGE_SIZE 4000      // 能承受的尚未发出去的edge数量
#define MAX_POINT_SIZE 8000      // 能承受的尚未发出去的point数量

// const char hosts_path[SERVER_PATH_CNT][20] = {"10.0.1.128", "10.0.1.128"};
// const int ports_path[SERVER_PATH_CNT] = {50000, 50001};

// path server的ip和端口号
// const char hosts_path[SERVER_PATH_CNT][20] = {"127.0.0.1", "127.0.0.1"};
// const int ports_path[SERVER_PATH_CNT] = {50000, 50001};

const char hosts_path[SERVER_PATH_CNT][20] = {"10.0.1.40"};
const int ports_path[SERVER_PATH_CNT] = {50000};

struct ring_buffer *rb = NULL;
FILE *fp = NULL;
FILE *edge_fp = NULL;
FILE *point_fp = NULL;
int SVCM_mode = 0;
__u64 edge_num = 0;
__u64 point_num = 0;

char *func_type_list[] = {
    "WRITE",
    "READ",
    "ADD_OPT",
    "PASER_OPT",
    "SENDTO",
    "RECVFROM",
    "SENDMSG",
    "RECVMSG",
    "SENDMMSG",
    "RECVMMSG",
    "WRITEV",
    "READV",
    "EDGE",
    "POINT",
};

char *L7_proto_list[] = {
    "UNKNOWN",
    "ORTHER",
    "HTTP1",
    "HTTP2",
    "TLS_HTTP1",
    "TLS_HTTP2",
    "DUBBO",
    "SOFARPC",
    "MYSQL",
    "POSTGRESQL",
    "REDIS",
    "KAFKA",
    "MQTT",
    "DNS",
    "NUM",
};

char *msg_direction_list[] = {
    "REQUEST",
    "RESPONSE",
};

bool has_c = false;
char *cgroup_path;
char send_edge[sizeof(struct edge_for_path) * MAX_EDGE_SIZE];
unsigned int send_edge_size = 0;
char send_point[sizeof(struct point) * MAX_POINT_SIZE];
unsigned int send_point_size = 0;
char send_metric[sizeof(struct metrics) * MAX_POINT_SIZE];
unsigned int send_metric_size = 0;

int open_client(char *dest_ip, int dst_port)
{
    int sockfd, new_fd;           /*cocket句柄和接受到连接后的句柄 */
    struct sockaddr_in dest_addr; /*目标地址信息*/
    struct sockaddr_in client_address;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); /*建立socket*/
    if (sockfd == -1)
    {
        printf("socket failed:%d\n", errno);
    }

    // 参数意义见上面服务器端
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dst_port);
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
    bzero(&(dest_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
    {                                         // 连接方法，传入句柄，目标地址和大小
        printf("connect failed:%d\n", errno); // 失败时可以打印errno
    }
    else
    {
        return sockfd;
    }
    close(sockfd); // 关闭socket
    return -1;
}

static void parse_arg(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "p:c:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            cgroup_path = optarg;
            has_c = true;
            printf("cgroup path: %s\n", cgroup_path);
            break;
        case 'p':
            SVCM_mode = atoi(optarg);
            break;
        }
    }
    if (!has_c)
    {
        printf("please set cgroup path\n");
        exit(-1);
    }
}

static void read_trace_pipe(void)
{
    int trace_fd;

    trace_fd = open(DEBUGFS "trace_pipe", O_RDONLY, 0);
    if (trace_fd < 0)
        return;

    while (1)
    {
        static char buf[4096];
        ssize_t sz;

        sz = read(trace_fd, buf, sizeof(buf) - 1);
        if (sz > 0)
        {
            buf[sz] = 0;
            puts(buf);
        }
    }
}

int collect(void *ctx, void *data, size_t data_sz)
{
    enum msg_type type = *((enum msg_type *)data);
    switch (type)
    {
    case SENDTO:
    case RECVFROM:
    case READ:
    case WRITE:
    case SENDMSG:
    case RECVMSG:
    case SENDMMSG:
    case RECVMMSG:
    case WRITEV:
    case READV:
    {
        // struct send_write_msg *msg = data;
        // fprintf(fp, "ts:%llu syscall:%s sip:%u dip:%u sport:%u dport:%u pid:%u \
        //         seq:%u io_state:%u context-> senderid:%llu invokeID:%u traceid:%llu buf_len::%llu\n",
        //         msg->timestamp,
        //         func_type_list[msg->type], msg->tuple.sip, msg->tuple.dip,
        //         msg->tuple.sport, msg->tuple.dport, msg->tgid, msg->seq, msg->io_s,
        //         msg->context.senderID, msg->context.invokeID, msg->context.traceID, msg->buf_len);
        break;
    }
    case ADD_OPT:
    case PARSE_OPT:
    {
        // struct option_msg *msg = data;
        // fprintf(fp, "ts:%llu type:%s sip:%u dip:%u sport:%u dport:%u seq:%u  opt->\
        //         senderid:%llu invokeID:%u traceid:%llu\n",
        //         msg->timestamp, func_type_list[msg->type], msg->tuple.sip, msg->tuple.dip,
        //         msg->tuple.sport, msg->tuple.dport, msg->seq,
        //         msg->option.senderID, msg->option.invokeID, msg->option.traceID);
        break;
    }
    case EDGE:
    {
        struct edge_msg *msg = data;
        // fprintf(edge_fp, "type:%s edge->edgeNum:%u traceid:%llu p1  componentID:%llu\
        // invokeID:%u p2 componentID:%llu invokeID:%u\n",
        //         func_type_list[msg->type], msg->e.edgeNum, msg->e.p1.traceID, msg->e.p1.componentID, msg->e.p1.invokeID,
        //         msg->e.p2.componentID, msg->e.p2.invokeID);

        struct edge_for_path e = {
            .traceID = msg->e.p1.traceID,
            .componentID1 = msg->e.p1.componentID,
            .invokeID1 = msg->e.p1.invokeID,
            .componentID2 = msg->e.p2.componentID,
            .invokeID2 = msg->e.p2.invokeID,
            .num = msg->e.edgeNum,
        };
        if (send_edge_size > MAX_EDGE_SIZE * sizeof(struct edge_for_path))
        {
            printf("edge size too small!\n");
        }
        edge_num += 1;
        if (edge_num % 20 == 0)
        {
            printf("edge_num: %llu\n", edge_num);
        }
        memcpy(send_edge + send_edge_size, &e, sizeof(struct edge_for_path));
        send_edge_size += sizeof(struct edge_for_path);
        break;
    }
    case POINT:
    {   
        struct point_msg *msg = data;
        // fprintf(point_fp, "type:%s point->traceid:%llu componentID:%llu\
        // invokeID:%u metrics-> srtt_us:%u mdev_max_us:%u rttvar_us:%u\
        // mdev_us:%u bytes_sent:%llu bytes_received:%llu bytes_acked:%llu delivered:%u\
        // snd_cwnd:%u rtt_us:%u duration:%llu\n",
        // func_type_list[msg->type], msg->p.traceID, msg->p.componentID, msg->p.invokeID,
        // msg->m.srtt_us, msg->m.mdev_max_us, msg->m.rttvar_us, msg->m.mdev_us,
        // msg->m.bytes_sent, msg->m.bytes_received, msg->m.bytes_acked, msg->m.delivered,
        // msg->m.snd_cwnd, msg->m.rtt_us, msg->m.duration);
        
        struct point p = {
            .traceID = msg->p.traceID,
            .componentID = msg->p.componentID,
            .invokeID = msg->p.invokeID,
        };
        memcpy(send_point + send_point_size, &p, sizeof(struct point));
        send_point_size += sizeof(struct point);
        struct metrics m = {
            .srtt_us = msg->m.srtt_us,
            .mdev_max_us = msg->m.mdev_max_us,
            .rttvar_us = msg->m.rttvar_us,
            .mdev_us = msg->m.mdev_us,
            .bytes_sent = msg->m.bytes_sent,
            .bytes_received = msg->m.bytes_received,
            .bytes_acked = msg->m.bytes_acked,
            .delivered = msg->m.delivered,
            .snd_cwnd = msg->m.snd_cwnd,
            .rtt_us = msg->m.rtt_us,
            .duration = msg->m.duration,
        };
        
        memcpy(send_metric + send_metric_size, &m, sizeof(struct metrics));
        //struct metrics * mp = (struct metrics*)(send_metric + send_metric_size);
        //printf("compare:%llu,%llu\n",m.duration,mp->duration);

        point_num += 1;
        if(point_num % 200 == 0){
            printf("point num: %llu\n", point_num);
        }
        if(send_point_size > MAX_POINT_SIZE * sizeof(struct point)){
            printf("point size too small!\n");
        }
        send_metric_size += sizeof(struct metrics);
        break;
    }
    }
    fflush(fp);
    fflush(edge_fp);
    fflush(point_fp);
    return 0;
}

int run()
{

    if (SVCM_mode == 0)
    {
        int err;
        int idx = 0, cgroup_fd = -1;
        struct kernel *kernel_skel;
        struct bpf_map *cgroup_map;

        kernel_skel = kernel__open();
        if (!kernel_skel)
        {
            fprintf(stderr, "Failed to open and load BPF skeleton\n");
            return -1;
        }
        /* Load & verify BPF programs */
        err = kernel__load(kernel_skel);
        if (err)
        {
            fprintf(stderr, "Failed to load and verify BPF skeleton\n");
            return -1;
        }
        cgroup_map = kernel_skel->maps.cgroup_map;
        if (!cgroup_map)
        {
            printf("Failed to find cgroup map\n");
            goto cleanup1;
        }

        struct bpf_map *pid_map = kernel_skel->maps.pid_map;
        FILE *pid_fp;
        char line[20];

        // 打开文件
        pid_fp = fopen("pids.txt", "r");

        if (pid_fp == NULL)
        {
            printf("无法打开文件\n");
            exit(EXIT_FAILURE);
        }

        // 逐行读取文件并将字符串转换为整数
        while (fgets(line, 20, pid_fp) != NULL)
        {
            __u32 pid = atoi(line);
            __u32 value = 1;
            if (bpf_map_update_elem(bpf_map__fd(pid_map), &pid, &value, BPF_ANY))
            {
                printf("Failed adding filter pid to map\n");
                goto cleanup1;
            }
        }
        // 关闭文件
        fclose(pid_fp);
        if (!cgroup_map)
        {
            printf("Failed to find cgroup map\n");
            goto cleanup1;
        }

        if (has_c)
        {
            int bpf_cgroup_fd = open(cgroup_path, O_RDONLY);
            assert(bpf_cgroup_fd);
            if (bpf_map_update_elem(bpf_map__fd(cgroup_map), &idx, &bpf_cgroup_fd, BPF_ANY))
            {
                printf("Failed adding filter cgroup to map\n");
                goto cleanup1;
            }
            else
            {
                printf("Add cgroup fd %d\n", bpf_cgroup_fd);
            }
            kernel_skel->links.tcp_int = bpf_program__attach_cgroup(kernel_skel->progs.tcp_int, bpf_cgroup_fd);
            close(bpf_cgroup_fd);
        }

        kernel__attach(kernel_skel);
        printf("load cmd bpf\n");

        // read_trace_pipe();
        rb = ring_buffer__new(bpf_map__fd(kernel_skel->maps.rb), collect, NULL, NULL);
        if (!rb)
        {
            printf("Failed to create ring buffer\n");
            goto cleanup1;
        }

        /*build client*/
        int sockfd_path_array[SERVER_PATH_CNT];
        for (int i = 0; i < SERVER_PATH_CNT; ++i)
        {
            sockfd_path_array[i] = open_client((char *)hosts_path[i], ports_path[i]);
            printf("%s,%d\n", hosts_path[i], ports_path[i]);
            if (sockfd_path_array[i] < 0)
            {
                printf("open client path failed.\n");
                return -1;
            }
        }
        int sockfd_metric = open_client(HOST_METRIC, PORT_METRIC);
        if (sockfd_metric < 0)
        {
            printf("open client metric failed.\n");
            return -1;
        }

        /* Process events */

        while (1)
        {
            err = ring_buffer__poll(rb, 100 /* timeout, ms */);
            /* Ctrl-C will cause -EINTR */
            if (err == -EINTR)
            {
                err = 0;
                break;
            }
            if (err < 0)
            {
                printf("Error polling ring buffer: %d\n", err);
                break;
            }

            if (send_point_size)
            {
                for (int i = 0; i < send_point_size/sizeof(struct point); i ++)
                {
                    //struct point* p = (struct point*)(send_point + i);
                    //printf("%llu,%llu,%u\n",p->traceID,p->componentID,p->invokeID);
                    write(sockfd_metric, send_point + i * sizeof(struct point), sizeof(struct point));
                    //char buf2[6];
                    //read(sockfd_metric, buf2, 5);
                    //buf2[5] = '\0';
                    //printf("Receive2: %s\n", buf2);
                    
                    //struct metrics * m = (struct metrics*)(send_metric + i);
                    //printf("%llu\n",m->duration);

                    write(sockfd_metric, send_metric + i * sizeof(struct metrics), sizeof(struct metrics));
                    //char buf3[6];
                    //read(sockfd_metric, buf3, 5);
                    //buf3[5] = '\0';
                    //printf("Receive3: %s\n", buf3);
                }
                send_point_size = 0;
                send_metric_size = 0;
            }

            if (send_edge_size)
            {
                for (int i = 0; i < send_edge_size; i += sizeof(struct edge_for_path))
                {
                    struct edge_for_path *e = (struct edge_for_path *)(send_edge + i);
                    int id = e->traceID % SERVER_PATH_CNT;
                    // printf("%u\n",e->num);
                    write(sockfd_path_array[id], send_edge + i, sizeof(struct edge_for_path));
                    //char buf1[6];
                    //read(sockfd_path_array[id], buf1, 5);
                    //buf1[5] = '\0';
                    // printf("Receive1: %s\n", buf1);
                }

                send_edge_size = 0;
            }
        }
    cleanup1:
        kernel__destroy(kernel_skel);
        ring_buffer__free(rb);
        // bpf_link__detach(kernel_skel->links.enter_write);
        fclose(fp);
    }
    return 0;
}

int main(int argc, char **argv)
{

    parse_arg(argc, argv);
    fp = fopen("./log/log1.txt", "a+");
    edge_fp = fopen("./log/edge_log1.txt", "a+");
    point_fp = fopen("./log/point_log1.txt", "a+");
    int err = run();
    return 0;
}