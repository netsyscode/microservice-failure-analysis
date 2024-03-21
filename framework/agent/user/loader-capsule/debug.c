#include "debug.h"

volatile sig_atomic_t keep_running = 1;

void int_signal_handler(int dummy) {
    keep_running = 0;
}

void read_trace_pipe(void) {
    int trace_fd;
    trace_fd = open(DEBUG_FS, O_RDONLY, 0);
    if (trace_fd < 0) {
        printf("Failed to open %s\n", DEBUG_FS);  // DEBUGFS is a macro defined in debug.h
        return;
    }

    while (keep_running) {
        static char buf[4096];
        ssize_t sz;

        sz = read(trace_fd, buf, sizeof(buf) - 1);
        if (sz > 0) {
            buf[sz] = 0;
            puts(buf);
        }
    }

    close(trace_fd);
}