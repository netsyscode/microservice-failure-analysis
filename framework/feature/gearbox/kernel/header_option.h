#ifndef __GEARBOX_HEADER_OPTION_H__
#define __GEARBOX_HEADER_OPTION_H__

#include "utils.h"
#include "structs.h"
#include "maps.h"

#define TCP_FLAG_PSH  0x08
#define TCP_FLAG_ACK  0x10

// Custom TCP Option Kind
#define TCP_INT_OPT_KIND 0x88

/**
 *  Determines if a custom TCP option can be added to the socket's TCP segment.
 * This decision is based on two main criteria:
 * 1. Ensures that adding the TCP option does not cause the TCP segment to be larger than 
 * what the network can handle, potentially leading to fragmentation or other issues.
 * 2. The TCP flags of the current segment (`skb_tcp_flags`) must be exactly equal 
 * to the combination of ACK and PSH flags.  
 */
// TODO: This might introduce fragmentation in traces.
static inline bool skops_can_add_option(struct bpf_sock_ops *skops) {
    // https://elixir.bootlin.com/linux/latest/source/include/linux/tcp.h
    // Check if the sum of the current socket buffer length and the size of the TCP option
    // exceeds the maximum segment size (skops->mss_cache).
    if (skops->skb_len + sizeof(struct tcp_int_opt) > skops->mss_cache) {
        return false;
    }

    // https://datatracker.ietf.org/doc/html/rfc9293#name-header-format
    // Check if the TCP segment's flags are exactly set to ACK and PSH.
    if (skops->skb_tcp_flags != (TCP_FLAG_ACK | TCP_FLAG_PSH)) {
        return false;
    }

    return true;
}

/**
 * Set callback flags for eBPF-based TCP header options processing.
 */
static inline void tcp_int_enable_tcp_opt_cb(struct bpf_sock_ops *skops) {
    int cb_flags;

    // https://lore.kernel.org/bpf/20200730205736.3354304-1-kafai@fb.com/
    cb_flags = skops->bpf_sock_ops_cb_flags |
               BPF_SOCK_OPS_WRITE_HDR_OPT_CB_FLAG |
               BPF_SOCK_OPS_PARSE_UNKNOWN_HDR_OPT_CB_FLAG |
               BPF_SOCK_OPS_PARSE_ALL_HDR_OPT_CB_FLAG;
    
    bpf_sock_ops_cb_flags_set(skops, cb_flags);
}

/**
 * Reserves space for custom TCP header options which is later injected by tcp_int_add_tcpopt().
 */
static inline void tcp_int_reserve_hdr_opt(struct bpf_sock_ops *skops, __u32 size) {
    bpf_reserve_hdr_opt(skops, size, 0);
}

/**
 * Attempts to inject a custom TCP option to the socket.
 * This function checks if the option can be added and, 
 * if so, retrieves struct msg_context with struct flow_tuple from flow_context_map, 
 * and inject the option in the TCP header.
 * Upon successful storage, the relevant flow context is removed from the flow_context_map.
 */
static inline void tcp_int_add_tcpopt(struct bpf_sock_ops *skops) {
    if (!skops_can_add_option(skops)) {
        return;
    }

    struct flow_tuple f_tuple = {
        .sport = skops->local_port,
        .dport = bpf_ntohl(skops->remote_port),
        .sip = bpf_ntohl(skops->local_ip4),
        .dip = bpf_ntohl(skops->remote_ip4),
    };

    struct msg_context *_context = bpf_map_lookup_elem(&flow_context_map, &f_tuple);
    if (!_context) {
        bpf_printk("tcp_int_add_tcpopt() error: Failed to get msg_context from flow_context_map\n");
    }

    struct tcp_int_opt iopt = {0};
    iopt.kind = TCP_INT_OPT_KIND;
    iopt.len = sizeof(iopt);
    iopt.sender_id = _context->sender_id;
    iopt.invoke_id = _context->invoke_id;
    iopt.trace_id = _context->trace_id;

    long ret;
    ret = bpf_store_hdr_opt(skops, &iopt, sizeof(iopt), 0);
    if (ret != 0) {
        bpf_printk("tcp_int_add_tcpopt() error: Failed to inject TCP option, error code %ld\n", ret);
        return;
    }

    bpf_map_delete_elem(&flow_context_map, &f_tuple);
}

/**
 * Processes TCP options injected by tcp_int_add_tcpopt() from the socket.
 * This function loads the custom TCP option from the header, if present,
 * and updates flow_context_map with the struct msg_context encapsulated in the option.
 */
static inline void tcp_int_process_tcpopt(struct bpf_sock_ops *skops) {
    struct tcp_int_opt iopt = {};
    long ret;

    iopt.kind = TCP_INT_OPT_KIND;
    ret = bpf_load_hdr_opt(skops, &iopt, sizeof(iopt), 0);
    if (ret <= 0) {
        bpf_printk("tcp_int_process_tcpopt() error: Failed to load TCP option, error code %ld\n", ret);
        return;
    }

    struct msg_context context = {
        .sender_id = iopt.sender_id,
        .invoke_id = iopt.invoke_id,
        .trace_id = iopt.trace_id,
    };

    struct flow_tuple f_tuple = {
        .sport = skops->local_port,
        .dport = bpf_ntohl(skops->remote_port),
        .sip = bpf_ntohl(skops->local_ip4),
        .dip = bpf_ntohl(skops->remote_ip4),
    };

    bpf_map_update_elem(&flow_context_map, &f_tuple, &context, BPF_ANY);
}

#endif // __GEARBOX_HEADER_OPTION_H__