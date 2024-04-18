#include "header_option.h"

// Determines if a custom TCP option can be added to the socket's TCP segment.
// This decision is based on two main criteria:
// 1. Ensures that adding the TCP option does not cause the TCP segment to be larger than 
//    what the network can handle, potentially leading to fragmentation or other issues.
// 2. The TCP flags of the current segment (`skb_tcp_flags`) must be exactly equal 
//    to the combination of ACK and PSH flags. 
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

inline void tcp_int_enable_tcp_opt_cb(struct bpf_sock_ops *skops) {
    int cb_flags;

    // https://lore.kernel.org/bpf/20200730205736.3354304-1-kafai@fb.com/
    cb_flags = skops->bpf_sock_ops_cb_flags |
               BPF_SOCK_OPS_WRITE_HDR_OPT_CB_FLAG |
               BPF_SOCK_OPS_PARSE_UNKNOWN_HDR_OPT_CB_FLAG |
               BPF_SOCK_OPS_PARSE_ALL_HDR_OPT_CB_FLAG;
    
    bpf_sock_ops_cb_flags_set(skops, cb_flags);
}

inline void tcp_int_reserve_hdr_opt(struct bpf_sock_ops *skops, __u32 size) {
    bpf_reserve_hdr_opt(skops, size, 0);
}

inline void tcp_int_add_tcpopt(struct bpf_sock_ops *skops) {
    if (!skops_can_add_option(skops)) {
        return;
    }

    struct flow_tuple f_tuple = {
        .sport = skops->local_port,
        .dport = bpf_ntohl(skops->remote_port),
        .sip = bpf_ntohl(skops->local_ip4),
        .dip = bpf_ntohl(skops->remote_ip4),
    };

    struct MsgContext *_context = bpf_map_lookup_elem(&tuple_context_map, &f_tuple);
    struct tcp_int_opt iopt = {0};
    if (_context) {
        iopt.kind = TCP_INT_OPT_KIND;
        iopt.len = sizeof(iopt);
        iopt.senderID = _context->senderID;
        iopt.invokeID = _context->invokeID;
        iopt.traceID = _context->traceID;
        iopt.edgeNum = _context->edgeNum;
        if (bpf_store_hdr_opt(skops, &iopt, sizeof(iopt), 0) == 0) {
            bpf_map_delete_elem(&tuple_context_map, &f_tuple);
        }
    }
}

inline void tcp_int_process_tcpopt(struct bpf_sock_ops *skops) {
    struct tcp_int_opt iopt = {};
    int rv;
    iopt.kind = TCP_INT_OPT_KIND;
    rv = bpf_load_hdr_opt(skops, &iopt, sizeof(iopt), 0);
    if (rv <= 0) {
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
}
