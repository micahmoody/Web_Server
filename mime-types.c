#include "constants.h"

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

const char *get_mime_type(char *ext, int len) {
    int j;
    for (int i = 0; i < sizeof(mime_types)/sizeof(struct mime_type); i += 1) {
        j = 0;
        while (mime_types[i].extension[j] != '\0') {
            if (j >= len || ext[j] != mime_types[i].extension[j]) {
                break;
            }
            j += 1;
        }
        if (mime_types[i].extension[j] == '\0' && j == len) {
            return mime_types[i].type;
        }
    }
    return FALLBACK_MIME_TYPE;
}