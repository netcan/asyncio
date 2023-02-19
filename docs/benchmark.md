<!--ts-->
* [AsyncIO eventloop echo server benchmark](#asyncio-eventloop-echo-server-benchmark)
   * [Python3.9.9 asyncio](#python399-asyncio)
   * [This project](#this-project)
   * [Asio1.18.0 in coroutine mode](#asio1180-in-coroutine-mode)
   * [Tokio-rs 1.14.0](#tokio-rs-1140)
   * [C epoll version](#c-epoll-version)
   * [C libevent-2.1.so.7](#c-libevent-21so7)
   * [C libuv1.42.0](#c-libuv1420)
* [Test Code](#test-code)
   * [Python version](#python-version)
   * [This project](#this-project-1)
   * [Asio version](#asio-version)
   * [Tokio-rs version](#tokio-rs-version)
   * [C epoll version](#c-epoll-version-1)
   * [C libevent version](#c-libevent-version)
   * [C libuv version](#c-libuv-version)

<!-- Added by: netcan, at: Tue Dec  7 07:54:39 PM HKT 2021 -->

<!--te-->

# AsyncIO eventloop echo server benchmark
- Date: 2021-12-3
- CommitId: bc031cc835d8da4549aebc752c3a3bda5f103012
- Test command: `ab -n 10000000 -c 1000 -k http://127.0.0.1:8888/`
- OS: Linux debian 5.15.0-1-amd64 #1 SMP Debian 5.15.3-1 (2021-11-18) x86_64 GNU/Linux
- CPU: AMD Ryzen 5 2600X Six-Core Processor
- Compiler: g++-12 (Debian 12-20211117-1) 12.0.0 20211117 (experimental) [master r12-5346-gd3a9082d7ac]
- Compiler arguments: `-O3 -DNDEBUG`

## Python3.9.9 asyncio
```shell
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   210.999 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    47393.59 [#/sec] (mean)
Time per request:       21.100 [ms] (mean)
Time per request:       0.021 [ms] (mean, across all concurrent requests)
Transfer rate:          4905.98 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   8.5      0    1044
Processing:     6   21   4.6     21    1782
Waiting:        0   21   4.6     21    1782
Total:          6   21  11.8     21    2816

Percentage of the requests served within a certain time (ms)
  50%     21
  66%     21
  75%     21
  80%     21
  90%     21
  95%     22
  98%     22
  99%     25
 100%   2816 (longest request)
```

with uvloop:
```shell
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   99.575 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    100426.97 [#/sec] (mean)
Time per request:       9.957 [ms] (mean)
Time per request:       0.010 [ms] (mean, across all concurrent requests)
Transfer rate:          10395.76 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   8.3      0    1033
Processing:     2   10   7.0      7    1711
Waiting:        0   10   7.0      7    1711
Total:          2   10  13.6      7    2741

Percentage of the requests served within a certain time (ms)
  50%      7
  66%     13
  75%     13
  80%     13
  90%     14
  95%     14
  98%     14
  99%     14
 100%   2741 (longest request)
```

## This project
```shell
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   60.806 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    164457.63 [#/sec] (mean)
Time per request:       6.081 [ms] (mean)
Time per request:       0.006 [ms] (mean, across all concurrent requests)
Transfer rate:          17023.93 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      24
Processing:     4    6   0.5      6      24
Waiting:        0    6   0.5      6      14
Total:          4    6   0.5      6      33

Percentage of the requests served within a certain time (ms)
  50%      6
  66%      6
  75%      6
  80%      6
  90%      7
  95%      7
  98%      8
  99%      8
 100%     33 (longest request)
```

## Asio1.18.0 in coroutine mode
```shell
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   62.766 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    159322.66 [#/sec] (mean)
Time per request:       6.277 [ms] (mean)
Time per request:       0.006 [ms] (mean, across all concurrent requests)
Transfer rate:          16492.38 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      25
Processing:     2    6   0.2      6      24
Waiting:        0    6   0.2      6      15
Total:          2    6   0.3      6      35

Percentage of the requests served within a certain time (ms)
  50%      6
  66%      6
  75%      6
  80%      6
  90%      6
  95%      7
  98%      7
  99%      7
 100%     35 (longest request)
```

## Tokio-rs 1.14.0
```shell
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   63.754 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    156852.70 [#/sec] (mean)
Time per request:       6.375 [ms] (mean)
Time per request:       0.006 [ms] (mean, across all concurrent requests)
Transfer rate:          16236.71 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      24
Processing:     0    6   1.2      6     164
Waiting:        0    6   1.2      6     164
Total:          0    6   1.3      6     174

Percentage of the requests served within a certain time (ms)
  50%      6
  66%      6
  75%      6
  80%      6
  90%      7
  95%      7
  98%      7
  99%      8
 100%    174 (longest request)
```

## C epoll version
```shell
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   65.296 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    153147.79 [#/sec] (mean)
Time per request:       6.530 [ms] (mean)
Time per request:       0.007 [ms] (mean, across all concurrent requests)
Transfer rate:          15853.19 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      25
Processing:     2    7   0.1      7      25
Waiting:        0    7   0.1      7      13
Total:          2    7   0.3      7      35

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      7
  75%      7
  80%      7
  90%      7
  95%      7
  98%      7
  99%      7
 100%     35 (longest request)
```

## C libevent-2.1.so.7
```shell
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   72.995 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    136996.46 [#/sec] (mean)
Time per request:       7.299 [ms] (mean)
Time per request:       0.007 [ms] (mean, across all concurrent requests)
Transfer rate:          14181.27 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      28
Processing:     4    7   0.3      7      28
Waiting:        0    7   0.3      7      14
Total:          4    7   0.4      7      39

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      7
  75%      7
  80%      7
  90%      7
  95%      8
  98%      8
  99%      8
 100%     39 (longest request)
```

## C libuv1.42.0
```shell
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   62.524 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    159937.73 [#/sec] (mean)
Time per request:       6.252 [ms] (mean)
Time per request:       0.006 [ms] (mean, across all concurrent requests)
Transfer rate:          16556.05 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      25
Processing:     2    6   0.2      6      25
Waiting:        0    6   0.2      6      14
Total:          2    6   0.3      6      38

Percentage of the requests served within a certain time (ms)
  50%      6
  66%      6
  75%      6
  80%      6
  90%      6
  95%      6
  98%      7
  99%      7
 100%     38 (longest request)
```

# Test Code
## Python version
asyncio:
```python
import asyncio

async def handle_echo(reader, writer):
    while True:
        data = await reader.read(200)
        if len(data) == 0: break

        writer.write(data)
        await writer.drain()

    writer.close()

async def main():
    server = await asyncio.start_server(
        handle_echo, '127.0.0.1', 8888)

    addrs = ', '.join(str(sock.getsockname()) for sock in server.sockets)
    print(f'Serving on {addrs}')

    async with server:
        await server.serve_forever()

asyncio.run(main())
```

asyncio with uvloop:
```python
import asyncio
import uvloop

async def handle_echo(reader, writer):
    while True:
        data = await reader.read(200)
        if len(data) == 0: break

        writer.write(data)
        await writer.drain()

    writer.close()

async def main():
    server = await asyncio.start_server(
        handle_echo, '127.0.0.1', 8888)

    addrs = ', '.join(str(sock.getsockname()) for sock in server.sockets)
    print(f'Serving on {addrs}')

    async with server:
        await server.serve_forever()

uvloop.install()
asyncio.run(main())
```

## This project
```cpp
#include <asyncio/runner.h>
#include <asyncio/start_server.h>
#include <asyncio/task.h>
#include <fmt/core.h>

using asyncio::Stream;
using asyncio::Task;

Task<> handle_echo(Stream stream) {
    while (true) {
        auto data = co_await stream.read(200);
        if (data.empty()) { break; }
        co_await stream.write(data);
    }

    stream.close();
}

Task<> echo_server() {
    auto server = co_await asyncio::start_server(
            handle_echo, "127.0.0.1", 8888);

    fmt::print("Serving on 127.0.0.1:8888\n");

    co_await server.serve_forever();
}

int main() {
    asyncio::run(echo_server());
    return 0;
}
```

## Asio version
```cpp
//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

boost::asio::awaitable<void> echo(tcp::socket socket)
{
    try
    {
        char data[1024];
        for (;;)
        {
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), boost::asio::use_awaitable);
            co_await async_write(socket, boost::asio::buffer(data, n), boost::asio::use_awaitable);
        }
    }
    catch (std::exception& e)
    {
        std::printf("echo Exception: %s\n", e.what());
    }
}

class server
{
public:
  server(boost::asio::io_service& io_service, short port):
      io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      socket_(io_service)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(socket_,
            [this](boost::system::error_code ec)
            {
                boost::asio::co_spawn(io_service_.get_executor(),
                    [socket = std::move(socket_)]() mutable
                    { return echo(std::move(socket)); },
                    boost::asio::detached);
                do_accept();
            }
        );
    }

    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_service io_service(1);

        server s(io_service, std::atoi(argv[1]));
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
```

## Tokio-rs version
Reference: https://github.com/tokio-rs/tokio

`cargo run --release`

```rs
use tokio::net::TcpListener;
use tokio::io::{AsyncReadExt, AsyncWriteExt};

#[tokio::main(worker_threads = 1)]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let listener = TcpListener::bind("127.0.0.1:8888").await?;

    loop {
        let (mut socket, _) = listener.accept().await?;

        tokio::spawn(async move {
            let mut buf = [0; 1024];

            // In a loop, read data from the socket and write the data back.
            loop {
                let n = match socket.read(&mut buf).await {
                    // socket closed
                    Ok(n) if n == 0 => return,
                    Ok(n) => n,
                    Err(e) => {
                        eprintln!("failed to read from socket; err = {:?}", e);
                        return;
                    }
                };

                // Write the data back
                if let Err(e) = socket.write_all(&buf[0..n]).await {
                    eprintln!("failed to write to socket; err = {:?}", e);
                    return;
                }
            }
        });
    }
}
```

## C epoll version
Reference: [https://github.com/frevib/epoll-echo-server/blob/master/epoll_echo_server.c](https://github.com/frevib/epoll-echo-server/blob/master/epoll_echo_server.c)
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 512
#define MAX_EVENTS 2048
#define MAX_MESSAGE_LEN 2048

void error(char* msg);


int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Please give a port number: ./epoll_echo_server [port]\n");
        exit(0);
    }

    // some variables we need
    int portno = strtol(argv[1], NULL, 10);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buffer[MAX_MESSAGE_LEN];
    memset(buffer, 0, sizeof(buffer));


    // setup socket
    int sock_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_listen_fd < 0) {
        error("Error creating socket..\n");
    }
    int yes = 1;
    // lose the pesky "address already in use" error message
    setsockopt(sock_listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr.s_addr = INADDR_ANY;


    // bind socket and listen for connections
    if (bind(sock_listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("Error binding socket..\n");

    if (listen(sock_listen_fd, BACKLOG) < 0) {
        error("Error listening..\n");
    }
    printf("epoll echo server listening for connections on port: %d\n", portno);


    struct epoll_event ev, events[MAX_EVENTS];
    int new_events, sock_conn_fd, epollfd;

    epollfd = epoll_create(MAX_EVENTS);
    if (epollfd < 0)
    {
        error("Error creating epoll..\n");
    }
    ev.events = EPOLLIN;
    ev.data.fd = sock_listen_fd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_listen_fd, &ev) == -1)
    {
        error("Error adding new listeding socket to epoll..\n");
    }

    while(1)
    {
        new_events = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        if (new_events == -1)
        {
            error("Error in epoll_wait..\n");
        }

        for (int i = 0; i < new_events; ++i)
        {
            if (events[i].data.fd == sock_listen_fd)
            {
                sock_conn_fd = accept4(sock_listen_fd, (struct sockaddr *)&client_addr, &client_len, SOCK_NONBLOCK);
                if (sock_conn_fd == -1)
                {
                    error("Error accepting new connection..\n");
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = sock_conn_fd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_conn_fd, &ev) == -1)
                {
                    error("Error adding new event to epoll..\n");
                }
            }
            else
            {
                int newsockfd = events[i].data.fd;
                int bytes_received = recv(newsockfd, buffer, MAX_MESSAGE_LEN, 0);
                if (bytes_received <= 0)
                {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, newsockfd, NULL);
                    shutdown(newsockfd, SHUT_RDWR);
                }
                else
                {
                    send(newsockfd, buffer, bytes_received, 0);
                }
            }
        }
    }
}

void error(char* msg)
{
    perror(msg);
    printf("erreur...\n");
    exit(1);
}
```

## C libevent version
```c
/*
 * echo-server.c
 *
 * This is a modified version of the "simpler ROT13 server with
 * Libevent" from:
 * http://www.wangafu.net/~nickm/libevent-book/01_intro.html
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

void
on_read(struct bufferevent *bev, void *ctx)
{
     struct evbuffer *input, *output;
     char *line;
     size_t n;
     int i;

     input = bufferevent_get_input(bev);
     output = bufferevent_get_output(bev);

     while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) {
	  evbuffer_add(output, line, n);
	  evbuffer_add(output, "\n", 1);
	  free(line);
     }

     if (evbuffer_get_length(input) >= MAX_LINE) {
	  /* line is too long */
	  char buf[1024];
	  while (evbuffer_get_length(input)) {
	       int n = evbuffer_remove(input, buf, sizeof(buf));
	       evbuffer_add(output, buf, n);
	  }
	  evbuffer_add(output, "\n", 1);
     }
}

void
on_error(struct bufferevent *bev, short error, void *ctx)
{
     if (error & BEV_EVENT_EOF) {
     } else if (error & BEV_EVENT_ERROR) {
     } else if (error & BEV_EVENT_TIMEOUT) {
     }
     bufferevent_free(bev);
}

void
on_accept(evutil_socket_t listener, short event, void *arg)
{
     struct event_base *base = arg;
     struct sockaddr_storage ss;
     socklen_t slen = sizeof(ss);
     int fd = accept(listener, (struct sockaddr*)&ss, &slen);
     if (fd < 0) {
	  perror("accept");
     } else if (fd > FD_SETSIZE) {
	  close(fd);
     } else {
	  struct bufferevent *bev;
	  evutil_make_socket_nonblocking(fd);
	  bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	  bufferevent_setcb(bev, on_read, NULL, on_error, NULL);
	  bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
	  bufferevent_enable(bev, EV_READ | EV_WRITE);
     }
}

int
main(int argc, char **argv)
{
     setvbuf(stdout, NULL, _IONBF, 0);

     evutil_socket_t listener;
     struct sockaddr_in sin;
     struct event_base *base;
     struct event *listener_event;

     base = event_base_new();
     if (!base)
	  return 1;

     sin.sin_family = AF_INET;
     sin.sin_addr.s_addr = 0;
     sin.sin_port = htons(8888);

     listener = socket(AF_INET, SOCK_STREAM, 0);
     evutil_make_socket_nonblocking(listener);
     int yes = 1;
     // lose the pesky "address already in use" error message
     setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

     /* win32 junk? */

     if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
	  perror("bind");
	  return 1;
     }

     if (listen(listener, 16) < 0) {
	  perror("listen");
	  return 1;
     }

     listener_event = event_new(base, listener, EV_READ | EV_PERSIST, on_accept, (void *)base);
     /* check it? */
     event_add(listener_event, NULL);

     event_base_dispatch(base);

     return 0;
}
```

## C libuv version
Reference: [https://github.com/delaemon/libuv-tutorial/blob/master/tcp-echo-server.c](https://github.com/delaemon/libuv-tutorial/blob/master/tcp-echo-server.c)
```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t *loop;
struct sockaddr_in addr;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t*) client, NULL);
        }
    } else if (nread > 0) {
        uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
        uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
        uv_write(req, client, &wrbuf, 1, echo_write);
    }

    if (buf->base) {
        free(buf->base);
    }
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*)client, alloc_buffer, echo_read);
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main() {
    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_ip4_addr("127.0.0.1", 8888, &addr);

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*)&server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
```
