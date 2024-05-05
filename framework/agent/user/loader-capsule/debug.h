#ifndef __CAPSULE_DEBUG_H__
#define __CAPSULE_DEBUG_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define DEBUG_FS "/sys/kernel/debug/tracing/trace_pipe"

void read_trace_pipe();
void int_signal_handler(int signo);

#endif // __CAPSULE_DEBUG_H__