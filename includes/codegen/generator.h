#pragma once
#include "parser/AST.h"

void generate_temp_variable_prefix();
void generate_number_variable(struct Slice * number);
void generate_variable(struct AST * ast);

void generate_binary_operator(const char * operator, size_t lhs, size_t rhs);
void generate_call(struct AST * ast);
void generate_op(struct AST * ast);

void generate_expr_node(struct AST * ast);
void generate_expression(struct AST * ast);

void generate_return(struct AST * ast);
void generate_for(struct AST * ast);
void generate_if(struct AST * ast);
void generate_declaration(struct AST * ast);

void generate_scope(struct AST * ast);
void generate_function(struct AST * ast);
void generate_qed(struct AST * ast);
