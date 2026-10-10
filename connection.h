#ifndef CONNECTION_H

#define CONNECTION_H

#include <netinet/in.h>

#include "buffer.h"
#include "event.h"
#include "http-parse.h"

struct connection {

    int cfd;

    int alive;

    struct buffer read_buffer;
    struct buffer write_buffer;

    struct http_parser parser;

    struct event ev;

};

enum connection_read_result {
    CONNECTION_READ_ERR,
    CONNECTION_READ_CLOSE,
    CONNECTION_READ_DRAINED
};

struct connection *connection_create(int cfd);

int connection_handle_event(int epfd, struct connection *con, uint32_t events);

// pass epfd == -1 for connection_close to not attempt deleting cfd from epoll
void connection_close(int epfd, struct connection *con);

void connection_destroy(struct connection *con);

#endif