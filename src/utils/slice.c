#include "utils/slice.h"
#include <stdlib.h>
#include <string.h>

struct Slice init_slice(const char * start, size_t length) {
    struct Slice slice = {.start = start, .length = length};
    return slice;
}

struct Slice * copy_slice(struct Slice src) {
    struct Slice * dest = malloc(sizeof(src));
    memcpy(dest, &src, sizeof(src));
    return dest;
}

char * slice_to_string(struct Slice * slice) {
    char * str = malloc(sizeof(char) * (slice->length + 1));
    
    memcpy(str, slice->start, slice->length);
    str[slice->length] = '\0';

    return str;
}
