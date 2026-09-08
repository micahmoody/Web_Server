#include <stdio.h>

#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

enum HTTP_PARSER_STATE {
    REQUEST_LINE,
    HEADERS,
    BODY,
    COMPLETE,
    ERROR
};

struct string {
    char *addr;
    int len;
};

struct http_parser {
    enum HTTP_PARSER_STATE state;
    int p;

    struct string method;
    struct string target;
    struct string version;
};

int get_end(char *start, int len);
int extract_request(struct http_parser *dst, char *data, int len);

#endif