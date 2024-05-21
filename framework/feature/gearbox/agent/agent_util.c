#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

#include <cjson/cJSON.h>

#include "agent_util.h"

char *config_path = NULL;
char *pid_filter_path = NULL;
int this_aggregator_idx = -1;

DebugLevel debug_level = NO_DEBUG;

static void print_help(char *prog_name) {
    printf("Usage: %s -c <config> -i <index> -p <pid_filter> [-h] [-d <0|1|2>]\n", prog_name);

    printf("  -c <config_file_path>\t\tSpecify the cgroup path that is required for operation.\n");
    printf("  -d <0|1|2>\t\t\tControl the level of debug information (0 for none, 1 for minimal, 2 for full).\n");
    printf("  -i <index_of_this_aggregator>\tSpecify the idx of this aggregator in the configuration file.\n");
    printf("  -p <pid_filter_file_path>\tSpecify the path to the pid filter file.\n");
    printf("  -h\t\t\t\tDisplay this help and exit.\n");
}

void parse_args(int argc, char **argv) {
    int c;

    while ((c = getopt(argc, argv, "c:d:i:p:h")) != -1) {
        switch (c) {
            case 'c':
                config_path = optarg;
                break;
            case 'd':
                if (atoi(optarg) >= 0 && atoi(optarg) <= 2) {
                    debug_level = (DebugLevel)atoi(optarg);
                } else {
                    fprintf(stderr, "Invalid debug level: %s\n", optarg);
                    print_help(argv[0]);
                    exit(1);
                }
                break;
            case 'i':
                this_aggregator_idx = atoi(optarg);
                break;
            case 'h':
                print_help(argv[0]);
                exit(0);
            case 'p':
                pid_filter_path = optarg;
                break;
            default:
                print_help(argv[0]);
                exit(1);
        }
    }

    if (config_path == NULL) {
        fprintf(stderr, "Error: The -f option is required.\n");
        print_help(argv[0]);
        exit(1);
    } else {
        INFO("Config file path set to: %s\n", config_path);
    }

    if (this_aggregator_idx == -1) {
        fprintf(stderr, "Error: The -i option is required.\n");
        print_help(argv[0]);
        exit(1);
    } else {
        INFO("This aggregator idx set to: %d\n", this_aggregator_idx);
    }

    if (pid_filter_path == NULL) {
        fprintf(stderr, "Error: The -p option is required.\n");
        print_help(argv[0]);
        exit(1);
    } else {
        INFO("Pid filter file path set to: %s\n", pid_filter_path);
    }
}

int open_client(char *dest_ip, int dst_port) {
    int sockfd, new_fd;
    struct sockaddr_in dest_addr, client_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
        return -1;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dst_port);
    if (inet_aton(dest_ip, &dest_addr.sin_addr) == 0) {
        fprintf(stderr, "Invalid address: %s\n", dest_ip);
        close(sockfd);
        return -1;
    }
    memset(&(dest_addr.sin_zero), 0, sizeof(dest_addr.sin_zero));

    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Failed to connect: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int parse_config_file(const char *filename, ConfigData *config) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", filename);
        return -1;
    }

    // Read the entire file
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    // Parse JSON
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        free(data);
        return -1;
    }

       //Parse Controller
    cJSON *controllers = cJSON_GetObjectItemCaseSensitive(json, "controller");
    int num_controller = cJSON_GetArraySize(controllers);
    config->num_controllers = num_controller;
    config->controller_ips = malloc(num_controller * sizeof(char *));
    config->controller_ports = malloc(num_controller * sizeof(int));
    for (int i = 0; i < num_controller; i++) {
        cJSON *controller = cJSON_GetArrayItem(controllers, i);
        config->controller_ips[i] = strdup(cJSON_GetObjectItemCaseSensitive(controller, "ip")->valuestring);
        config->controller_ports[i] = cJSON_GetObjectItemCaseSensitive(controller, "port")->valueint;
    }

    // // Parse Aggregators
    // cJSON *aggregators = cJSON_GetObjectItemCaseSensitive(json, "aggregators");
    // int num_aggregators = cJSON_GetArraySize(aggregators);
    // config->num_aggregators = num_aggregators;
    // config->aggregator_ips = malloc(num_aggregators * sizeof(char *));
    // config->aggregator_ports = malloc(num_aggregators * sizeof(int));
    // config->aggregator_fds = malloc(num_aggregators * sizeof(int));
    // for (int i = 0; i < num_aggregators; i++) {
    //     cJSON *aggregator = cJSON_GetArrayItem(aggregators, i);
    //     config->aggregator_ips[i] = strdup(cJSON_GetObjectItemCaseSensitive(aggregator, "ip")->valuestring);
    //     config->aggregator_ports[i] = cJSON_GetObjectItemCaseSensitive(aggregator, "loader_port")->valueint;
    // }

    // // Parse Manangers
    // cJSON *managers = cJSON_GetObjectItemCaseSensitive(json, "managers");
    // int num_managers = cJSON_GetArraySize(managers);
    // config->num_managers = num_managers;
    // config->manager_ips = malloc(num_managers * sizeof(char *));
    // config->manager_ports = malloc(num_managers * sizeof(int));
    // config->manager_fds = malloc(num_managers * sizeof(int));
    // for (int i = 0; i < num_managers; i++) {
    //     cJSON *manager = cJSON_GetArrayItem(managers, i);
    //     config->manager_ips[i] = strdup(cJSON_GetObjectItemCaseSensitive(manager, "ip")->valuestring);
    //     config->manager_ports[i] = cJSON_GetObjectItemCaseSensitive(manager, "loader_port")->valueint;
    // }

    // Clean up
    cJSON_Delete(json);
    free(data);

    return 0;
}

void print_config_data(const ConfigData *config) {
    // 打印 Aggregators 的信息
    // printf("Aggregators:\n");
    // for (int i = 0; i < config->num_aggregators; i++) {
    //     printf("  Aggregator %d:\n", i + 1);
    //     printf("    IP: %s\n", config->aggregator_ips[i]);
    //     printf("    Port for loader: %d\n", config->aggregator_ports[i]);
    // }

    // // 打印 Managers 的信息
    // printf("Managers:\n");
    // for (int i = 0; i < config->num_managers; i++) {
    //     printf("  Manager %d:\n", i + 1);
    //     printf("    IP: %s\n", config->manager_ips[i]);
    //     printf("    Port for loader: %d\n", config->manager_ports[i]);
    // }

    printf("Controllers:\n");
    for (int i = 0; i < config->num_controllers; i++) {
        printf("  Controller %d:\n", i + 1);
        printf("    IP: %s\n", config->controller_ips[i]);
        printf("    Port: %d\n", config->controller_ports[i]);
    }
}