#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/epoll.h>

#include "server.h"
#include "event.h"

int server_init(struct server *s, struct server_config *config) {

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket");
        return -1;
    }

    int flags = fcntl(lfd, F_GETFL, 0);
    if (flags < 0) {
        close(lfd);

        perror("fcntl");
        return -1;
    }
    if (fcntl(lfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(lfd);

        perror("fcntl");
        return -1;
    }

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = htons(config->port);
    socklen_t bind_addr_len = sizeof(bind_addr);

    if (bind(lfd, (struct sockaddr *)&bind_addr, bind_addr_len) < 0) {
        close(lfd);

        perror("bind");
        return -1;
    }

    if (listen(lfd, config->backlog) < 0) {
        close(lfd);

        perror("listen");
        return -1;
    }

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        close(lfd);

        perror("epoll_create1");
        return -1;
    }

    s->ev.accept = 1;
    s->ev.write = s->ev.read = 0;
    s->ev.con = NULL;

    struct epoll_event event;
    event.data.ptr = &s->ev;
    event.events = EPOLLIN;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &event)) {
        close(epfd);
        close(lfd);

        perror("epoll_ctl");
        return -1;
    }

    s->lfd = lfd;
    s->config = *config;
    s->efd = epfd;

    return 0;
    
}