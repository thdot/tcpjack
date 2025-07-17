// Code originally written by Kris Nóva <admin@krisnova.net>
// Modified and adapted by Thorsten Horstmann <mail@thdot.de>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "tcpjack.h"

struct ProcEntry proc_entry_from_pid(pid_t pid)
{
    struct ProcEntry proc_entry = { 0 };
    char* comm = malloc(1024);
    memset(comm, 0, 1024);
    char proc_comm_path[64];
    snprintf(proc_comm_path, 64, "/proc/%d/comm", pid);
    FILE* comm_f = fopen(proc_comm_path, "r");
    if (comm_f == NULL)
        return proc_entry;
    while (fgets(comm, 1024, comm_f)) {
        comm[strcspn(comm, "\n")] = 0;
        struct ProcEntry proc_entry = {
            .pid = pid, .comm = comm, .jacked_fd = 0
        };
        fclose(comm_f);
        return proc_entry;
    }
    fclose(comm_f);
    return proc_entry;
}

struct ProcEntry proc_entry_from_ino(ino_t inode)
{
    struct ProcEntry proc_entry = { 0 };
    char path[756];
    char needle[64] = "";
    struct dirent* procdentry; // Procfs
    snprintf(needle, 64, "socket:[%lu]", inode);
    DIR* procdp = opendir("/proc");
    if (procdp == NULL)
        return proc_entry;
    while ((procdentry = readdir(procdp)) != NULL) {
        struct dirent* procsubdentry; // Procfs Subdir
        snprintf(path, 512, "/proc/%s/fd", procdentry->d_name);
        DIR* procsubdp = opendir(path);
        if (procsubdp == NULL) {
            continue;
        }
        while ((procsubdentry = readdir(procsubdp)) != NULL) {
            char fd_content[64] = "";
            snprintf(path, 756, "/proc/%s/fd/%s", procdentry->d_name, procsubdentry->d_name);
            readlink(path, fd_content, 64);
            if (strcmp(fd_content, needle) == 0) {
                // Found the process
                pid_t pid = atoi(procdentry->d_name);
                closedir(procdp);
                closedir(procsubdp);
                struct ProcEntry rproc_entry = proc_entry_from_pid(pid);
                int pidfd = syscall(SYS_pidfd_open, pid, 0);
                rproc_entry.jacked_fd = syscall(SYS_pidfd_getfd, pidfd, atoi(procsubdentry->d_name), 0);
                return rproc_entry;
            }
        }
        closedir(procsubdp);
    }
    closedir(procdp);
    return proc_entry;
}
