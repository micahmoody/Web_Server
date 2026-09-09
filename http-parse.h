#include <stdio.h>

#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

enum HTTP_REQUEST_STATE {
    ERR_MALFORMED_REQUEST,
    INCOMPLETE_REQUEST,
    SUCCESS
};

struct string {
    char *addr;
    int len;
};

struct http_parser {
    struct string method;
    struct string target;
    struct string version;
};

int get_end(char *start, int len);
int get_dbend(char *start, int len);
enum HTTP_REQUEST_STATE extract_request(struct http_parser *dst, char *data, int len);

#endif