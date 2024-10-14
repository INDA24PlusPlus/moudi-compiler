#pragma once
#include "common/list.h"
#include "utils/slice.h"
#include "parser/AST.h"

void checker_check_declaration(struct AST * ast);
void checker_check_expr(struct AST * ast);
void checker_check_expr_node(struct AST * ast);
void checker_check_scope(struct AST * ast);
void checker_check_function(struct AST * ast);
void checker_check(struct AST * ast);
