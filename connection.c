#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

#include "connection.h"
#include "buffer.h"

#define INIT_READBUF_SIZE 4096
#define READBUF_REALLOC_FACTOR 2

#define INIT_WRITEBUF_SIZE 4096

struct connection *connection_create(int cfd) {

    struct connection *con = malloc(sizeof(struct connection));

    if (con == NULL) {
        perror("malloc");
        return NULL;
    }

    if (buffer_init(&con->read_buffer, INIT_READBUF_SIZE) < 0) {
        free(con);
        return NULL;
    }

    if (buffer_init(&con->write_buffer, INIT_WRITEBUF_SIZE) < 0) {
        buffer_destroy(&con->read_buffer);
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

    if (epfd != -1) {
        if (epoll_ctl(epfd, EPOLL_CTL_DEL, con->cfd, NULL) < 0) {
            perror("epoll_ctl");
        }
    }

    buffer_destroy(&con->read_buffer);
    buffer_destroy(&con->write_buffer);

    close(con->cfd);

}

enum connection_read_result connection_read(struct connection *con) {

    while (1) {

        ssize_t n = read(con->cfd, buffer_addr(&con->read_buffer), buffer_available(&con->read_buffer));
        if (n > 0) {

            buffer_produce(&con->read_buffer, n);
            if (buffer_available(&con->read_buffer) == 0) {
                buffer_compact(&con->read_buffer);
                if (buffer_available(&con->read_buffer) == 0) {
                    if (buffer_resize(&con->read_buffer, con->read_buffer.cap * READBUF_REALLOC_FACTOR) < 0) {
                        return CONNECTION_READ_ERR;
                    }
                }
            }

            printf("read %zd bytes, buffer contains %zu bytes\n",
                n, con->read_buffer.data_end);
            fflush(stdout);

        } else if (n == 0) {

            return CONNECTION_READ_CLOSE;

        } else {

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return CONNECTION_READ_DRAINED;
            }

            perror("read");
            return CONNECTION_READ_ERR;

        }

    }

}

int connection_handle_event(int epfd, struct connection *con, uint32_t events) {

    if (!con->alive) {
        return 0;
    }

    if (events & EPOLLIN) {

        enum connection_read_result result = connection_read(con);
        switch (result) {
            case CONNECTION_READ_DRAINED:

                break;

            case CONNECTION_READ_CLOSE:

                connection_close(epfd, con);
                return 0;

            case CONNECTION_READ_ERR:

                connection_close(epfd, con);
                return -1;
        }

    }

    if (events & EPOLLOUT) {
        //con writable
    }

    if (events & EPOLLERR) {
        // err
    }

    if (events & EPOLLHUP) {
        // hangup
    }

    return 0;

}

void connection_destroy(struct connection *con) {

    free(con);

}