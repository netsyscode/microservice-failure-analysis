// +build ignore

// modified from pixie and datadog repos
// combines uprobe for http/2.0 and uprobe for tls encrpytion
// modified by Yunxi Shen

#include <uapi/linux/ptrace.h>

#define MAX_SIZE 400

#define HEADER_FIELD_STR_SIZE 128
#define MAX_HEADER_COUNT 64

struct tls_data_args_t {
    const char* buf;
};

struct tls_ex_data_args_t {
    const char* buf;
    size_t *buf_size;
};

enum traffic_direction_t {
    kEgress,
    kIngress,
};

struct data_event_t {
    uint64_t timestamp_ns;
    uint32_t pid;
    enum traffic_direction_t direction;
    char msg [MAX_SIZE];
};


struct header_field_t {
  int32_t size;
  char msg[HEADER_FIELD_STR_SIZE];
};

struct go_grpc_http2_header_event_t {
  struct header_field_t name;
  struct header_field_t value;
};

// This matches the golang string object memory layout. Used to help read golang string objects in BPF code.
struct gostring {
  const char* ptr;
  int64_t len;
};


BPF_HASH(tls_write_args_map, uint64_t, struct tls_data_args_t);
BPF_HASH(tls_read_args_map, uint64_t, struct tls_data_args_t);

BPF_HASH(tls_write_ex_args_map, uint64_t, struct tls_ex_data_args_t);
BPF_HASH(tls_read_ex_args_map, uint64_t, struct tls_ex_data_args_t);

BPF_PERF_OUTPUT(data_events);

BPF_PERCPU_ARRAY(pid, int, 1);

BPF_PERF_OUTPUT(go_http2_header_events);

static inline void process_data(struct pt_regs* ctx, uint64_t id, size_t bytes_count, enum traffic_direction_t direction, const char* buf) {
    if (buf == NULL) return;
    if (PT_REGS_RC(ctx) <= 0) return;

    int key = 0;
    int *pidToAllow = pid.lookup(&key);
    if (pidToAllow != NULL && *pidToAllow != -1) {
        bpf_trace_printk("%d\n", *pidToAllow);
        const int current_pid = id >> 32;
        if (current_pid != *pidToAllow) return;
    }

    struct data_event_t event = {};
    event.timestamp_ns = bpf_ktime_get_ns();
    event.direction = direction;
    event.pid = id >> 32;
    int i;
    for (i = 0; buf[i] != '\0'; ++i) {
        if (i < MAX_SIZE - 1) {
            event.msg[i] = buf[i];
        }
        else {
            // Ensure we don't write past the end of `msg`
            break;
        }
    }
    // size_t msg_size = bytes_count < MAX_SIZE ? bytes_count : MAX_SIZE;
    // bpf_probe_read(&event.msg, msg_size, buf);
    data_events.perf_submit(ctx, &event, sizeof(struct data_event_t));
}

int probe_entry_ssl_read(struct pt_regs *ctx) {
    uint64_t id = bpf_get_current_pid_tgid();
    struct tls_data_args_t read_args = {};
    read_args.buf = (char*)PT_REGS_PARM2(ctx);
    tls_read_args_map.update(&id, &read_args);
    return 0;
}

int probe_ret_ssl_read(struct pt_regs *ctx) {
    uint64_t id = bpf_get_current_pid_tgid();
    struct tls_data_args_t* read_args = tls_read_args_map.lookup(&id);
    if (read_args != NULL) {
        tls_read_args_map.delete(&id);
        process_data(ctx, id, PT_REGS_RC(ctx), kIngress, read_args->buf);
    }
    return 0;
}

int probe_entry_ssl_write(struct pt_regs *ctx) {
    uint64_t id = bpf_get_current_pid_tgid();
    struct tls_data_args_t write_args = {};
    write_args.buf = (const char *) PT_REGS_PARM2(ctx);
    tls_write_args_map.update(&id, &write_args);
    return 0;
}

int probe_ret_ssl_write(struct pt_regs *ctx) {
    uint64_t id = bpf_get_current_pid_tgid();
    struct tls_data_args_t* write_args = tls_write_args_map.lookup(&id);
    if (write_args != NULL) {
        tls_write_args_map.delete(&id);
        process_data(ctx, id, PT_REGS_RC(ctx), kEgress, write_args->buf);
    }
    return 0;
}

int probe_entry_ssl_read_ex(struct pt_regs *ctx) {
    uint64_t id = bpf_get_current_pid_tgid();
    struct tls_ex_data_args_t read_args = {};
    read_args.buf = (char*)PT_REGS_PARM2(ctx);
    read_args.buf_size = (size_t*)PT_REGS_PARM4(ctx);
    tls_read_ex_args_map.update(&id, &read_args);
    return 0;
}

int probe_ret_ssl_read_ex(struct pt_regs *ctx) {
    uint64_t id = bpf_get_current_pid_tgid();
    struct tls_ex_data_args_t* read_args = tls_read_ex_args_map.lookup(&id);
    if (read_args != NULL) {
        tls_read_ex_args_map.delete(&id);
        size_t bytes_count = 0;
        bpf_probe_read_user(&bytes_count, sizeof(bytes_count), read_args->buf_size);
        process_data(ctx, id, bytes_count, kIngress, read_args->buf);
    }
    return 0;
}

int probe_entry_ssl_write_ex(struct pt_regs *ctx) {
    uint64_t id = bpf_get_current_pid_tgid();
    struct tls_ex_data_args_t write_args = {};
    write_args.buf = (const char *)PT_REGS_PARM2(ctx);
    write_args.buf_size = (size_t*)PT_REGS_PARM4(ctx);
    tls_write_ex_args_map.update(&id, &write_args);
    return 0;
}

int probe_ret_ssl_write_ex(struct pt_regs *ctx) {
    uint64_t id = bpf_get_current_pid_tgid();

    struct tls_ex_data_args_t* write_args = tls_write_ex_args_map.lookup(&id);
    if (write_args != NULL) {
        tls_write_ex_args_map.delete(&id);
        size_t bytes_count = 0;
        bpf_probe_read_user(&bytes_count, sizeof(bytes_count), write_args->buf_size);
        process_data(ctx, id, bytes_count, kEgress, write_args->buf);
    }
    return 0;
}



// Copy the content of a hpack.HeaderField object into header_field_t object.
static void copy_header_field(struct header_field_t* dst, const void* header_field_ptr) {
  struct gostring str = {};
  bpf_probe_read(&str, sizeof(str), header_field_ptr);
  if (str.len <= 0) {
    dst->size = 0;
    return;
  }
  dst->size = str.len < HEADER_FIELD_STR_SIZE ? str.len : HEADER_FIELD_STR_SIZE;
  bpf_probe_read(dst->msg, HEADER_FIELD_STR_SIZE, str.ptr);
}

// Copies and submits content of an array of hpack.HeaderField to perf buffer.
static void submit_headers(struct pt_regs* ctx, void* fields_ptr, int64_t fields_len) {
  // Size of the golang hpack.HeaderField struct.
  const size_t header_field_size = 40;
  struct go_grpc_http2_header_event_t event = {};
  for (size_t i = 0; i < MAX_HEADER_COUNT; ++i) {
    if (i >= fields_len) {
      continue;
    }
    const void* header_field_ptr = fields_ptr + i * header_field_size;
    copy_header_field(&event.name, header_field_ptr);
    copy_header_field(&event.value, header_field_ptr + 16);
    go_http2_header_events.perf_submit(ctx, &event, sizeof(event));
  }
}

// Signature: func (l *loopyWriter) writeHeader(streamID uint32, endStream bool, hf []hpack.HeaderField, onWrite func())
int probe_loopy_writer_write_header(struct pt_regs* ctx) {
  void* fields_ptr = (void*) PT_REGS_PARM1(ctx);
  int64_t fields_len = (int64_t)PT_REGS_PARM2(ctx);
  submit_headers(ctx, fields_ptr, fields_len);
  return 0;
}

// Signature: func (t *http2Server) operateHeaders(frame *http2.MetaHeadersFrame, handle func(*Stream),
// traceCtx func(context.Context, string) context.Context)
int probe_http2_server_operate_headers(struct pt_regs* ctx) {
  void* fields_ptr = (void*) ctx->r8;
  // void* fields_ptr = (void *)PT_REGS_PARM4(ctx);
  int64_t fields_len = (int64_t) ctx->r11;
  // int64_t fields_len = (int64_t) PT_REGS_PARM5(ctx);
  submit_headers(ctx, fields_ptr, fields_len);
  return 0;
}