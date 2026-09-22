#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "constants.h"
#include "connection.h"
#include "http-response.h"

enum http_response_resolve_state resolve_target(struct http_parser *hp, char **dst_path, int *ffd) {
    int root_len = strlen(DOCUMENT_ROOT);
    char *path = malloc(root_len + hp->target.len + 1); //this must be freed after function call unless error
    if (path == NULL) {
        perror("malloc");
        return HTTP_RESOLVE_MALLOC_ERROR;
    }
    sprintf(path, "%s", DOCUMENT_ROOT);
    memcpy(path + root_len, hp->target.addr, hp->target.len); //this is vulnerable to path traversal e.g. ../../../etc/passwd
    *(path + root_len + hp->target.len) = '\0';
    int target = open(path, O_RDONLY); //this must be closed after function call unless error
    if (target < 0) {
        free(path);
        if (errno == ENOENT) {
            return HTTP_RESOLVE_NOT_FOUND;
        }
        perror("open");
        return HTTP_RESOLVE_OPEN_ERROR;
    }
    *dst_path = path;
    *ffd = target;
    return HTTP_RESOLVE_SUCCESS;
}

enum http_construct_headers_state construct_http_headers(struct connection *con, int ffd) {
    struct stat st;
    if (fstat(ffd, &st) < 0) {
        return HTTP_CONSTRUCT_HEADERS_FSTAT_ERROR;
    }
    int n = snprintf(con->write_buf, con->ws, 
        "%s\r\n%s%ld\r\n\r\n", 
        HTTP_RESPONSE_OK, HTTP_RESPONSE_CONTENT_LEN_HEADER, st.st_size
    );
    if (n < con->ws) {
        return HTTP_CONSTRUCT_HEADERS_SMALL_BUFFER;
    }
    con->wl = n; //number of bytes written minus null terminator
    return HTTP_CONSTRUCT_HEADERS_SUCCESS;
}