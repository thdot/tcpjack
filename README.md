# TCP Jack

Hijack and send data over established TCP connections.

### Compiling

``` 
git clone https://github.com/thdot/tcpjack.git
cd tcpjack
make
```

### Example Hijacking a TCP connection

The following example shows how to hijack an existing TCP connection using the `-j` flag.
`tcpjack` will use `ptrace` to briefly interrupt the client with the specified inode.
During the interruption, `tcpjack` will steal the established connection's open file descriptor.
After the file descriptor has been copied, the process resumes normal processing.
The newly copied file descriptor is used to create a spoofed client over the same connection as the original.

```bash
# Terminal 1
ncat -l 9074

# Terminal 2 
ncat localhost 9074

# Terminal 3 
./tcpjack -l | grep ncat 
  ncat   9321  72294 127.0.0.1:48434 ->  127.0.0.1:9074 
  ncat   9237  76747  127.0.0.1:9074 -> 127.0.0.1:48434 
echo "PAYLOAD" | sudo ./tcpjack -j 72294
```

### Example *shared* socket HTTP Server (python)

`shared_httpd` demonstrate the `pidfd_getfd` system call in python:

```bash
# Terminal 1
./shared-httpd.py
Listening on port 8000, pid=23304, fd=3

# Terminal 2 
sudo ./shared-httpd.py --fd 3 --pid 23304
Duplicated the socket fd 3 from pid 23304 and listening on it, pid=23333

# Terminal 3 
for n in {0..3}; do curl 127.0.0.1:8000; sleep 0.5; done
Response from process 23333
Response from process 23304
Response from process 23333
Response from process 23304
```

As can be seen, responses come from the two servers (share a same TCP socket).

## Credits

This project is derived from [Kris Nóva](https://github.com/krisnova/tcpjack).
The original code has been cleaned up and extended to suport IPv6.
