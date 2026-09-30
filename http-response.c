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

struct response_code {
    int code;
    char *response;
};

struct response_code response_codes[] = {
    {200, HTTP_RESPONSE_OK},
    {400, HTTP_RESPONSE_BAD_REQUEST},
    {403, HTTP_RESPONSE_FORBIDDEN},
    {404, HTTP_RESPONSE_404},
    {405, HTTP_RESPONSE_BAD_METHOD},
    {413, HTTP_RESPONSE_413},
    {431, HTTP_RESPONSE_431},
    {500, HTTP_RESPONSE_500},
};

char *get_response(int code) {
    for (int i = 0; i < sizeof(response_codes)/sizeof(struct response_code); i += 1) {
        if (response_codes[i].code == code) {
            return response_codes[i].response;
        }
    }
    return NULL;
}

enum http_response_resolve_state resolve_target(struct connection *con, int not_found) { //call with not_found = 0
    if (not_found > 1) {
        return HTTP_RESOLVE_NO_404;
    }
    char *path;
    int root_len;

    if (not_found) {
        path = malloc(strlen(HTTP_404_FILE_PATH) + 1);
        if (path == NULL) {
            perror("malloc");
            return HTTP_RESOLVE_MALLOC_ERROR;
        }
        strcpy(path, HTTP_404_FILE_PATH);
    } else {
        if (con->hp.target.len == 1 && *con->hp.target.addr == '/') {
            path = malloc(strlen(DEFAULT_PATH) + 1);
            if (path == NULL) {
                perror("malloc");
                return HTTP_RESOLVE_MALLOC_ERROR;
            }
            strcpy(path, DEFAULT_PATH);
        } else {
            root_len = strlen(DOCUMENT_ROOT);
            path = malloc(root_len + con->hp.target.len + 1);
            if (path == NULL) {
                perror("malloc");
                return HTTP_RESOLVE_MALLOC_ERROR;
            }
            memcpy(path, DOCUMENT_ROOT, root_len);
            memcpy(path + root_len, con->hp.target.addr, con->hp.target.len);
            *(path + root_len + con->hp.target.len) = '\0';
        }
    }

    char *resolved = realpath(path, NULL);
    if (resolved == NULL) {
        int err = errno;
        free(path);
        if (err == ENOENT) {
            return resolve_target(con, not_found + 1);
        }
        perror("realpath");
        return HTTP_RESOLVE_REALPATH_ERROR;
    }
    char *resolved_docroot = realpath(DOCUMENT_ROOT, NULL);
    if (resolved_docroot == NULL) {
        int err = errno;
        free(path);
        free(resolved);
        if (err == ENOENT) {
            return HTTP_RESOLVE_NO_DOCROOT;
        }
        perror("realpath");
        return HTTP_RESOLVE_REALPATH_ERROR;
    }
    size_t rdocroot_len = strlen(resolved_docroot);
    for (size_t i = 0; i < rdocroot_len; i += 1) {
        if (resolved[i] != resolved_docroot[i]) {
            free(resolved);
            free(resolved_docroot);
            free(path);
            return HTTP_RESOLVE_PATH_ESCAPE;
        }
    }
    if (resolved[rdocroot_len] != '/') {
        free(resolved);
        free(resolved_docroot);
        free(path);
        return HTTP_RESOLVE_PATH_ESCAPE;
    }
    free(resolved);
    free(resolved_docroot);

    int target = open(path, O_RDONLY);
    if (target < 0) {
        perror("open");
        free(path);
        return HTTP_RESOLVE_OPEN_ERROR;
    }

    con->content.path = path;
    con->content.ffd = target;
    return not_found ? HTTP_RESOLVE_404 : HTTP_RESOLVE_SUCCESS;
}

enum http_construct_headers_state construct_http_headers(struct connection *con, int code) {
    struct stat st;
    int path_len, index, n;
    const char *mt;
    if (code != 403) {
        if (fstat(con->content.ffd, &st) < 0) {
            perror("fstat");
            return HTTP_CONSTRUCT_HEADERS_FSTAT_ERROR;
        }
        path_len = strlen(con->content.path);
        index = get_extension_index(con->content.path, path_len);
        if (index < 0) {
            mt = FALLBACK_MIME_TYPE;
        } else {
            mt = get_mime_type(con->content.path + index, path_len - index);
        }
        n = snprintf(con->write_buf, con->ws, 
            "%s\r\n%s%ld\r\n%s%s\r\n\r\n", 
            get_response(code), 
            HTTP_RESPONSE_CONTENT_LEN_HEADER, st.st_size,
            HTTP_RESPONSE_CONTENT_TYPE_HEADER, mt
        );
    } else {
        n = snprintf(con->write_buf, con->ws,
            "%s\r\n%s0\r\n\r\n",
            get_response(code),
            HTTP_RESPONSE_CONTENT_LEN_HEADER
        );
    }
    if (n >= con->ws) {
        return HTTP_CONSTRUCT_HEADERS_SMALL_BUFFER;
    }
    con->wl = n; //number of bytes written minus null terminator
    con->wp = 0;
    con->content.fs = st.st_size;
    return HTTP_CONSTRUCT_HEADERS_SUCCESS;
}