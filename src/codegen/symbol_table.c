#include "codegen/symbol_table.h"
#include "common/common.h"
#include "common/list.h"
#include "common/logger.h"
#include "fmt.h"
#include "parser/AST.h"
#include "utils/slice.h"
#include <stdlib.h>
#include <string.h>

struct symbol_table init_symbol_table() {
    return (struct symbol_table) {.list = init_list(sizeof(struct symbol_entry *))};
}

struct symbol_entry * init_symbol_entry(struct Slice * slice) {
    struct symbol_entry * entry = malloc(sizeof(struct symbol_entry));
    *entry = (struct symbol_entry) {.ID = slice};
    return entry;
}

void symbol_table_add(struct symbol_table * sym_table, struct AST * item) {
    ASSERT1(sym_table != NULL && item != NULL);
    struct Slice slice;

    switch (item->type) {
        case AST_VARIABLE:
            slice = item->value.variable.name;
            break;
        case AST_FUNCTION:
            slice = item->value.function.name;
            break;
        default:
            logger_log(format("Invalid symbol item type: '{s}'", AST_type_to_string(item->type)), CHECKER, ERROR);
            exit(1);
    }

    list_push(&sym_table->list, init_symbol_entry(copy_slice(slice)));
}

char find_symbol(struct AST * scope, struct Slice * cmp) {
    ASSERT1(scope->type == AST_SCOPE || scope->type == AST_ROOT);
    struct symbol_table table;
    if (scope->type == AST_SCOPE) {
        table = scope->value.scope.sym_table;
    } else {
        table = scope->value.root.sym_table;
    }

    struct Slice * slice = NULL;

    for (size_t i = 0; i < table.list.size; ++i) {
        slice = ((struct symbol_entry *) list_at(&table.list, i))->ID;
        if (slice->length == cmp->length && strncmp(slice->start, cmp->start, slice->length) == 0) {
            return 1;
        }
    }
    
    if (scope->type == AST_SCOPE) {
        return find_symbol(scope->scope, cmp);
    }

    return 0;
}
