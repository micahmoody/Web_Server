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
#include "http-parse.h"
#include "mime-types.h"

struct connection;

enum http_response_resolve_state resolve_target(struct connection *con, int not_found) { //call with not_found = 0
    if (not_found > 1) {
        return HTTP_RESOLVE_NO_404;
    }
    int root_len = strlen(DOCUMENT_ROOT);
    char *path = malloc(not_found ? strlen(HTTP_404_FILE_PATH) + 1 : (root_len + con->hp.target.len + 1)); //this must be freed after function call unless error
    if (path == NULL) {
        perror("malloc");
        return HTTP_RESOLVE_MALLOC_ERROR;
    }
    if (not_found) {
        sprintf(path, "%s", HTTP_404_FILE_PATH);
    } else {
        memcpy(path, DOCUMENT_ROOT, root_len);
        memcpy(path + root_len, con->hp.target.addr, con->hp.target.len); //this is vulnerable to path traversal e.g. ../../../etc/passwd
        *(path + root_len + con->hp.target.len) = '\0';
    }
    int target = open(path, O_RDONLY);
    if (target < 0) {
        int err = errno;
        free(path);
        if (err == ENOENT) {
            return resolve_target(con, not_found + 1);
        }
        perror("open");
        return HTTP_RESOLVE_OPEN_ERROR;
    }
    con->content.path = path;
    con->content.ffd = target;
    return HTTP_RESOLVE_SUCCESS;
}

enum http_construct_headers_state construct_http_headers(struct connection *con) {
    struct stat st;
    if (fstat(con->content.ffd, &st) < 0) {
        perror("fstat");
        return HTTP_CONSTRUCT_HEADERS_FSTAT_ERROR;
    }
    int path_len = strlen(con->content.path);
    int index = get_extension_index(con->content.path, path_len);
    const char *mt;
    if (index < 0) {
        mt = FALLBACK_MIME_TYPE;
    } else {
        mt = get_mime_type(con->content.path + index, path_len - index);
    }
    int n = snprintf(con->write_buf, con->ws, 
        "%s\r\n%s%ld\r\n%s%s\r\n\r\n", 
        HTTP_RESPONSE_OK, 
        HTTP_RESPONSE_CONTENT_LEN_HEADER, st.st_size,
        HTTP_RESPONSE_CONTENT_TYPE_HEADER, mt
    );
    if (n >= con->ws) {
        return HTTP_CONSTRUCT_HEADERS_SMALL_BUFFER;
    }
    con->wl = n; //number of bytes written minus null terminator
    con->wp = 0;
    return HTTP_CONSTRUCT_HEADERS_SUCCESS;
}