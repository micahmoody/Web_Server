#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "connection.h"

void disconnect(int epfd, struct connection *con) {
    close(con -> fd);
    free(con -> read_buf);
    free(con);
}