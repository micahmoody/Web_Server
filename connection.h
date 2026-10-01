#ifndef CONNECTION_H

#define CONNECTION_H

#include <netinet/in.h>

#include "buffer.h"
#include "event.h"

struct connection {

    int cfd;

    int alive;

    struct buffer read_buffer;
    struct buffer write_buffer;

    struct event ev;

};

struct connection *connection_create(int cfd);

void connection_close(int epfd, struct connection *con);

void connection_destroy(struct connection *con);

#endif