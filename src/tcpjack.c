// Code originally written by Kris Nóva <admin@krisnova.net>
// Modified and adapted by Thorsten Horstmann <mail@thdot.de>

#include "tcpjack.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(void)
{
    printf("\e[0;33m  _             _            _    \e[0m\n");
    printf("\e[0;33m | |           (_)          | |   \e[0m\n");
    printf("\e[0;33m | |_ ___ _ __  _  __ _  ___| | __\e[0m\n");
    printf("\e[0;33m | __/ __| '_ \\| |/ _` |/ __| |/ /\e[0m\n");
    printf("\e[0;33m | || (__| |_) | | (_| | (__|   < \e[0m\n");
    printf("\e[0;33m  \\__\\___| .__/| |\\__,_|\\___|_|\\_\\ \e[0m\n");
    printf("\e[0;33m         | |  _/ |                \e[0m\n");
    printf("\e[0;33m         |_| |__/   \e[0mv%s              \n", VERSION);
    printf("\n");
    printf("\e[1;34mAuthors\e[0m: \e[0;34mKris Nóva\e[0m <\e[0;32mkrisnova@krisnova.net\e[0m>\n");
    printf("         \e[0;34mThorsten Horstmann\e[0m <\e[0;32mmail@thdot.de\e[0m>\n");
    printf("\n");
    printf("TCP Hijack tool.\n");
    printf("Use tcpjack to send exciting payloads across already established TCP streams.\n");
    printf("\n");
    printf("Usage: \n");
    printf("tcpjack [options]\n");
    printf("\n");
    printf("Options:\n");
    printf("-h, --help           Display help and usage.\n");
    printf("-l, --list           List established TCP connections and inodes.\n");
    printf("-j, --jack <inode>   Send data to existing TCP connection.\n");
    printf("\n");
    exit(0);
}

static void priv(void)
{
    if (geteuid() != 0) {
        fprintf(stderr, "Permission denied.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv)
{
    int opt;
    int option_index = 0;
    int do_list = 0;
    int do_jack = 0;
    ino_t ino = 0;

    static struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
        { "list", no_argument, 0, 'l' },
        { "jack", required_argument, 0, 'j' },
        { 0, 0, 0, 0 }
    };

    while ((opt = getopt_long(argc, argv, "hlj:", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        case 'l':
            do_list = 1;
            break;
        case 'j':
            do_jack = 1;
            char* endptr;
            ino = (ino_t)strtoull(optarg, &endptr, 10);
            if (errno != 0 || *endptr != '\0' || ino == 0) {
                fprintf(stderr, "Invalid or bad inode number: %s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    if (do_list) {
        struct TCPList tcplist = list();
        print_list(tcplist);
        return EXIT_SUCCESS;
    }

    if (do_jack) {
        priv();
        struct ProcEntry proc_entry = proc_entry_from_ino(ino);
        if (proc_entry.pid == 0) {
            fprintf(stderr, "Unable to trace inode %lu. Error finding proc entry for inode.\n", (unsigned long)ino);
            return EXIT_FAILURE;
        }
        int fd = proc_entry.jacked_fd;
        if (fd < 0) {
            if (errno == EPERM) {
                fprintf(stderr, "Permission denied.\n");
                return EXIT_FAILURE;
            }
            fprintf(stderr, "Error hijacking file descriptor for established TCP connection! %d %d\n", fd, errno);
            return EXIT_FAILURE;
        }
        char ch;
        while (read(STDIN_FILENO, &ch, 1) > 0) {
            int z = write(fd, &ch, 1);
            if (z != 1) {
                fprintf(stderr, "Error writing to hijacked connection! %d\n", errno);
                return EXIT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    }

    usage();
    return EXIT_FAILURE;
}
