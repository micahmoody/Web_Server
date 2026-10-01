#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "connection.h"
#include "buffer.h"

#define INIT_READBUF_SIZE 4096
#define INIT_WRITEBUF_SIZE 4096

struct connection *connection_create(int cfd) {

    struct connection *con = malloc(sizeof(struct connection));

    if (
        buffer_init(&con->read_buffer, INIT_READBUF_SIZE) < 0 ||
        buffer_init(&con->write_buffer, INIT_WRITEBUF_SIZE) < 0
    ) {
        free(con);
        return NULL;
    }
     
    con->alive = 1;
    con->cfd = cfd;

    return con;

}

void connection_close(int epfd, struct connection *con) {

    if (!con->alive) {
        return;
    }

    con->alive = 0;

    if (epoll_ctl(epfd, EPOLL_CTL_DEL, con->cfd, NULL) < 0) {
        perror("epoll_ctl");
    }

    buffer_destroy(&con->read_buffer);
    buffer_destroy(&con->write_buffer);

    close(con->cfd);

}

void connection_destroy(struct connection *con) {

    free(con);

}