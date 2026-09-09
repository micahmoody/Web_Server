#include "http-parse.h"

int get_end(char *start, int len) {
    for (int i = 0; i < len - 1; i += 1) {
        if (start[i] == '\r' && start[i + 1] == '\n') {
            return i;
        }
    }
    return -1;
}

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
