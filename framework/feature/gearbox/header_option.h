#ifndef __LIB_HEADER_OPTION_H__
#define __LIB_HEADER_OPTION_H__

#include "utils.h"

#define TCP_FLAG_PSH  0x08
#define TCP_FLAG_ACK  0x10

inline void tcp_int_add_tcpopt(struct bpf_sock_ops *skops);
inline void tcp_int_process_tcpopt(struct bpf_sock_ops *skops);
inline void tcp_int_enable_tcp_opt_cb(struct bpf_sock_ops *skops);
inline void tcp_int_reserve_hdr_opt(struct bpf_sock_ops *skops, __u32 size);

#endif // __LIB_HEADER_OPTION_H__