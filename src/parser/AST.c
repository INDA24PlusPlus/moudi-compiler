#include "parser/AST.h"
#include "common/common.h"
#include "common/list.h"
#include "common/logger.h"
#include "utils/slice.h"
#include "utils/string.h"
#include <stdlib.h>

#define PADDING_DIRECT_CHILD  "  "
#define PADDING_LIST_CHILDREN "  │"
#define AST_TREE_PADDING(comp) ((comp) ? PADDING_LIST_CHILDREN : PADDING_DIRECT_CHILD)
#define AST_TREE_PRINT_CHILDREN(list, pstring) for (int i = 0; i < (list).size; ++i){ _print_ast_tree(list_at(&list, i), pstring, 1, i+1 == (list).size);}

GENERATE_ENUM_TO_STR_FUNC_BODY(AST_type, AST_FOREACH);

struct AST * init_ast(enum AST_type type, struct AST * scope) {
    ASSERT1(scope != NULL);
    ASSERT1(scope->type == AST_SCOPE || scope->type == AST_ROOT);
    struct AST * ast = calloc(1, sizeof(struct AST));
    ast->type = type;
    ast->scope = scope;
    ast->value = init_ast_type_value(type);

    return ast;
}

union AST_value init_ast_type_value(enum AST_type type) {
    union AST_value value = {0};

    struct List ast_list_default = init_list(sizeof(struct AST));
    
    switch (type) {
        case AST_ROOT: 
            value.root = (struct a_root) {.nodes = ast_list_default}; break;
        case AST_SCOPE:
            value.scope = (struct a_scope) {.nodes = ast_list_default, .variables = ast_list_default}; break;
        case AST_EXPR:
            value.expr = (struct a_expr) {.children = ast_list_default}; break;
        case AST_IF:
            value._if = (struct a_if) {.bodies = ast_list_default, .conditions = ast_list_default}; break;
        default: break;
    }

    return value;
}

void free_ast(struct AST * ast) {
    memset(ast, 0, sizeof(*ast));
    free(ast);
}

void _print_ast_tree(struct AST *ast, String *pad, char is_list, char is_last) {
    if (is_list) {
        string_cut(pad, 1);
    }
    
    print("{2s}", pad->_ptr, pad->size == 0 ? "" : ((is_last || !is_list) ? "└─" : "├─"));
    if (ast == NULL) {
        println("(NULL)");
        return;
    }

    print_ast("{s}\n", ast);

    if (is_list) {
        if (is_last) {
            string_append(pad, " ");
        } else {
            string_append(pad, "│");
        }
    }

    switch (ast->type) {
        case AST_ROOT:
        {
            struct a_root * root = &ast->value.root;

            String * next_pad = string_copy(pad);
            string_append(next_pad, AST_TREE_PADDING(root->nodes.size > 1));
            AST_TREE_PRINT_CHILDREN(root->nodes, next_pad);
            free_string(&next_pad);
            
            break;
        }
        case AST_FUNCTION:
        {
            struct a_function * func = &ast->value.function;
            
            String * next_pad = string_copy(pad);
            string_append(next_pad, AST_TREE_PADDING(1));

            _print_ast_tree(func->arguments, next_pad, 1, 0);
            _print_ast_tree(func->body, next_pad, 1, 1);
            free_string(&next_pad);

            break;
        }
        case AST_SCOPE:
        {
            struct a_scope * scope = &ast->value.scope;
            String * next_pad = string_copy(pad);
            struct List list = list_combine(&scope->variables, &scope->nodes);

            string_append(next_pad, AST_TREE_PADDING(list.size > 1));
            AST_TREE_PRINT_CHILDREN(list, next_pad);
            free_string(&next_pad);
        } break;
#ifdef IMPL_PRINT
        case AST_IMPL:
        {
            struct a_impl * impl = ast->value;
            String * next_pad = string_copy(pad);
            
            string_append(next_pad, AST_TREE_PADDING(impl->members->size > 1));
            AST_TREE_PRINT_CHILDREN(impl->members, next_pad);
            free_string(&next_pad);

        } break;
#endif
        case AST_DECLARATION:
        {
            struct a_declaration * decl = &ast->value.declaration;
            
            String * next_pad = string_copy(pad);
            string_append(next_pad, PADDING_DIRECT_CHILD);
            _print_ast_tree(decl->expression, next_pad, 0, 0);
            free_string(&next_pad);

            break;
        }
        case AST_EXPR:
        {
            struct a_expr * expr = &ast->value.expr;

            String * next_pad = string_copy(pad);
            string_append(next_pad, AST_TREE_PADDING(expr->children.size > 1));
            AST_TREE_PRINT_CHILDREN(expr->children, next_pad);
            free_string(&next_pad);
            
            break;
        }
        case AST_OP:
        {
            struct a_op * op = &ast->value.op;

            String * next_pad = string_copy(pad);
            if (op->op.mode == BINARY) {
                string_append(next_pad, PADDING_LIST_CHILDREN);
                _print_ast_tree(op->left, next_pad, 1, 0);
                _print_ast_tree(op->right, next_pad, 1, 1);
            } else {
                string_append(next_pad, PADDING_DIRECT_CHILD);
                _print_ast_tree(op->right, next_pad, 0, 0);
            }
            free_string(&next_pad);

            break;
        }
        case AST_RETURN:
        {
            struct a_return * ret = &ast->value._return;

            if (ret->expr) {
                String * next_pad = string_copy(pad);
                string_append(next_pad, PADDING_DIRECT_CHILD);
                _print_ast_tree(ret->expr, next_pad, 0, 0);
                free_string(&next_pad);
            }

            break;
        }
        case AST_FOR:
        {
            struct a_for * for_statement = &ast->value._for;

            String * next_pad = string_copy(pad);
            string_append(next_pad, PADDING_LIST_CHILDREN);
            _print_ast_tree(for_statement->condition, next_pad, 1, 0);
            _print_ast_tree(for_statement->_continue, next_pad, 1, 0);
            _print_ast_tree(for_statement->body, next_pad, 1, 1);
            free_string(&next_pad);

            break;
        }
        case AST_IF:
        {
            struct a_if * if_statement = &ast->value._if;

            String * next_pad = string_copy(pad);
            string_append(next_pad, PADDING_LIST_CHILDREN);

            for (int i = 0; i < if_statement->bodies.size; ++i) {
                if (i < if_statement->conditions.size) {
                    _print_ast_tree(list_at(&if_statement->conditions, i), next_pad, 1, i + 1 == if_statement->conditions.size);
                }
                _print_ast_tree(list_at(&if_statement->bodies, i), next_pad, 1, i + 1 == if_statement->bodies.size);
            }
            
            free_string(&next_pad);

            break;
        }
    }
}

void print_ast_tree(struct AST * ast) {
    String * string = init_string("");
    _print_ast_tree(ast, string, 0, 0);
    free_string(&string);
}

const char * ast_type_to_str_ast(struct AST * ast) {
    if (ast == NULL) {
        return "(NULL)";
    }

    return AST_type_to_string(ast->type);
}

void print_ast(const char * template, struct AST * ast) {
	const char * type_str = AST_type_to_string(ast->type);
	const char * scope = ast_type_to_str_ast(ast->scope);
    
    char * prefix_str = format(RED "{s}" RESET ": ", type_str);
    char * formatted_str = NULL;

    switch (ast->type) {
        case AST_FUNCTION:
        {
            struct a_function * func = &ast->value.function;
            formatted_str = format("{s} " GREY "<" BLUE "Name" RESET ": {s}" GREY ">" RESET, prefix_str, func->name);
            break;
        }
        case AST_SCOPE:
        {
            struct a_scope * scope = &ast->value.scope;
            formatted_str = format("{s} " GREY "<" BLUE "Variables" RESET ": {i}, " BLUE "Nodes" RESET ": {i}" GREY ">" RESET, prefix_str, scope->variables.size, scope->nodes.size);
        } break;
        case AST_OP:
        {
            struct a_op * op = &ast->value.op;
            formatted_str = format("{s} " GREY "<" BLUE "Op" RESET ": '{s}', " BLUE "Mode" RESET ": {s}" GREY ">" RESET, prefix_str, op->op.str, op->op.mode == BINARY ? "Binary" : "Unary");
            break;
        }
        case AST_VARIABLE:
        {
            struct a_variable * var = &ast->value.variable;
            char * var_name = slice_to_string(&var->name);
            formatted_str = format("{s} " GREY "<" BLUE "Name" RESET ": {s}" GREY ">" RESET, prefix_str, var_name);
            free(var_name);
            break;
        }
        case AST_NUMBER:
        {
            struct a_number * number = &ast->value.number;
            char * number_value = slice_to_string(&number->value);
            formatted_str = format("{s} " GREY "<" BLUE "Value" RESET ": {s}" GREY ">" RESET, prefix_str, number_value);
            free(number_value);
            break;
        }
        case AST_DECLARATION:
        {
            struct a_declaration * declaration = &ast->value.declaration;
            formatted_str = format("{s} " GREY "<>" RESET, prefix_str);
            break;
        }
        default:
        {
            formatted_str = format("{s} " GREY "<>" RESET, prefix_str);
            break;
        }
    }

	print(template, formatted_str);
    free(prefix_str);
	free(formatted_str);
}
