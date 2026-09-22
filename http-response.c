#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "constants.h"
#include "connection.h"
#include "http-response.h"

enum http_response_resolve_state resolve_target(struct http_parser *hp, char **dst_path) {
    int root_len = strlen(DOCUMENT_ROOT);
    char *path = malloc(root_len + hp->target.len + 1);
    if (path == NULL) {
        perror("malloc");
        return HTTP_RESOLVE_MALLOC_ERROR;
    }
    sprintf(path, "%s", DOCUMENT_ROOT);
    memcpy(path + root_len, hp->target.addr, hp->target.len); //this is vulnerable to path traversal
    *(path + root_len + hp->target.len) = '\0';
    int target = open(path, O_RDONLY);
    if (target < 0) {
        if (errno == ENOENT) {
            return HTTP_RESOLVE_NOT_FOUND;
        }
        perror("open");
        return HTTP_RESOLVE_OPEN_ERROR;
    }
    close(target);
    *dst_path = path;
    return HTTP_RESOLVE_SUCCESS;
}