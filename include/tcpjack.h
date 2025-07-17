// Code originally written by Kris Nóva <admin@krisnova.net>
// Modified and adapted by Thorsten Horstmann <mail@thdot.de>

#ifndef TCPJACK_H
#define TCPJACK_H

#define VERSION "0.0.3"
#define TCP_LIST_MAXSIZE 1024

#include <arpa/inet.h>
#include <errno.h>

/**
 * List all TCP connections which can be instrumented.
 *
 * @return TCPList A structure containing a list of ESTABLISHED TCP connections
 */
struct TCPList list();

/**
 * Print a TCP list using tcpjack default printing semantics.
 *
 * @param tcplist
 */
void print_list(struct TCPList tcplist);

/**
 * ProcEntry is a entry from procfs for a given process at runtime.
 */
struct ProcEntry {
    pid_t pid;
    char* comm;
    int jacked_fd;
};

/**
 * A TCP connection which can be instrumented, also
 * an associated ProcEntry for the corresponding process.
 */
struct TCPConn {
    ino_t inode;
    sa_family_t family;
    char local_addr[INET6_ADDRSTRLEN];
    unsigned int local_port;
    char remote_addr[INET6_ADDRSTRLEN];
    unsigned int remote_port;
    uid_t uid;
    struct ProcEntry proc_entry;
};

/**
 * A set of valid TCP connections which can be instrumented.
 */
struct TCPList {
    size_t numconns;
    struct TCPConn conns[TCP_LIST_MAXSIZE];
};

/**
 * Will lookup a ProcEntry for a given inode (fd) found in /proc/net/tcp
 *
 * @param ino
 * @return
 */
struct ProcEntry proc_entry_from_ino(ino_t inode);

/**
 * Will lookup a ProcEntry for a give pid.
 *
 * @param pid
 * @return
 */
struct ProcEntry proc_entry_from_pid(pid_t pid);

#endif