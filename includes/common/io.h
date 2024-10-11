#pragma once

#include "common/common.h"

FILE * open_file(const char * filepath, const char options[]);
char * read_file(const char * filepath, size_t * length);
