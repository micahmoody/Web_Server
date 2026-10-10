#ifndef SERVER_H

#define SERVER_H

#include <stdint.h>
#include <stddef.h>

#include "event.h"

struct server_config {

    uint16_t port;

    int backlog;

    const char *document_root;
    
    const char *index_page;
    const char *http_403_page;
    const char *http_404_page;

    size_t max_request_size;
    size_t max_method_size;
    size_t max_target_size;

};

struct server {

    struct server_config config;

    struct event ev;

    int lfd;
    int efd;
    
};

int server_init(struct server *s, struct server_config *config);

int server_run(struct server *s);

void server_destroy(struct server *s);

#endif