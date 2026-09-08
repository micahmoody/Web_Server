#include <stdio.h>
#include "http-parse.h"

#ifndef CONNECTION_H
#define CONNECTION_H

struct connection {
    int fd;

    struct http_parser hp;

    char *read_buf;
    char *write_buf;

    int rs;
    int ws;

    int rp;
    int wp;
};

int disconnect(int epfd, struct connection *con);

#endif