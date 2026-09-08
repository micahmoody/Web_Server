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

struct http_parser {
    enum HTTP_PARSER_STATE state;
    int p;
};

#endif