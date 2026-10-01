#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"

int buffer_resize(struct buffer *buf, size_t new_capacity) {
    char *ptr = realloc(buf->data, new_capacity);
    if (ptr == NULL) {
        perror("realloc");
        return -1;
    }
    buf->data = ptr;
    buf->cap = new_capacity;
    return 0;
}

int buffer_init(struct buffer *buf, size_t initial_capacity) {
    char *ptr = malloc(initial_capacity);
    if (ptr == NULL) {
        perror("malloc");
        return -1;
    }
    buf->data = ptr;
    buf->cap = initial_capacity;
    buf->position = 0;
    buf->data_end = 0;
    return 0;
}

void buffer_compact(struct buffer *buf) {
    memmove(buf->data, buf->data + buf->position, buf->data_end - buf->position);
    buf->data_end -= buf->position;
    buf->position = 0;
}

char *buffer_data(struct buffer *buf) {
    return buf->data + buf->position;
}

size_t buffer_available(struct buffer *buf) {
    return buf->cap - buf->data_end;
}

void buffer_clear(struct buffer *buf) {
    buf->position = 0;
    buf->data_end = 0;
}

void buffer_destroy(struct buffer *buf) {
    free(buf->data);
    buf->data = NULL;
    buf->data_end = buf->cap = buf->position = 0;
}