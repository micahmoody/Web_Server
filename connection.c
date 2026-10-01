#include <stdlib.h>
#include <netinet/in.h>

#include "connection.h"
#include "buffer.h"

#define INIT_READBUF_SIZE 4096
#define INIT_WRITEBUF_SIZE 4096

struct connection *connection_create(int cfd) {

    struct connection *con = malloc(sizeof(struct connection));

    if (
        buffer_init(&con->read_buffer, INIT_READBUF_SIZE) < 0 ||
        buffer_init(&con->write_buffer, INIT_WRITEBUF_SIZE) < 0
    ) {
        free(con);
        return NULL;
    }
     
    con->alive = 1;
    con->cfd = cfd;
    con->client_addr_len = sizeof(struct sockaddr_storage);

    return con;

}