#ifndef CONNECTION_H

#define CONNECTION_H

#include <netinet/in.h>

#include "buffer.h"

struct connection {

    int cfd;

    int alive;

    struct buffer read_buffer;
    struct buffer write_buffer;

    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;

};

#endif