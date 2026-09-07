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
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) < 0) {
        perror("epoll_ctl");
        exit(1);
    }

    struct epoll_event ready[EPOLL_READY_SIZE];

    while (1) {
        int n = epoll_wait(epfd, ready, EPOLL_READY_SIZE, -1);
        if (n < 0) {
            perror("epoll_wait");
        }
        for (int i = 0; i < n; i += 1) {
            int fd = ready[i].data.fd;
            if (fd == lfd) {
                while (1) {
                    int cfd = accept4(lfd, NULL, NULL, SOCK_NONBLOCK);
                    if (cfd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        }
                        perror("accept4");
                        break;
                    }
                    ev.events = EPOLLIN;
                    ev.data.fd = cfd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                }
            } else {

            }
        }
    }    
}