// Code originally written by Kris Nóva <admin@krisnova.net>
// Modified and adapted by Thorsten Horstmann <mail@thdot.de>

#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcpjack.h"

#define PROC_NET_TCP "/proc/net/tcp"
#define PROC_NET_TCP6 "/proc/net/tcp6"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

//            46: 010310AC:9C4C 030310AC:1770 01
//        |      |      |      |      |   |--> connection state
//        |      |      |      |      |------> remote TCP port number
//        |      |      |      |-------------> remote IPv4 address
//        |      |      |--------------------> local TCP port number
//        |      |---------------------------> local IPv4 address
//        |----------------------------------> number of entry
//
//              00000150:00000000 01:00000019 00000000
//        |        |     |     |       |--> number of unrecovered RTO timeouts
//        |        |     |     |----------> number of jiffies until timer
//        expires |        |     |----------------> timer_active (see below) |
//        |----------------------> receive-queue
//        |-------------------------------> transmit-queue

static void parse_ipv4_hex(const char* hex_str, char* addr_str)
{
    struct in_addr addr;
    sscanf(hex_str, "%x", &addr.s_addr);
    inet_ntop(AF_INET, &addr, addr_str, INET6_ADDRSTRLEN);
}
static void parse_ipv6_hex(const char* hex_str, char* addr_str)
{
    struct in6_addr addr;
    sscanf(hex_str, "%8x%8x%8x%8x",
        &addr.s6_addr32[0], &addr.s6_addr32[1], &addr.s6_addr32[2], &addr.s6_addr32[3]);
    inet_ntop(AF_INET6, &addr, addr_str, INET6_ADDRSTRLEN);
}

static void parse_proc_net_tcp(const char* path, sa_family_t family, struct TCPList* tcplist)
{
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
        return;
    }
    char line[1024];
    int line_num = 0;
    while (fgets(line, sizeof(line), f)) {
        if (line_num++ == 0) continue; // Skip header
        struct TCPConn conn = { .family = family };
        unsigned int state;
        char local_ip_hex[65];
        char remote_ip_hex[65];
        int items = sscanf(line, "%*d: %64[^:]:%x %64[^:]:%x %x %*X:%*X %*x:%*x %*s %u %*d %lu",
            local_ip_hex, &conn.local_port, remote_ip_hex, &conn.remote_port, &state, &conn.uid, &conn.inode);
        if (items < 7) {
            fprintf(stderr, "Failed to parse %s\n", path);
            exit(EXIT_FAILURE);
        }
        if (state == TCP_ESTABLISHED && tcplist->numconns < TCP_LIST_MAXSIZE) {
            conn.proc_entry = proc_entry_from_ino(conn.inode);
            tcplist->conns[tcplist->numconns++] = conn;
        }
        if (family == AF_INET) {
            parse_ipv4_hex(local_ip_hex, conn.local_addr);
            parse_ipv4_hex(remote_ip_hex, conn.remote_addr);
        } else { // AF_INET6
            parse_ipv6_hex(local_ip_hex, conn.local_addr);
            parse_ipv6_hex(remote_ip_hex, conn.remote_addr);
        }
    }
    fclose(f);
}

static int compareTCPConn(const void* a, const void* b)
{
    return ((struct TCPConn*)a)->proc_entry.pid - ((struct TCPConn*)b)->proc_entry.pid;
}

struct TCPList list()
{
    struct TCPList tcplist = {};
    parse_proc_net_tcp(PROC_NET_TCP, AF_INET, &tcplist);
    parse_proc_net_tcp(PROC_NET_TCP6, AF_INET6, &tcplist);
    qsort(tcplist.conns, tcplist.numconns, sizeof(struct TCPConn), compareTCPConn);
    return tcplist;
}

void print_list(struct TCPList tcplist)
{
    unsigned max_addr_size = 0;
    for (int i = 0; i < tcplist.numconns; i++) {
        max_addr_size = MAX(max_addr_size, strlen(tcplist.conns[i].local_addr));
    }
    printf("\e[1;31m%16s %6s %6s %*s    %s\x1B[0m\n", "process", "pid", "inode",
        max_addr_size + 6, "source address and port", "destination address and port");
    for (int i = 0; i < tcplist.numconns; i++) {
        struct TCPConn* conn = &tcplist.conns[i];
        printf("\x1B[32m%16s %6d\x1B[0m ", conn->proc_entry.comm, conn->proc_entry.pid);
        printf("\x1B[33m%6lu\x1B[0m ", conn->inode);
        printf("%*s %-5d ", max_addr_size, conn->local_addr, conn->local_port);
        printf("%s", "\x1B[33m->\x1B[0m ");
        printf("%s %d", conn->remote_addr, conn->remote_port);
        printf("\n");
    }
}
