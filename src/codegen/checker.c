#include "codegen/checker.h"

#include "codegen/symbol_table.h"
#include "common/list.h"
#include "common/logger.h"
#include "fmt.h"
#include "parser/AST.h"
#include "parser/operators.h"
#include "utils/slice.h"
#include "codegen/intrinsics.h"

void checker_check_return(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_RETURN);
    struct a_return _return = ast->value._return;

    checker_check_expr(_return.expr);
}

void checker_check_op(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_OP);
    struct a_op op = ast->value.op;
    
    if (op.op.key == CALL) {
        if (op.left->type != AST_VARIABLE) {
            logger_log("Left operand must be a symbol", CHECKER, ERROR);
            exit(1);
        } else if (op.left->value.variable.name.start[0] == '#') {
            // check if it is a valid intrinsic
            struct Slice name = op.left->value.variable.name;
            if (strncmp(name.start, "#print", name.length) == 0) {
                ast->value.op.left->value.variable.name = init_slice(print_intrinsic_to_c_func, sizeof(print_intrinsic_to_c_func));
            } else {
                logger_log(format("Invalid intrinsic: \"{s}\"", slice_to_string(&name)), CHECKER, ERROR);
                exit(1);
            }

            ASSERT1(op.right->type == AST_EXPR);
            struct a_expr expr = op.right->value.expr;
            struct AST * node = NULL;

            // skip AST_STRING as these will be asserted and are only allowed here directly in intrinsics
            for (size_t i = 0; i < expr.children.size; ++i) {
                node = list_at(&expr.children, i);
                if (node->type != AST_STRING) {
                    checker_check_expr_node(node);
                }
            }

            return;
        }
    } else if (op.op.key == ASSIGNMENT) {
        if (op.left->type != AST_VARIABLE) {
            logger_log("LHS of assignment must be an immediate variable", CHECKER, ERROR);
            exit(1);
        }
    }

    if (op.left != NULL) {
        checker_check_expr_node(op.left);
    }
    checker_check_expr_node(op.right);
}

void checker_check_variable(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_VARIABLE);
    struct a_variable variable = ast->value.variable;

    if (!find_symbol_slice(ast->scope, &variable.name)) {
        logger_log(format("Unknown symbol '{s}'.\n" GREY "Tips: Did you forget to declare it as a variable?" RESET, slice_to_string(&variable.name)), CHECKER, ERROR);
    }
}

void checker_check_for(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_FOR);
    struct a_for _for = ast->value._for;

    checker_check_expr(_for.condition);
    if (_for._continue != NULL) {
        checker_check_expr(_for._continue);
    }
    checker_check_scope(_for.body);
}

void checker_check_if(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_IF);
    struct a_if _if = ast->value._if;

    for (size_t i = 0; i < _if.conditions.size; ++i) {
        checker_check_expr(list_at(&_if.conditions, i));
    }

    for (size_t i = 0; i < _if.bodies.size; ++i) {
        checker_check_scope(list_at(&_if.bodies, i));
    }
}

void checker_check_declaration(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_DECLARATION);
    struct a_declaration declaration = ast->value.declaration;

    symbol_table_add(&ast->scope->value.scope.sym_table, declaration.variable);
    checker_check_expr(declaration.expression);
}

void checker_check_expr_node(struct AST * ast) {
    ASSERT1(ast != NULL);
    switch (ast->type) {
        case AST_EXPR:
            checker_check_expr(ast); break;
        case AST_OP:
            checker_check_op(ast); break;
        case AST_VARIABLE:
            checker_check_variable(ast); break;
        case AST_NUMBER: break;
        case AST_STRING:
            logger_log("String literals are only allowed in intrinsic function calls", CHECKER, ERROR);
            exit(1);
        default:
            logger_log(format("Invalid expr node '{s}'", AST_type_to_string(ast->type)), CHECKER, ERROR);
            exit(1);
    }
}

void checker_check_expr(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_EXPR);
    struct a_expr expr = ast->value.expr;

    for (size_t i = 0; i < expr.children.size; ++i) {
        checker_check_expr_node(list_at(&expr.children, i));
    }

}

void checker_check_scope(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_SCOPE);
    struct a_scope * scope = &ast->value.scope;
    struct AST * node = NULL;

    for (size_t i = 0; i < scope->nodes.size; ++i) {
        node = list_at(&scope->nodes, i);
        switch (node->type) {
            case AST_EXPR:
                checker_check_expr(node); break;
            case AST_DECLARATION:
                checker_check_declaration(node); break;
            case AST_IF:
                checker_check_if(node); break;
            case AST_FOR:
                checker_check_for(node); break;
            case AST_RETURN:
                if (i + 1 != scope->nodes.size) {
                    logger_log("Return is only allowed at the end of a scope", CHECKER, ERROR);
                    exit(1);
                }
                scope->returns = 1;
                checker_check_return(node); break;
            default:
                logger_log(format("Invalid scope child '{s}'", AST_type_to_string(node->type)), CHECKER, ERROR);
                exit(1);
        }
    }
}

void checker_check_function(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_FUNCTION);
    
    struct a_function * func = &ast->value.function;
    ASSERT1(func->body->type == AST_SCOPE);
    struct a_scope * scope = &func->body->value.scope;

    for (size_t i = 0; i < func->arguments.size; ++i) {
        symbol_table_add(&scope->sym_table, list_at(&func->arguments, i));
    }

    checker_check_scope(func->body);

    if (!scope->returns) {
        logger_log(format("Function '{s}' must have a return statement before the end of scope", slice_to_string(&func->name)), CHECKER, ERROR);
        exit(1);
    }
}

void checker_check(struct AST * ast) {
    ASSERT1(ast != NULL);
    ASSERT1(ast->type == AST_ROOT);
    struct AST * node = NULL;
    struct a_root * root = &ast->value.root;
    root->sym_table = init_symbol_table();

    size_t length = root->nodes.size;
    // add all functions as symbols so that they can be called in any order
    for (size_t i = 0; i < length; ++i) {
        symbol_table_add(&root->sym_table, list_at(&root->nodes, i));
    }

    const char main_str[] = "main";
    if (!find_symbol(ast, "main", sizeof(main_str) / sizeof(char) - 1)) {
        logger_log("No main function entry point found!", CHECKER, ERROR);
        exit(1);
    }

    for (size_t i = 0; i < length; ++i) {
        checker_check_function(list_at(&root->nodes, i));
    }
}
