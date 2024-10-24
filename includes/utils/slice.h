#pragma once
#include "common/common.h"

struct Slice {
    const char * start;
    size_t length;
};

struct Slice init_slice(const char * start, size_t length);
struct Slice * copy_slice(struct Slice);
char * slice_to_string(struct Slice * slice);
