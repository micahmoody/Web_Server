#include <stdio.h>
#include "http-parse.h"
#include "http-response.h"

#ifndef CONNECTION_H
#define CONNECTION_H

struct connection {
    int fd;

    struct http_parser hp;
    struct http_content content;

    char *read_buf;
    char *write_buf;
    int wl; //length of content in write buffer

    int rs; //read buffer size
    int ws; //write buffer size

    int rp; //read buffer position
    int wp; //write buffer position
};

int disconnect(int epfd, struct connection *con);

#endif