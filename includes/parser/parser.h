#pragma once

#include "common/common.h"
#include "parser/lexer.h"
#include "parser/AST.h"
#include "parser/operators.h"

struct Parser {
    struct AST * root;
    struct AST * current_scope;
    struct Lexer lexer;
};

static inline char parser_token_is_type(struct Parser * parser, enum token_t expected_type);
void parser_eat(struct Parser * parser, enum token_t expected_type);

struct AST * parser_parse_expr_exit_on(struct Parser * parser, enum Operators op);
struct AST * parser_parse_expr(struct Parser * parser);

struct AST * parser_parse_variable(struct Parser * parser);
struct AST * parser_parse_number(struct Parser * parser);
struct AST * parser_parse_operator(struct Parser * parser);
struct AST * parser_parse_scope(struct Parser * parser);

struct Parser init_parser(char * filepath);
struct AST * parser_parse(char * filepath);

void parser_parse_root(struct Parser * parser);
