#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "connection.h"

int disconnect(int epfd, struct connection *con) {
    int err = 0;
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, con -> fd, NULL) < 0) {
        perror("epoll_ctl");
        err = 1;
    }
    close(con -> fd);
    free(con -> read_buf);
    free(con);
    return err;
}