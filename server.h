#include <stdint.h>

#ifndef SERVER_H

#define SERVER_H

struct server_config {

    uint16_t port;

    int backlog;

    const char *document_root;

    const char *http_403_page;
    const char *http_404_page;

};

struct server {

    struct server_config *config;

    int lfd;
    int efd;
    
};

int server_init(struct server *s, struct server_config *config);

#endif