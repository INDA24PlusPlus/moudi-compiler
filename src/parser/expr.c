#include "fmt.h"
#include "parser/parser.h"
#include "common/logger.h"
#include "parser/token.h"
#include "common/deque.h"
#include "parser/operators.h"
#include "utils/slice.h"

struct Operator * str_to_operator(struct Slice str, enum OP_mode mode, char * enclosed_flag) {
    const char * op_str;
    for (int i = 0; i < sizeof(op_conversion) / sizeof(op_conversion[0]); ++i) {
        op_str = op_conversion[i].str;

        if (op_conversion[i].enclosed == ENCLOSED && op_conversion[i].mode == mode) {
            if (!strncmp(str.start, op_str, str.length)) {
                *enclosed_flag = 0;
                return &op_conversion[i];
            } else if (!strncmp(str.start, op_str + strlen(op_str) + 1, str.length)) {
                *enclosed_flag = 1;
                return &op_conversion[i];
            }
        }

        if ((mode == OP_TYPE_ANY || op_conversion[i].mode == mode) && !strncmp(str.start, op_str, str.length)) {
            return &op_conversion[i];
        }
    }
    return &op_conversion[0];
}

struct Operator * get_operator(struct Slice str, struct Token * token, enum OP_mode mode, char * enclosed_flag) {
    struct Operator * op = str_to_operator(str, mode, enclosed_flag);

    if (op->key == OP_NOT_FOUND && mode == BINARY) {
        op = str_to_operator(str, UNARY_POST, enclosed_flag);
    }

    if (op->key == OP_NOT_FOUND) {
        logger_log(format("{s} operator '{s}' not found: ", mode == BINARY ? "Binary" : "Unary", slice_to_string(&str)), PARSER, ERROR);
        print("{s}\n", token_to_string(token));
        exit(1);
    }

    return op;
}

void consume_add_operator(struct Operator * op, struct List * list, struct Parser * parser) {
    struct AST * ast = init_ast(AST_OP, parser->current_scope);
    struct a_op * operator = &ast->value.op;

    if (list->size == 0) {
        logger_log("Invalid expression: Operator without valid operands", PARSER, ERROR);
        exit(1);
    }

    operator->op = *op;
    operator->right = list_at(list, -1);
    list_pop(list);

    if (op->mode == BINARY) {
        if (list->size == 0) {
            logger_log("Invalid expression: Binary operator without two valid operands", PARSER, ERROR);
            exit(1);
        }
        operator->left = list_at(list, -1);
        list_pop(list);
    }

    list_push(list, ast);
}

struct List _parser_parse_expr(struct Parser * parser, struct List * output, struct Deque * operators, enum Operators EXIT_ON_KEY) {
    struct AST * node, * temp;
    struct List expressions = init_list(sizeof(struct AST *));
    
    enum OP_mode mode = UNARY_PRE;
    struct Operator * op1, * op2;

    char flag;

    while (1) {
        switch (parser->lexer.token.type) {
            case TOKEN_ID:
            {
                if (mode == BINARY) {
                    println("[Parser] Invalid expression token: {s}\n", token_to_string(&parser->lexer.token));
                    ASSERT1(0);
                }

                list_push(output, parser_parse_variable(parser));
                mode = BINARY;
            } break;
            case TOKEN_NUMBER:
            {
                list_push(output, parser_parse_number(parser));

                mode = BINARY;
            } break;
            case TOKEN_OP:
            {
                // enclosed flag is true if enclosed operator is the closing enclosing operator
                op1 = get_operator(parser->lexer.token.value, &parser->lexer.token, mode, &flag);

                if (op1->enclosed == ENCLOSED) {
                    if (!flag) { // open enclosed operator
                        parser_eat(parser, parser->lexer.token.type);
                        
                        struct Deque temp_d = init_deque(sizeof(struct Operator *));
                        struct List temp_l = init_list(sizeof(struct AST *));
                        push_back(&temp_d, &op1);

                        node = init_ast(AST_OP, parser->current_scope);
                        temp = init_ast(AST_EXPR, parser->current_scope);
                        node->value.op.op = *op1;
 
                        temp->value.expr.children = _parser_parse_expr(parser, &temp_l, &temp_d, -1);
                        node->value.op.right = temp; 
                    } else { // closing enclosed operator
                        while (strcmp(op1->str, (op2 = deque_back(operators))->str)) { // while not start version of this enclosed operator
                            if (op2->key == EXIT_ON_KEY) {
                                ASSERT(sizeof(op1->key) != (sizeof(op_conversion) / sizeof(op_conversion[0])), "Possibly invalid EXIT_ON_KEY:");
                                goto exit;
                            }
                            if (operators->size == 1) {
                                println("[Parser] Unmatched enclosed operator: {s}\n", token_to_string(&parser->lexer.token));
                                exit(1);
                            }
                            consume_add_operator(op2, output, parser);
                            pop_back(operators);
                        }
                       
                        pop_back(operators);
                        parser_eat(parser, parser->lexer.token.type);
                        goto exit;
                    }
                }


                while (operators->size) {
                    op2 = deque_back(operators);
                    if (op2->enclosed == ENCLOSED || (op1->precedence <= op2->precedence && (op1->precedence != op2->precedence || op2->associativity != LEFT))) {
                        break;
                    }
                    
                    consume_add_operator(op2, output, parser);
                    pop_back(operators);
                }
                
                if (op1->enclosed == ENCLOSED) {
                    if (op1->mode == BINARY) {
                        node->value.op.left = list_at(output, -1);
                        list_pop(output);
                    }

                    list_push(output, node);
                    mode = BINARY;
                    break;
                }

                push_back(operators, op1);

                mode = op1->mode == UNARY_POST ? BINARY : UNARY_PRE;
                parser_eat(parser, TOKEN_OP);
            } break;
            /* case TOKEN_COLON: */
            case TOKEN_COMMA:
            {
                while (operators->size && (op2 = deque_back(operators))->enclosed != ENCLOSED) {
                    consume_add_operator(op2, output, parser);
                    pop_back(operators);
                }
                
                if (output->size == 1) {
                    list_push(&expressions, list_at(output, -1));
                    *output = init_list(sizeof(struct AST *));
                }else if (output->size != 1) {
                    logger_log("Unprecedentent usage of expression separator", PARSER, FATAL);
                    println("{s}", token_to_string(&parser->lexer.token));
                    exit(1);
                }

                mode = UNARY_PRE;
                parser_eat(parser, parser->lexer.token.type);
            } break;
            case TOKEN_PLUS:
            case TOKEN_MINUS:
            case TOKEN_MULTIPLY:
            case TOKEN_DIVIDE:
            case TOKEN_EQUAL:
            case TOKEN_VERTICALLINE:
            case TOKEN_LT:
            case TOKEN_GT:
            case TOKEN_LPAREN:
            case TOKEN_RPAREN:
                lexer_parse_operator(&parser->lexer);
                break;
            case TOKEN_EOF:
            case TOKEN_LINEBREAK:
            case TOKEN_LBRACE:
            case TOKEN_RBRACE:
                goto exit;
            default:
                println("[Parser]: Unrecognized token in expression\n{s}\n", token_to_string(&parser->lexer.token));
                exit(1);
        }
    }

exit: 

    // re-use flag to check if there has been an incorrectly placed operator
    flag = operators->size != 0;

    while (operators->size) {
        op1 = deque_back(operators);
        if (op1->enclosed == ENCLOSED) {
            println("[Parser] Unmatched enclosed operator near: {s}", token_to_string(&parser->lexer.token));
            exit(1);
        }
        consume_add_operator(op1, output, parser);
        pop_back(operators);
    }

    if (output->size == 1) {
        list_push(&expressions, list_at(output, 0));
    } else if (flag) {
        logger_log(format("Invalid expression; too many discarded expressions({i})", output->size), PARSER, FATAL);
        println("\tLast token: {s}", token_to_string(&parser->lexer.token));
    }

    return expressions;

}

struct AST * parser_parse_expr_exit_on(struct Parser * parser, enum Operators op) {
    struct AST * ast = init_ast(AST_EXPR, parser->current_scope);

    struct List output = init_list(sizeof(struct AST *));
    struct Deque operators = init_deque(sizeof(struct Operator *));

    if (op != -1) {
        struct Operator * temp_operator = malloc(sizeof(struct Operator));
        for (int i = 0; i < (sizeof(op_conversion) / sizeof(struct Operator)); ++i) {
            if (op == op_conversion[i].key) {
                *temp_operator = op_conversion[i];
                break;
            }
        }
        push_front(&operators, temp_operator);
    }

    ast->value.expr.children = _parser_parse_expr(parser, &output, &operators, op);

    return ast;
}

struct AST * parser_parse_expr(struct Parser * parser) {
    return parser_parse_expr_exit_on(parser, -1);
}
