#include "http-parse.h"

#ifndef HTTP_RESPONSE_H

#define HTTP_RESPONSE_H

struct connection; //avoid circular dependency by defining connection here instead of including the header file

enum http_response_resolve_state {
    HTTP_RESOLVE_NO_404,
    HTTP_RESOLVE_MALLOC_ERROR,
    HTTP_RESOLVE_OPEN_ERROR,
    HTTP_RESOLVE_SUCCESS
};

enum http_construct_headers_state {
    HTTP_CONSTRUCT_HEADERS_FSTAT_ERROR,
    HTTP_CONSTRUCT_HEADERS_SMALL_BUFFER,
    HTTP_CONSTRUCT_HEADERS_SUCCESS
};

struct http_content {
    int fp; //file fd position
    int ffd; //content file descriptor
    int fs; //file size
    char *path; //path to file
};

enum http_response_resolve_state resolve_target(struct connection *con, int not_found);
enum http_construct_headers_state construct_http_headers(struct connection *con);

#define HTTP_RESPONSE_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_RESPONSE_OK_LEN sizeof(HTTP_RESPONSE_OK) - 1 // -1 to not count \0 as part of the response

#define HTTP_RESPONSE_CONTENT_LEN_HEADER "Content-Length: "
#define HTTP_RESPONSE_CONTENT_LEN_HEADER_LEN sizeof(HTTP_RESPONSE_CONTENT_LEN_HEADER) - 1

#define HTTP_RESPONSE_CONTENT_TYPE_HEADER "Content-Type: "
#define HTTP_RESPONSE_CONTENT_TYPE_HEADER_LEN sizeof(HTTP_RESPONSE_CONTENT_TYPE_HEADER) - 1

#endif