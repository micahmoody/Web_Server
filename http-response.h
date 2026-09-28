#include <sys/sendfile.h>
#include <sys/types.h>
#include "http-parse.h"

#ifndef HTTP_RESPONSE_H

#define HTTP_RESPONSE_H

struct connection; //avoid circular dependency by defining connection here instead of including the header file

enum http_response_resolve_state {
    HTTP_RESOLVE_NO_404,
    HTTP_RESOLVE_MALLOC_ERROR,
    HTTP_RESOLVE_OPEN_ERROR,
    HTTP_RESOLVE_SUCCESS,
    HTTP_404
};

enum http_construct_headers_state {
    HTTP_CONSTRUCT_HEADERS_FSTAT_ERROR,
    HTTP_CONSTRUCT_HEADERS_SMALL_BUFFER,
    HTTP_CONSTRUCT_HEADERS_SUCCESS
};

struct http_content {
    off_t fp; //file offset
    int ffd; //content file descriptor
    off_t fs; //file size
    char *path; //path to file
};

enum http_response_resolve_state resolve_target(struct connection *con, int not_found);
enum http_construct_headers_state construct_http_headers(struct connection *con, int code);

#define HTTP_RESPONSE_OK "HTTP/1.1 200 OK"
#define HTTP_RESPONSE_OK_LEN sizeof(HTTP_RESPONSE_OK) - 1 // -1 to not count \0 as part of the response

#define HTTP_RESPONSE_404 "HTTP/1.1 404 Not Found"

#define HTTP_RESPONSE_CONTENT_LEN_HEADER "Content-Length: "
#define HTTP_RESPONSE_CONTENT_LEN_HEADER_LEN sizeof(HTTP_RESPONSE_CONTENT_LEN_HEADER) - 1

#define HTTP_RESPONSE_CONTENT_TYPE_HEADER "Content-Type: "
#define HTTP_RESPONSE_CONTENT_TYPE_HEADER_LEN sizeof(HTTP_RESPONSE_CONTENT_TYPE_HEADER) - 1

#endif