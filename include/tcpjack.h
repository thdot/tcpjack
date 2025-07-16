/******************************************************************************\
*                                                                             *
*                     ███╗   ██╗ ██████╗ ██╗   ██╗ █████╗                     *
*                     ████╗  ██║██╔═══██╗██║   ██║██╔══██╗                    *
*                     ██╔██╗ ██║██║   ██║██║   ██║███████║                    *
*                     ██║╚██╗██║██║   ██║╚██╗ ██╔╝██╔══██║                    *
*                     ██║ ╚████║╚██████╔╝ ╚████╔╝ ██║  ██║                    *
*                     ╚═╝  ╚═══╝ ╚═════╝   ╚═══╝  ╚═╝  ╚═╝                    *
*               Written By: Kris Nóva    <admin@krisnova.net>                 *
*                                                                             *
\******************************************************************************/

#ifndef TCPJACK_H
#define TCPJACK_H

#define VERSION "0.0.3"
#define TCP_LIST_SIZE SIZE_1024
#define TIME_MS 1000
#define DATAGRAM_LEN 4096
#define OPT_SIZE 20

#define SIZE_64 64
#define SIZE_1024 1024

#include <arpa/inet.h>
#include <dirent.h>

#include <errno.h>
#include "pcap.h"

/**
 * List all TCP connections which can be instrumented.
 *
 * @return TCPList A structure containing a list of ESTABLISHED TCP connections
 */
struct TCPList
list();

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
    /**
     * The jacked_fd is a 1 to many mapping of socket FDs to a given pid.
     * If this is parsed via an inode it is a specific FD.
     * If this is parsed via a pid, it is the first one found at runtime!
     */
    int jacked_fd;
};

/**
 * A TCP connection which can be instrumented, also
 * an associated ProcEntry for the corresponding process.
 */
struct TCPConn {
    ino_t ino;
    struct in_addr local_addr;
    int local_port;
    struct in_addr remote_addr;
    int remote_port;
    uid_t uid;
    struct ProcEntry proc_entry;
};

/**
 * A set of valid TCP connections which can be instrumented.
 */
struct TCPList {
    int numconns;
    struct TCPConn conns[TCP_LIST_SIZE];
};

/**
 * Will lookup a ProcEntry for a given inode (fd) found in /proc/net/tcp
 *
 * @param ino
 * @return
 */
struct ProcEntry
proc_entry_from_ino(ino_t ino);

/**
 * Will lookup a ProcEntry for a give pid.
 *
 * @param pid
 * @return
 */
struct ProcEntry
proc_entry_from_pid(pid_t pid);

/**
 * Print the asciiheader and version number to stdout.
 */
void asciiheader();

#endif