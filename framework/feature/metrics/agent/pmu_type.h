#include <assert.h>
#include <fcntl.h>
#include <linux/perf_event.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <sys/syscall.h>
#define SAMPLE_PERIOD 0x7fffffffffffffffULL

struct perf_event_attr attr_cycles = {
    .freq = 0,
    .sample_period = SAMPLE_PERIOD,
    .inherit = 0,
    .type = PERF_TYPE_HARDWARE,
    .read_format = 0,
    .sample_type = 0,
    .config = PERF_COUNT_HW_CPU_CYCLES,
};



struct perf_event_attr attr_clock = {
    .freq = 0,
    .sample_period = SAMPLE_PERIOD,
    .inherit = 0,
    .type = PERF_TYPE_SOFTWARE,
    .read_format = 0,
    .sample_type = 0,
    .config = PERF_COUNT_SW_CPU_CLOCK,
};
struct perf_event_attr attr_raw = {
    .freq = 0,
    .sample_period = SAMPLE_PERIOD,
    .inherit = 0,
    .type = PERF_TYPE_RAW,
    .read_format = 0,
    .sample_type = 0,
    /* Intel Instruction Retired */
    .config = 0xc0,
    // .config = PERF_COUNT_HW_INSTRUCTIONS,
};

struct perf_event_attr attr_ins = {
    .freq = 0,
    // .sample_period = SAMPLE_PERIOD,
    .inherit = 0,
    .type = PERF_TYPE_HARDWARE,
    .read_format = 0,
    .sample_type = 0,
    .config = PERF_COUNT_HW_INSTRUCTIONS,
};

struct perf_event_attr attr_l1d_load = {
    .freq = 0,
    .sample_period = SAMPLE_PERIOD,
    .inherit = 0,
    .type = PERF_TYPE_HW_CACHE,
    .read_format = 0,
    .sample_type = 0,
    .config =
        PERF_COUNT_HW_CACHE_L1D |
        (PERF_COUNT_HW_CACHE_OP_READ << 8) |
        (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16),
};
struct perf_event_attr attr_llc_miss = {
    .freq = 0,
    .sample_period = SAMPLE_PERIOD,
    .inherit = 0,
    .type = PERF_TYPE_HW_CACHE,
    .read_format = 0,
    .sample_type = 0,
    .config =
        PERF_COUNT_HW_CACHE_LL |
        (PERF_COUNT_HW_CACHE_OP_READ << 8) |
        (PERF_COUNT_HW_CACHE_RESULT_MISS << 16),
};
struct perf_event_attr attr_msr_tsc = {
    .freq = 0,
    .sample_period = 0,
    .inherit = 0,
    /* From /sys/bus/event_source/devices/msr/ */
    .type = 7,
    .read_format = 0,
    .sample_type = 0,
    .config = 0,
};