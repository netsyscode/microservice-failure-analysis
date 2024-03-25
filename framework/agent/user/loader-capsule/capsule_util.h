#ifndef CAPSULE_UTIL_H
#define CAPSULE_UTIL_H

#define STRINGIZE(x)                #x
#define STRINGIZE_AFTER_EXPAND(x)   STRINGIZE(x)
#define TO_HEADER(x)                STRINGIZE(x.skel.h)

#define EXPAND_MACRO(x)             x
#define CONCAT_MACRO(x, y)          x##y
#define SKEL_OPEN(x)                EXPAND_MACRO(CONCAT_MACRO(x, __open))
#define SKEL_LOAD(x)                EXPAND_MACRO(CONCAT_MACRO(x, __load))
#define SKEL_ATTACH(x)              EXPAND_MACRO(CONCAT_MACRO(x, __attach))
#define SKEL_DETACH(x)              EXPAND_MACRO(CONCAT_MACRO(x, __detach))
#define SKEL_DESTROY(x)             EXPAND_MACRO(CONCAT_MACRO(x, __destroy))

#define BPF_LINK_PTR_SIZE           sizeof(void *)
#define BPF_LINKS_COUNT(links)      (sizeof(links) / BPF_LINK_PTR_SIZE)
#define bpf_object__for_each_link(link, links) \
    for ((link) = (void *)&(links); \
         (link) < (void *)&(links) + BPF_LINKS_COUNT(links); \
         (link)++)

#define BASE_FOLDER                 "/sys/fs/bpf"
#define PIN_FOLDER                  BASE_FOLDER "/" STRINGIZE_AFTER_EXPAND(EXPAND_MACRO(BPF_KERNEL_SKELETON))

#endif