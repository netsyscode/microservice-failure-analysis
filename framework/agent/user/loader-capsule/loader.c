#define _XOPEN_SOURCE 500 // Required for FTW_PHYS flag
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <ftw.h>

#include "debug.h"
#include "capsule_util.h"
#include TO_HEADER(BPF_KERNEL_SKELETON)

typedef enum {
    DIR_ERROR = -1,
    DIR_NON_EXISTENT,
    DIR_EMPTY,
    DIR_NOT_EMPTY
} DirStatus;

int remove_file(const char *filepath, const struct stat *sbuf, int type, struct FTW *ftwb) {
    if(remove(filepath) < 0) {
        perror("Failed to remove");
        return -1;
    }
    return 0;
}

static int rmtree(const char *path) {
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry;

    // stat for the path
    stat(path, &stat_path);

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0) {
        // ignore
        return 0;
    }

    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        return 1;
    }

    // remove the directory
    if(nftw(path, remove_file, 64, FTW_DEPTH | FTW_PHYS) < 0) {
        fprintf(stderr, "Failed to remove directory");
        return 1;
    }

    /*
    // the length of the path
    path_len = strlen(path);

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        // determinate a full path of an entry
        full_path = calloc(path_len + strlen(entry->d_name) + 2, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        // stat for the entry
        stat(full_path, &stat_entry);

        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            rmtree(full_path);
            continue;
        }

        // remove a file object
        if (unlink(full_path)) {
            printf("Can`t remove a file: %s\n", full_path);
            return 1;
        }
        free(full_path);
    }

    // remove the devastated directory and close the object of it
    if (rmdir(path)) {
        printf("Can`t remove a directory: %s\n", path);
        return 1;
    }
    closedir(dir);
    */
    return 0;
}

DirStatus check_dir(const char *path) {
    DIR *dir;
    struct stat stat_path;
    struct dirent *entry;

    // stat for the path
    stat(path, &stat_path);

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0) {
        printf("%s\n", "Directory does not exist");
        return DIR_NON_EXISTENT;  // Dir non existent
    }

    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        return DIR_ERROR;
    }

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        fprintf(stderr, "%s %s\n", path, "is not empty");
        closedir(dir);
        return DIR_NOT_EMPTY;
    }

    closedir(dir);
    return DIR_EMPTY;  // Dir empty
}

static int cleanup_pinfolder() {
    return rmtree(PIN_FOLDER);
}

static int pin_link(struct bpf_link *link, const char* path) {
    int err = 0;
    err = bpf_link__pin(link, path);
    if (err) {
        printf("Could not pin link %s: %d\n", path, err);
        perror("Error Reason:");
    }
    return err;
}

static int pin_stuff(struct BPF_KERNEL_SKELETON *skel) {
    int err = 0, counter;
    char *name, pin_path[1024];

    // Pin maps and programs
    bpf_object__pin(skel->obj, PIN_FOLDER);

    // Pin Links

    // Here we use void * and casting because struct bpf_link is *declared* in /usr/include/bpf/libbpf.h
    // Its definition is in either vmlinux.h or libbpf
    // Hence, arithmetic on its pointer, which are an incomplete type, leads to complication error 
    // counter = 0;
    // void *link_ptr;  
    // bpf_object__for_each_link(link_ptr, skel->links) {
    //     memset(pin_path, 0, sizeof(pin_path));
    //     sprintf(pin_path, "%s/link_%s", PIN_FOLDER, bpf_object__name((struct bpf_object *)(*(struct bpf_link **)link_ptr)));
    //     printf("Pinning link %s\n", pin_path);
    //     err = pin_link(*(struct bpf_link **)link_ptr, pin_path);
    //     if (err) { 
    //         return err; 
    //     }
    // }

    return 0;
}

int main() {
    int err = 0;
    struct BPF_KERNEL_SKELETON *skel;

    skel = SKEL_OPEN(BPF_KERNEL_SKELETON)();
    if (!skel) {
        fprintf(stderr, "%s\n", "Failed to open skeleton");
        return -1;
    }
    printf("Skeleton opened\n");

    err = SKEL_LOAD(BPF_KERNEL_SKELETON)(skel);
    if (err) {
        fprintf(stderr, "%s\n", "Failed to load skeleton");
        goto cleanup;
    }
    printf("Skeleton loaded\n");

    err = SKEL_ATTACH(BPF_KERNEL_SKELETON)(skel);
    if (err) {
        fprintf(stderr, "%s\n", "Failed to attach skeleton");
        goto cleanup;
    }
    printf("Skeleton attached\n");

#ifdef DEBUG
    signal(SIGINT, int_signal_handler);
    fprintf(stdout, "%s\n", "Reading trace_pipe, press Ctrl+C to stop");
    read_trace_pipe();

    err = SKEL_DETACH(BPF_KERNEL_SKELETON)(skel);
    if (err) {
        fprintf(stderr, "%s\n", "Failed to detach skeleton");
        goto cleanup;
    }
#else
    DirStatus status = check_dir(PIN_FOLDER);
    switch (status) {
        case DIR_ERROR:
            fprintf(stderr, "%s %s\n", "Failed to check pin folder", PIN_FOLDER);
            goto cleanup;
        case DIR_NON_EXISTENT:
            fprintf(stdout, "%s %s\n", "Creating pin folder", PIN_FOLDER);
            err = mkdir(PIN_FOLDER, 0777);
            if (err) {
                fprintf(stderr, "%s %s\n", "Failed to create pin folder", PIN_FOLDER);
                perror("Error Reason:");
                goto cleanup;
            }
            printf("Pin folder created\n");
            break;
        case DIR_EMPTY:
            break;
        case DIR_NOT_EMPTY:
            fprintf(stdout, "%s\n", "Do you want to clean up the directory? (y/n): ");

            char input[256];
            memset(input, 0, sizeof(input));
            if (fgets(input, sizeof(input), stdin)) {
                input[strcspn(input, "\n")] = 0;
                if (strcmp(input, "y") == 0 || strcmp(input, "Y") == 0) {
                    err = cleanup_pinfolder();
                    if (err) {
                        fprintf(stderr, "%s\n", "Failed to cleanup pin folder");
                        goto cleanup;
                    }
                    fprintf(stdout, "%s\n", "Pin folder cleaned up successfully");
                } else {
                    fprintf(stderr, "%s\n", "Skipping cleanup. Exiting loader capsule without attachment.");
                    goto cleanup;
                }
            } else {
                fprintf(stderr, "%s\n", "Failed to read user input");
                goto cleanup;
            }
            break;
    }

    err = pin_stuff(skel);
    if (err) {
        fprintf(stderr, "%s\n", "Failed to pin eBPF program");
        cleanup_pinfolder();
        goto cleanup;
    }

    fprintf(stdout, "%s %s %s\n", 
        "BPF program",
        STRINGIZE_AFTER_EXPAND(EXPAND_MACRO(BPF_KERNEL_SKELETON)),
        "attached. Exiting loader capsule.");
#endif

cleanup:
    SKEL_DESTROY(BPF_KERNEL_SKELETON)(skel);
    return err;
}