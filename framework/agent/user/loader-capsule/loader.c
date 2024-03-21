#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "debug.h"

#define STRINGIZE(x)    #x
#define TO_HEADER(x)    STRINGIZE(x.skel.h)

#define EXPAND_MACRO(x)     x
#define CONCAT_MACRO(x, y)  x##y
#define SKEL_OPEN(x)        EXPAND_MACRO(CONCAT_MACRO(x, __open))
#define SKEL_LOAD(x)        EXPAND_MACRO(CONCAT_MACRO(x, __load))
#define SKEL_ATTACH(x)      EXPAND_MACRO(CONCAT_MACRO(x, __attach))
#define SKEL_DETACH(x)      EXPAND_MACRO(CONCAT_MACRO(x, __detach))
#define SKEL_DESTROY(x)     EXPAND_MACRO(CONCAT_MACRO(x, __destroy))

#include TO_HEADER(BPF_KERNEL_SKELETON)

int main() {
    int err = 0;
    struct BPF_KERNEL_SKELETON *skel;

    skel = SKEL_OPEN(BPF_KERNEL_SKELETON)();

    if (!skel) {
        goto cleanup;
    }

    err = SKEL_LOAD(BPF_KERNEL_SKELETON)(skel);
    if (err) {
        goto cleanup;
    }

    err = SKEL_ATTACH(BPF_KERNEL_SKELETON)(skel);
    if (err) {
        goto cleanup;
    }

#ifdef DEBUG
    signal(SIGINT, int_signal_handler);
    read_trace_pipe();

    SKEL_DETACH(BPF_KERNEL_SKELETON)(skel);
#else
    printf("%s attached. Exiting loader capsule.\n", STRINGIZE(BPF_KERNEL_SKELETON));
    sleep(10);
#endif

cleanup:
    printf("Do not clean up loader capsule.\n");
    // SKEL_DESTROY(BPF_KERNEL_SKELETON)(skel);
    return err;
}