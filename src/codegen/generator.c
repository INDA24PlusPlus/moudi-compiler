#include "codegen/generator.h"

#include "common/common.h"
#include "common/io.h"
#include "common/list.h"
#include "common/logger.h"
#include "fmt.h"
#include "parser/AST.h"
#include "parser/operators.h"
#include "utils/slice.h"
#include "utils/string.h"
#include <stdlib.h>

FILE * file = NULL;
String * data;
size_t temp_expr_counter = 0;
size_t if_counter = 0;
size_t for_counter = 0;
size_t string_counter = 0;

#define LAST_TEMP_EXPR (temp_expr_counter - 1)

void generate_temp_variable_prefix() {
    writef(file, "%_{i} =l ", temp_expr_counter++);
}

void generate_number_variable(struct Slice * number) {
    char * number_str = slice_to_string(number);
    generate_temp_variable_prefix();
    writef(file, "copy {s}\n", number_str);
    free(number_str);
}

void generate_variable(struct AST * ast) {
    struct a_variable variable = ast->value.variable;

    char * variable_name_str = slice_to_string(&variable.name);
    generate_temp_variable_prefix();
    writef(file, "copy %{s}\n", variable_name_str);
    free(variable_name_str);
}

void generate_string_data(struct AST * ast) {
    struct a_string string = ast->value.string;
    
    string_append(data, format("data $str_{i} = {c}b \"{s}\", b 0}\n", string_counter, '{', slice_to_string(&string.value)));
}

void generate_call(struct AST * ast) {
    struct a_op op = ast->value.op;

    ASSERT1(op.left != NULL);
    ASSERT1(op.right != NULL);
    struct a_expr args_ast = op.right->value.expr;
    struct List arg_variables = init_list(sizeof(long));
    struct AST * node = NULL;

    for (size_t i = 0; i < args_ast.children.size; ++i) {
        node = list_at(&args_ast.children, i);
        if (node->type == AST_STRING) {
            string_counter += 1;
            generate_string_data(node);
            list_push(&arg_variables, (void *) -(string_counter));
        } else {
            generate_expr_node(node);
            list_push(&arg_variables, (void *) LAST_TEMP_EXPR);
        }
    }

    ASSERT1(op.left->type == AST_VARIABLE);
    generate_temp_variable_prefix();
    
    char * name_str = slice_to_string(&op.left->value.variable.name);
    writef(file, "call ${s}(", name_str);
    free(name_str);

    for (size_t i = 0; i < arg_variables.size; ++i) {
        if (i != 0) {
            writef(file, ", ");
        }
        long number = (long) list_at(&arg_variables, i);
        if (number < 0) {
            writef(file, "l $str_{i}", -number);
        } else {
            writef(file, "l %_{i}", number);
        }
    }
    writef(file, ")\n");
}

void generate_increment(struct AST * ast, char is_pre) {
    ASSERT(ast->type == AST_VARIABLE, "Decrement can only operate on a variable");
    struct a_variable variable = ast->value.variable;

    char * var_name_str = slice_to_string(&variable.name);

    generate_temp_variable_prefix();
    writef(file, "add %{s}, 1\n", var_name_str);
    if (is_pre) {
        writef(file, "%{s} =l copy %_{i}\n", var_name_str, LAST_TEMP_EXPR);
    } else {
        generate_temp_variable_prefix();
        writef(file, "copy %{s}\n", var_name_str);
        writef(file, "%{s} =l copy %_{i}\n", var_name_str, LAST_TEMP_EXPR - 1);
    }
    
    free(var_name_str);
}

void generate_decrement(struct AST * ast, char is_pre) {
    ASSERT(ast->type == AST_VARIABLE, "Decrement can only operate on a variable");
    struct a_variable variable = ast->value.variable;

    char * var_name_str = slice_to_string(&variable.name);

    generate_temp_variable_prefix();
    writef(file, "sub %{s}, 1\n", var_name_str);
    if (is_pre) {
        writef(file, "%{s} =l copy %_{i}\n", var_name_str, LAST_TEMP_EXPR);
    } else {
        generate_temp_variable_prefix();
        writef(file, "copy %{s}\n", var_name_str);
        writef(file, "%{s} =l copy %_{i}\n", var_name_str, LAST_TEMP_EXPR - 1);
    }

    free(var_name_str);
}

void generate_assignment(struct AST * ast, size_t expr_number) {
    ASSERT1(ast->type == AST_VARIABLE);
    struct a_variable variable = ast->value.variable;

    char * var_name_str = slice_to_string(&variable.name);

    writef(file, "%{s} =l copy %_{i}\n", var_name_str, expr_number);
    
    free(var_name_str);
}

void generate_binary_operator(const char * operator, size_t lhs, size_t rhs) {
    generate_temp_variable_prefix();
    writef(file, "{s} %_{i}, %_{i}\n", operator, lhs, rhs);
}

void generate_op(struct AST * ast) {
    struct a_op op = ast->value.op;

    switch (op.op.key) {
        case CALL:
            return generate_call(ast);
        case PARENTHESES:
            return generate_expression(op.right);
        case INCREMENT:
            if (op.op.mode == UNARY_PRE) {
                return generate_increment(op.right, 1);
            } else if (op.op.mode == UNARY_POST) {
                return generate_increment(op.right, 0);
            }
            ASSERT1(0);
        case DECREMENT:
            if (op.op.mode == UNARY_PRE) {
                return generate_decrement(op.right, 1);
            } else if (op.op.mode == UNARY_POST) {
                return generate_decrement(op.right, 0);
            }
            ASSERT1(0);
    }

    size_t left_expr = -1;
    size_t right_expr = -1;

    if (op.left != NULL) {
        generate_expr_node(op.left);
        left_expr = LAST_TEMP_EXPR;
    }

    if (op.right != NULL) {
        generate_expr_node(op.right);
        right_expr = LAST_TEMP_EXPR;
    }

    switch (op.op.key) {
        case ASSIGNMENT:
            generate_assignment(op.left, right_expr); break;
        case ADDITION:
            generate_binary_operator("add", left_expr, right_expr); break;
        case SUBTRACTION:
            generate_binary_operator("sub", left_expr, right_expr); break;
        case MULTIPLICATION:
            generate_binary_operator("mul", left_expr, right_expr); break;
        case DIVISION:
            generate_binary_operator("div", left_expr, right_expr); break;
        case REMAINDER:
            generate_binary_operator("rem", left_expr, right_expr); break;
        case EQUAL:
            generate_binary_operator("ceql", left_expr, right_expr); break;
        case NOT_EQUAL:
            generate_binary_operator("cnel", left_expr, right_expr); break;
        case LOGICAL_OR:
            generate_binary_operator("or", left_expr, right_expr); break;
        case LOGICAL_AND:
            generate_binary_operator("and", left_expr, right_expr); break;
        case LESS_THAN:
            generate_binary_operator("csltl", left_expr, right_expr); break;
        case LESS_EQUAL_TO:
            generate_binary_operator("cslel", left_expr, right_expr); break;
        case GREATER_THAN:
            generate_binary_operator("csgtl", left_expr, right_expr); break;
        case GREATER_EQUAL_TO:
            generate_binary_operator("csgel", left_expr, right_expr); break;
        default:
             ASSERT(0, format("Unimplemented operator generation: '{s}'", ast_to_string(ast)));
    }

}

void generate_return(struct AST * ast) {
    struct a_return _return = ast->value._return;

    generate_expression(_return.expr);
    writef(file, "ret %_{i}\n", LAST_TEMP_EXPR);
}

void generate_for(struct AST * ast) {
    struct a_for _for = ast->value._for;

    size_t for_number = for_counter++;
    
    writef(file, "@for_{i}_start\n", for_number);
    generate_expression(_for.condition);
    writef(file, "jnz %_{i}, @for_{i}_body, @for_{i}_end\n"
                 "@for_{i}_body\n", LAST_TEMP_EXPR, for_number, for_number, for_number);
    generate_scope(_for.body);
    writef(file, "@for_{i}_continue\n", for_number);
    if (_for._continue != NULL) {
        generate_expression(_for._continue);
    }
    writef(file, "jmp @for_{i}_start\n", for_number);
    writef(file, "@for_{i}_end\n", for_number);
    
}

void generate_if(struct AST * ast) {
    struct a_if _if = ast->value._if;
    struct AST * node = NULL;

    size_t if_number = if_counter++;

    for (size_t i = 0; i < _if.bodies.size; ++i) {
        if (i < _if.conditions.size) {
            generate_expression(list_at(&_if.conditions, i));
            writef(file, "jnz %_{i}, @if_{i}_{i}_start, @if_{i}_{i}_next\n", LAST_TEMP_EXPR, if_number, i, if_number, i);
        }
        writef(file, "@if_{i}_{i}_start\n", if_number, i);

        node = list_at(&_if.bodies, i);
        generate_scope(node);

        if (!node->value.scope.returns) {
            writef(file, "jmp @if_{i}_end\n", if_number);
        }
        writef(file, "@if_{i}_{i}_next\n", if_number, i);
    }
    writef(file, "@if_{i}_end\n", if_number);
}

void generate_expr_node(struct AST * ast) {
    ASSERT1(ast != NULL);

    switch (ast->type) {
         case AST_NUMBER:
             generate_number_variable(&ast->value.number.value); break;
        case AST_OP:
             generate_op(ast); break;
        case AST_VARIABLE:
             generate_variable(ast); break;
        case AST_EXPR:
             generate_expression(ast); break;
        default:
             ASSERT(0, format("Unhandled AST type: '{s}'", AST_type_to_string(ast->type)));
    }
}

void generate_expression(struct AST * ast) {
    struct a_expr expr = ast->value.expr;

    for (size_t i = 0; i < expr.children.size; ++i) {
        generate_expr_node(list_at(&expr.children, i));
    }
}

void generate_declaration(struct AST * ast) {
    struct a_declaration declaration = ast->value.declaration;

    size_t prev_value = temp_expr_counter;
    generate_expression(declaration.expression);
    
    ASSERT(prev_value != temp_expr_counter, "Unexpanded expression or something?");

    writef(file, "%{s} =l copy %_{i}\n",
            slice_to_string(&declaration.variable->value.variable.name),
            temp_expr_counter - 1);
}

void generate_scope(struct AST * ast) {
    struct a_scope scope = ast->value.scope;
    struct AST * node;

    for (size_t i = 0; i < scope.nodes.size; ++i) {
        node = list_at(&scope.nodes, i);
        switch (node->type) {
            case AST_DECLARATION:
                generate_declaration(node); break;
            case AST_EXPR:
                generate_expression(node); break;
            case AST_IF:
                generate_if(node); break;
            case AST_FOR:
                generate_for(node); break;
            case AST_RETURN:
                generate_return(node); break;
            default:
                ASSERT(0, format("Unimplemented scope node case '{s}'", AST_type_to_string(ast->type)));
        }
    }
}

void generate_function(struct AST * ast) {
    ASSERT1(ast->type == AST_FUNCTION);
    struct a_function function = ast->value.function;
    struct AST * node;
    
    writef(file, "export function w ${s}(", slice_to_string(&function.name));

    size_t i = 0;
    while (i < function.arguments.size) {
        node = list_at(&function.arguments, i);
        writef(file, "l %{s}", slice_to_string(&node->value.variable.name));

        if (++i == function.arguments.size) {
            break;
        }
        writef(file, ", ");
    }

    writef(file, ") {c}\n@start\n", '{');

    generate_scope(function.body);

    writef(file, "}\n");

}

void generate_qed(struct AST * ast) {
    ASSERT1(ast->type == AST_ROOT);

    data = init_string("");
    file = open_file("out.qbe", "wb");
    struct a_root root = ast->value.root;
    
    for (size_t i = 0; i < root.nodes.size; ++i) {
        generate_function(list_at(&root.nodes, i));
    }

    writef(file, "\n# Data:\n{s}", data->_ptr);
    fclose(file);
}
