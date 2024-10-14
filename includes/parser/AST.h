#pragma once

#include "codegen/symbol_table.h"
#include "common/common.h"
#include "utils/slice.h"
#include "operators.h"
#include "common/list.h"
#include "utils/macro.h"

struct a_root {
    struct List nodes;
    struct symbol_table sym_table;
};

struct a_function {
    struct Slice name;
    struct AST * body;
    struct List arguments;
};

struct a_scope {
    struct List nodes;
    struct symbol_table sym_table;
};

struct a_declaration {
    struct AST * variable;
    struct AST * expression;
};

struct a_expr {
    struct List children;
};

struct a_op {
    struct Operator op;
    struct AST * left;
    struct AST * right;
};

struct a_variable {
    struct Slice name;
};

struct a_number {
    struct Slice value;
};

struct a_for {
    struct AST * condition;
    struct AST * _continue;
    struct AST * body;
};

struct a_return {
    struct AST * expr;
};

struct a_if {
    struct List conditions;
    struct List bodies;
};


#ifndef AST_FOREACH
#define AST_FOREACH(f) \
    f(AST_UNKNOWN) \
    f(AST_ROOT) \
    f(AST_FUNCTION) \
    f(AST_SCOPE) \
    f(AST_DECLARATION) \
    f(AST_EXPR) \
    f(AST_OP) \
    f(AST_VARIABLE) \
    f(AST_NUMBER) \
    f(AST_FOR) \
    f(AST_RETURN) \
    f(AST_IF) \

GENERATE_ENUM(AST_type, AST_FOREACH);
GENERATE_ENUM_TO_STR_FUNC(AST_type);
#endif

struct AST {
    struct AST * scope;
    union AST_value {
        struct a_root root;
        struct a_function function;
        struct a_scope scope;
        struct a_declaration declaration;
        struct a_expr expr;
        struct a_op op;
        struct a_variable variable;
        struct a_number number;
        struct a_for _for;
        struct a_if _if;
        struct a_return _return;
    } value;

    enum AST_type type;
};

struct AST * init_ast(enum AST_type type, struct AST * scope);
union AST_value init_ast_type_value(enum AST_type type);
void free_ast(struct AST * ast);

char * ast_to_string(struct AST * ast);
void print_ast_tree(struct AST * ast);
