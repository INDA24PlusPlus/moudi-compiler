#pragma once
#include "common/common.h"
#include "common/list.h"
#include "common/logger.h"


struct symbol_entry {
    struct Slice * ID;
};

struct symbol_table {
    struct List list;
};

struct AST; // forward definition to avoid circular inclusion

struct symbol_table init_symbol_table();
void symbol_table_add(struct symbol_table * sym_table, struct AST * item);

char find_symbol(struct AST * scope, char * cmp, size_t length);
char find_symbol_slice(struct AST * scope, struct Slice * cmp);
