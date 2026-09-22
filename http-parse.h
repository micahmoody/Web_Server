#include "constants.h"

#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

enum HTTP_PARSE_STATE {
    HTTP_PARSE_ERR_MALFORMED_REQUEST,
    HTTP_PARSE_ERR_TOO_MANY_HEADERS,
    HTTP_PARSE_INCOMPLETE_REQUEST,
    HTTP_PARSE_SUCCESS
};

struct string {
    char *addr;
    int len;
};

struct header {
    struct string name;
    struct string value;
};

struct http_parser {
    struct string method;
    struct string target;
    struct string version;
    struct header headers[MAX_HEADER_COUNT];
    int header_count;
};

int get_end(char *start, int len);
int get_dbend(char *start, int len);
enum HTTP_PARSE_STATE extract_request_line(struct http_parser *dst, char *data, int len);
enum HTTP_PARSE_STATE extract_headers(struct http_parser *dst, char *data, int len);

#endif