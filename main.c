#include <stdlib.h>

#include "server.h"

int main() {

    struct server_config config = {

        .port = 8080,
        .backlog = 15,

        .index_page = "index.html",
        .http_404_page = "notfound.html",
        .http_403_page = "forbidden.html",

        .max_request_size = 65536,
        .max_method_size = 12,
        .max_target_size = 4096

    };

    struct server s;

    int n = server_init(&s, &config);
    if (n < 0) {
        exit(1);
    }

    n = server_run(&s);
    if (n < 0) {
        exit(1);
    }

    server_destroy(&s);

    return 0;

}