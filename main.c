#include <stdlib.h>

#include "server.h"

int main() {
    struct server_config config = {
        .port = 8080,
        .backlog = 15
    };

    struct server s;

    int n = server_init(&s, &config);

    if (n < 0) {
        exit(1);
    }

    server_destroy(&s);
}