#ifndef CONNECTION_H
#define CONNECTION_H

struct connection {
    int fd;
    char *read_buf;
    char *write_buf;
    int wp;
    int rp;
};

#endif