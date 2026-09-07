#include <stdio.h>

#ifndef CONNECTION_H
#define CONNECTION_H

struct connection {
    int fd;

    char *read_buf;
    char *write_buf;

    int rs;
    int ws;

    int rp;
    int wp;
};

void disconnect(int epfd, struct connection *con);

#endif