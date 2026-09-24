#include <stdio.h>
#include <string.h>
#include "http-parse.h"
#include "constants.h"

int get_end(char *start, int len) {
    if (len < 2) {
        return -1;
    }
    for (int i = 0; i < len - 1; i += 1) {
        if (start[i] == '\r' && start[i + 1] == '\n') {
            return i;
        }
    }
    return -1;
}

int get_dbend(char *start, int len) {
    if (len < 4) {
        return -1;
    }
    for (int i = 0; i < len - 3; i += 1) {
        if (start[i] == '\r' && start[i + 1] == '\n' && start[i + 2] == '\r' && start[i + 3] == '\n') {
            return i;
        }
    }
    return -1;
}

enum HTTP_PARSE_STATE extract_headers(struct http_parser *dst, char *data, int len) {
    int header_start = get_end(data, len), header_end = -1;
    int borders[MAX_HEADER_COUNT];
    int del[MAX_HEADER_COUNT];
    int spaces[MAX_HEADER_COUNT];
    int whitespace = 0;
    memset(spaces, 0, sizeof(spaces));
    int border_len = 0, del_len = 0;

    int state = 1;
    // possible states:
    // 1: ready for border
    // 2: ready for a header name
    // 3: ready for a delimeter (:)
    // 4: ready for header value but not next border
    // ^this should have been an enum
    for (int i = header_start; i < len - 3; i += 1) {
        printf("%c", data[i]);
        if (data[i] == '\r' && data[i + 1] == '\n') {
            if (state != 1) {
                return HTTP_PARSE_ERR_MALFORMED_REQUEST;
            }
            if (border_len >= MAX_HEADER_COUNT) {
                return HTTP_PARSE_ERR_TOO_MANY_HEADERS;
            }
            if (data[i + 2] == '\r' && data[i + 3] == '\n') {
                header_end = i;
                break;
            }
            borders[border_len] = i;
            border_len += 1;
            state = 2;
            continue;
        }
        if (data[i] == ':') {
            if (state == 4 || state == 1) {
                continue;
            }
            if (state != 3) {
                return HTTP_PARSE_ERR_MALFORMED_REQUEST;
            }
            if (del_len >= MAX_HEADER_COUNT) {
                return HTTP_PARSE_ERR_TOO_MANY_HEADERS;
            }
            del[del_len] = i;
            del_len += 1;
            state = 4;
            whitespace = 1;
            continue;
        }
        if (data[i] == ' ') {
            if (state != 4 && state != 1) {
                return HTTP_PARSE_ERR_MALFORMED_REQUEST;
            }
            if (whitespace) {
                spaces[del_len - 1] += 1;
            }
            continue;
        }
        
        if (state == 2) {
            state = 3;
        }
        if (state == 4) {
            state = 1;
        }
        whitespace = 0;
    }
    if (header_end < 0 || del_len != border_len) {
        return HTTP_PARSE_ERR_MALFORMED_REQUEST;
    }
    borders[border_len] = header_end;
    for (int i = 0; i < border_len; i += 1) {
        dst->headers[i].name.addr = data + borders[i] + 2;
        dst->headers[i].name.len = del[i] - borders[i] - 2;
        dst->headers[i].value.addr = data + del[i] + spaces[i] + 1;
        dst->headers[i].value.len = borders[i + 1] - del[i] - spaces[i] - 1;
    }
    return HTTP_PARSE_SUCCESS;
}

enum HTTP_PARSE_STATE extract_request_line(struct http_parser *dst, char *data, int len) {
    int found = 0;
    int space_indexes[2];
    int end = get_end(data, len);
    if (end < 0) {
        return HTTP_PARSE_INCOMPLETE_REQUEST;
    }
    for (int i = 0; i < end; i += 1) {
        if (data[i] == ' ') {
            space_indexes[found] = i;
            if (found == 1) {
                dst->method.addr = data;
                dst->method.len = space_indexes[0];

                dst->target.addr = data + space_indexes[0] + 1;
                dst->target.len = space_indexes[1] - space_indexes[0] - 1;

                dst->version.addr = data + space_indexes[1] + 1;
                dst->version.len = end - space_indexes[1] - 1;
                return HTTP_PARSE_SUCCESS;
            }
            found = 1;
        }
    }
    return HTTP_PARSE_ERR_MALFORMED_REQUEST;
}

int get_extension_index(char *target, int len) { // pass -1 to use \0 as end of string, otherwise pass len
    int i = 0, last_period = -1;
    if (len < 0) {
        while (1) {
            if (target[i] == '.') {
                last_period = i;
                i += 1;
                continue;
            }
            if (target[i] == '\0') {
                break;
            }
            i += 1;
        }
    } else {
        while (1) {
            if (i == len) {
                break;
            }
            if (target[i] == '.') {
                last_period = i;
                i += 1;
                continue;
            }
            i += 1;
        }
    }
    return last_period;
}