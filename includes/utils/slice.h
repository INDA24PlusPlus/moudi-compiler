#pragma once
#include "common/common.h"

struct Slice {
    char * start;
    size_t length;
};

struct Slice init_slice(char * start, size_t length);
struct Slice * copy_slice(struct Slice);
char * slice_to_string(struct Slice * slice);
