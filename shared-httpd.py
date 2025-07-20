#!/usr/bin/env python3

import argparse, ctypes, os, socket, time

SERVER_PORT = 8000
SYS_PIDFD_GETFD = 438

def pidfd_getfd(pidfd: int, target_fd: int, flags: int = 0) -> int:
    _syscall = ctypes.CDLL(None, use_errno=True).syscall
    ret = _syscall(ctypes.c_long(SYS_PIDFD_GETFD), ctypes.c_int(pidfd), 
                   ctypes.c_int(target_fd), ctypes.c_uint(flags))
    if ret < 0:
        e = ctypes.get_errno()
        raise OSError(e, os.strerror(e))
    return ret

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--pid', type=int, help='Target PID')
    parser.add_argument('-f', '--fd', type=int, help='Target FD')
    return parser.parse_args()

def start_server(target_pid: int, target_fd: int):
    if target_pid and target_fd:
        target_pidfd = os.pidfd_open(target_pid)
        local_fd = pidfd_getfd(target_pidfd, target_fd)
        server_socket = socket.fromfd(local_fd, socket.AF_INET, socket.SOCK_STREAM)
        print('Duplicated the socket fd %d from pid %d and listening on it, pid=%d' % (
                target_fd, target_pid, os.getpid()))
    else:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('127.0.0.1', SERVER_PORT))
        server_socket.listen(1)
        print('Listening on port %s, pid=%d, fd=%d' % (
                SERVER_PORT, os.getpid(), server_socket.fileno()))
    while True:
        client_connection, _ = server_socket.accept()
        client_connection.recv(1024).decode()
        response = 'HTTP/1.0 200 OK\n\nResponse from process %d\n' % os.getpid()
        client_connection.sendall(response.encode())
        client_connection.close()
        time.sleep(1)
    server_socket.close()

if __name__ == '__main__':
    args = parse_args()
    start_server(args.pid, args.fd)
