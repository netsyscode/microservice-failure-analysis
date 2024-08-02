#include "all.h"
#include "utils.h"

SEC("tracepoint/syscalls/sys_enter_read")
int enter_read(struct trace_event_raw_sys_enter *ctx) {
    process_enter_recv(ctx, READ);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_read")
int exit_read(struct trace_event_raw_sys_exit *ctx) {
    process_exit_recv(ctx, READ);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_recvmsg")
int enter_recvmsg(struct trace_event_raw_sys_enter *ctx) {
    process_enter_recv(ctx, RECVMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_recvmsg")
int exit_recvmsg(struct trace_event_raw_sys_exit *ctx) {
    process_exit_recv(ctx, RECVMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_recvmmsg")
int enter_recvmmsg(struct trace_event_raw_sys_enter *ctx) {
    process_enter_recv(ctx, RECVMMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_recvmmsg")
int exit_recvmmsg(struct trace_event_raw_sys_exit *ctx) {
    process_exit_recv(ctx, RECVMMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_readv")
int enter_readv(struct trace_event_raw_sys_enter *ctx) {
    process_enter_recv(ctx, READV);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_readv")
int exit_readv(struct trace_event_raw_sys_exit *ctx) {
    process_exit_recv(ctx, READV);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_recvfrom")
int enter_recvfrom(struct trace_event_raw_sys_enter *ctx) {
    process_enter_recv(ctx, RECVFROM);
    return 0;
}

SEC("tracepoint/syscalls/sys_exit_recvfrom")
int exit_recvfrom(struct trace_event_raw_sys_exit *ctx) {
    process_exit_recv(ctx, RECVFROM);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_write")
int enter_write(struct trace_event_raw_sys_enter *ctx) {
    process_enter_send(ctx, WRITE);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_sendmsg")
int enter_sendmsg(struct trace_event_raw_sys_enter *ctx) {
    process_enter_send(ctx, SENDMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_sendmmsg")
int enter_sendmmsg(struct trace_event_raw_sys_enter *ctx) {
    process_enter_send(ctx, SENDMMSG);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_writev")
int enter_writev(struct trace_event_raw_sys_enter *ctx) {
    process_enter_send(ctx, WRITEV);
    return 0;
}

SEC("tracepoint/syscalls/sys_enter_sendto")
int enter_sendto(struct trace_event_raw_sys_enter *ctx) {
    process_enter_send(ctx, SENDTO);
    return 0;
}

char LICENSE[] SEC("license") = "GPL";