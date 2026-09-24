#include <stdio.h>

struct mime_type {
    const char *extension;
    const char *type;
};

static const struct mime_type mime_types[] = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "text/javascript"},
    {".txt", "text/plain"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".gif", "image/gif"}
};

const char *get_mime_type(char *ext) {
    int equals, j;
    for (int i = 0; i < sizeof(mime_types)/sizeof(struct mime_type); i += 1) {
        equals = 1;
        j = 0;
        while (1) {
            if (ext[j] != mime_types[i].extension[j]) {
                equals = 0;
                break;
            }
            if (mime_types[i].extension[j] == '\0') {
                break;
            }
            j += 1;
        }
        if (equals) {
            return mime_types[i].type;
        }
    }
    return NULL;
}