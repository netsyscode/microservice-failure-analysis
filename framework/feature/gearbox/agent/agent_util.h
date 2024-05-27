#ifndef __AGENT_UTIL_H__
#define __AGENT_UTIL_H__

#include "agent_structs.h"

extern char *config_path;
extern char *pid_filter_path;

extern DebugLevel debug_level;

#define INFO(fmt, ...) \
    do { \
        if (debug_level >= MINIMAL_DEBUG) { \
            printf(fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#define DEBUG(fmt, ...) \
    do { \
        if (debug_level == FULL_DEBUG) { \
            printf(fmt, ##__VA_ARGS__); \
        } \
    } while (0)

void parse_args(int argc, char **argv);

int open_client(char *dest_ip, int dst_port);

int parse_config_file(const char *filename, ConfigData *config);
void print_config_data(const ConfigData *config);

#endif // __AGENT_UTIL_H__