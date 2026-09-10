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

enum HTTP_REQUEST_STATE extract_headers(struct http_parser *dst, char *data, int len) {
    //do not read data[len] or further
    //len is the amount of bytes safe to read
    int header_start = get_end(data, len), header_end = -1;
    int borders[MAX_HEADER_COUNT]; // track \r\n indexes
    int del[MAX_HEADER_COUNT]; //track : indexes
    int spaces[MAX_HEADER_COUNT]; //track how many spaces after : before header value
    int whitespace = 0; //boolean: track whether to count spaces as whitespace or not
    memset(spaces, 0, sizeof(spaces));
    int border_len = 0, del_len = 0;

    int state = 1;
    // possible states:
    // 1: ready for border
    // 2: ready for a header name
    // 3: ready for a delimeter (:)
    // 4: ready for header value but not next border
    // then back around to 1

    for (int i = header_start; i < len - 3; i += 1) {
        if (header_end > 0 && i == header_end) {
            break;
        }
        if (data[i] == '\r' && data[i + 1] == '\n') {
            if (state != 1) {
                return ERR_MALFORMED_REQUEST;
            }
            if (data[i + 2] == '\r' && data[i + 3] == '\n') {
                header_end = i;
                continue;
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
                return ERR_MALFORMED_REQUEST;
            }
            del[del_len] = i;
            del_len += 1;
            state = 4;
            whitespace = 1;
            continue;
        }
        if (data[i] == ' ') {
            if (state != 4 && state != 1) {
                return ERR_MALFORMED_REQUEST;
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
    if (header_end < 0 || del_len != border_len) { //redundant? don't know
        return ERR_MALFORMED_REQUEST;
    }
    borders[border_len] = header_end;
    for (int i = 0; i < border_len; i += 1) {
        dst->headers[i].name.addr = data + borders[i] + 2;
        dst->headers[i].name.len = del[i] - borders[i] - 2;
        dst->headers[i].value.addr = data + del[i] + spaces[i] + 1;
        dst->headers[i].value.len = borders[i + 1] - del[i] - spaces[i] - 1;
    }
    return SUCCESS;
}

// int end = get_dbend(data, len);
    // if (end < 0) {
    //     return ERR_MALFORMED_REQUEST;
    // }
    // int last_delimeter = 0;
    // int next_delimeter;
    // while (1) {
    //     last_delimeter = next_delimeter;
    //     next_delimeter = get_end(data + next_delimeter, end - next_delimeter);
    //     if (next_delimeter > 0) {
    //         for (int i = 0; i < next_delimeter; i += 1) {
    //             if (data[last_delimeter + i] == ':') {
    //                 dst->header_count += 1;
    //                 dst->headers[dst->header_count].name.addr = data + last_delimeter + 2;
    //                 dst->headers[dst->header_count].name.len = i - last_delimeter - 2;
    //                 int j = 1;
    //                 while (1) {
    //                     if (data[i + j + 1] != ' ') {
    //                         break;
    //                     }
    //                     j += 1;
    //                 }
    //                 dst->headers[dst->header_count].value.addr = data + last_delimeter + i + j;
    //                 dst->headers[dst->header_count].value.len = next_delimeter - i - j;
    //                 break;
    //             }
    //         }
    //     } else {
    //         break;
    //     }
    // }
    // return SUCCESS;

enum HTTP_REQUEST_STATE extract_request(struct http_parser *dst, char *data, int len) {
    int found = 0;
    int space_indexes[2];
    int end = get_end(data, len);
    if (end < 0) {
        return INCOMPLETE_REQUEST;
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
                return SUCCESS;
            }
            found = 1;
        }
    }
    return ERR_MALFORMED_REQUEST;
}
