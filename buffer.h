#ifndef BUFFER_H

#define BUFFER_H

#include <stddef.h>

struct buffer {
    char *data;
    size_t cap;
    size_t position;
    size_t data_end;
};

int buffer_resize(struct buffer *buf, size_t new_capacity);

int buffer_init(struct buffer *buf, size_t initial_capacity);

void buffer_compact(struct buffer *buf);

char *buffer_data(struct buffer *buf);

size_t buffer_available(struct buffer *buf);

void buffer_clear(struct buffer *buf);

void buffer_destroy(struct buffer *buf);

#endif