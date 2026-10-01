#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/epoll.h>

#include "server.h"
#include "event.h"
#include "connection.h"

#define MAX_EVENTS 64

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

    s->ev.type = EVENT_SERVER;
    s->ev.data = s;

    struct epoll_event event;
    event.data.ptr = &s->ev;
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &event) < 0) {
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

enum server_accept_result {
    SERVER_ACCEPT_NO_CON,
    SERVER_ACCEPT_ERR,
    SERVER_ACCEPT_SUCCESS
};

enum server_accept_result server_accept(struct server *s) {

    int cfd = accept4(s->lfd, NULL, NULL, SOCK_NONBLOCK);
    if (cfd < 0) {

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return SERVER_ACCEPT_NO_CON;
        }

        perror("accept4");
        return SERVER_ACCEPT_ERR;
    }

    struct connection *con = connection_create(cfd);
    if (con == NULL) {
        close(cfd);
        return SERVER_ACCEPT_ERR;
    }

    con->ev.data = con;
    con->ev.type = EVENT_CONNECTION;

    struct epoll_event ev;
    ev.data.ptr = &con->ev;
    ev.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(s->efd, EPOLL_CTL_ADD, cfd, &ev) < 0) {
        connection_close(-1, con);
        connection_destroy(con);

        perror("epoll_ctl");
        return SERVER_ACCEPT_ERR;
    }

    return SERVER_ACCEPT_SUCCESS;

}

void server_destroy(struct server *s) {

    close(s->lfd);
    close(s->efd);

}

int server_run(struct server *s) {

    struct epoll_event events[MAX_EVENTS];

    while (1) {

        int n = epoll_wait(s->efd, events, MAX_EVENTS, -1);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait");
            return -1;
        }

        for (int i = 0; i < n; i += 1) {

            struct event *event = events[i].data.ptr;

            enum server_accept_result accept_result;

            switch (event->type) {
                case EVENT_SERVER:

                    while ((accept_result = server_accept(s)) == SERVER_ACCEPT_SUCCESS) {}

                    break;

                case EVENT_CONNECTION:

                    connection_handle_event(s->efd, event->data, events[i].events);

                    break;
            }

        }

        for (int i = 0; i < n; i += 1) {

            struct event *event = events[i].data.ptr;
            struct connection *con;

            switch (event->type) {

                case EVENT_CONNECTION:

                    con = event->data;
                    if (!con->alive) {
                        connection_destroy(con);
                    }

                    break;

                default: break;

            }
            
        }

    }

}