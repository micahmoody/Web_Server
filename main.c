#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/epoll.h>

#include "constants.h"
#include "connection.h"

int main() {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket");
        exit(1);
    }

    int flags = fcntl(lfd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl");
        exit(1);
    }
    if (fcntl(lfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl");
        exit(1);
    }

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = htons(PORT);
    socklen_t bind_addr_len = sizeof(bind_addr);

    if (bind(lfd, (struct sockaddr *)&bind_addr, bind_addr_len) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(lfd, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        exit(1);
    }

    struct epoll_event ev;
    struct connection lfd_cont;
    lfd_cont.fd = lfd;
    ev.events = EPOLLIN;
    ev.data.ptr = &lfd_cont;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) < 0) {
        perror("epoll_ctl");
        exit(1);
    }

    struct epoll_event ready[EPOLL_READY_SIZE];

    while (1) {
        int n = epoll_wait(epfd, ready, EPOLL_READY_SIZE, -1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait");
            exit(1);
        }
        for (int i = 0; i < n; i += 1) {
            struct connection *con = ready[i].data.ptr;
            if (con -> fd == lfd) {
                while (1) {
                    int cfd = accept4(lfd, NULL, NULL, SOCK_NONBLOCK);
                    if (cfd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        }
                        perror("accept4");
                        break;
                    }

                    struct connection *con_ptr = malloc(sizeof *con_ptr);
                    if (con_ptr == NULL) {
                        perror("malloc");
                        close(cfd);
                        continue;
                    }
                    memset(con_ptr, 0, sizeof *con_ptr);

                    con_ptr -> fd = cfd;
                    con_ptr -> rs = INITIAL_READ_BUFFER_SIZE;
                    con_ptr -> read_buf = malloc(con_ptr -> rs);
                    if (con_ptr -> read_buf == NULL) {
                        perror("malloc");
                        disconnect(epfd, con_ptr);
                        continue;
                    }

                    ev.events = EPOLLIN;
                    ev.data.ptr = con_ptr;

                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) < 0) {
                        perror("epoll_ctl");
                        disconnect(epfd, con_ptr);
                        continue;
                    }
                }
            } else {
                while (1) {
                    if (con -> rs == con -> rp) {
                        int ns = con -> rs * READ_BUFFER_RESIZE_FACTOR;
                        if (ns > MAX_READ_BUFFER_SIZE) {
                            disconnect(epfd, con);
                            break;
                        }
                        char *nread_buf = realloc(con -> read_buf, ns);
                        if (nread_buf == NULL) {
                            perror("realloc");
                            break;
                        }
                        con -> read_buf = nread_buf;
                        con -> rs = ns;
                    }
                    int n = read(con -> fd, (con -> read_buf) + (con -> rp), (con -> rs) - (con -> rp));
                    if (n < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        }
                        perror("read");
                        break;
                    }
                    if (n == 0) {
                        disconnect(epfd, con);
                        break;
                    }
                    con -> rp += n;
                }
            }
        }
    }    
}