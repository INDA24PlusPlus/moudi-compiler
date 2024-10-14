#include "parser/parser.h"
#include "common/list.h"
#include "common/logger.h"
#include "fmt.h"
#include "parser/AST.h"
#include "parser/lexer.h"
#include "parser/operators.h"
#include "parser/token.h"
#include "utils/slice.h"
#include <string.h>

struct Parser init_parser(char * filepath) {
    return (struct Parser) {
        .root = init_ast(AST_ROOT, NULL),
        .lexer = init_lexer(filepath)
    };
}

static inline char parser_token_is_type(struct Parser * parser, enum token_t expected_type) {
    return parser->lexer.token.type == expected_type;
}

static inline char parser_token_compare(struct Parser * parser, const char * cmp) {
    return !strncmp(parser->lexer.token.value.start, cmp, strlen(cmp) - 1);
}

void parser_eat(struct Parser * parser, enum token_t expected_type) {
    if (!parser_token_is_type(parser, expected_type)) {
        println("[Parser] {2i::} Error: Expected type '{s}' found '{s}'", 
                parser->lexer.line, parser->lexer.pos,
                token_t_to_string(expected_type),
                token_t_to_string(parser->lexer.token.type));
    }

    lexer_next_token(&parser->lexer);
}

struct AST * parser_parse_variable(struct Parser * parser) {
    struct AST * ast = init_ast(AST_VARIABLE, parser->current_scope);

    struct a_variable * variable = &ast->value.variable;
    variable->name = parser->lexer.token.value;
    if (parser_token_is_type(parser, TOKEN_INTRINSIC)) {
        parser_eat(parser, TOKEN_INTRINSIC);
    } else {
        parser_eat(parser, TOKEN_ID);
    }

    return ast;
}

struct AST * parser_parse_number(struct Parser * parser) {
    struct AST * ast = init_ast(AST_NUMBER, parser->current_scope);
    
    struct a_number * number = &ast->value.number;
    number->value = parser->lexer.token.value;
    parser_eat(parser, TOKEN_NUMBER);

    return ast;
}

struct AST * parser_parse_declaration(struct Parser * parser) {
    struct AST * ast = init_ast(AST_DECLARATION, parser->current_scope), * node;
    struct a_declaration * decl = &ast->value.declaration;
    size_t line = parser->lexer.line, pos = parser->lexer.pos;
    parser_eat(parser, TOKEN_ID);

    decl->variable = parser_parse_variable(parser);
    parser_eat(parser, TOKEN_EQUAL);
    decl->expression = parser_parse_expr(parser);

    return ast;
}

struct AST * parser_parse_if(struct Parser * parser) {
    struct AST * ast = init_ast(AST_IF, parser->current_scope);
    struct a_if * _if = &ast->value._if;
    char prev_had_if = 1;

    parser_eat(parser, TOKEN_ID);
    list_push(&_if->conditions, parser_parse_expr(parser));
    list_push(&_if->bodies, parser_parse_scope(parser));

    while (parser_token_is_type(parser, TOKEN_ID) && parser_token_compare(parser, "else")) {
        if (!prev_had_if) {
            logger_log(format("{2i::} else block can not be followed by more else", parser->lexer.line, parser->lexer.pos), PARSER, ERROR);
            ASSERT1(0);
        }
        parser_eat(parser, TOKEN_ID);
        if (parser_token_is_type(parser, TOKEN_ID) && parser_token_compare(parser, "if")) {
            parser_eat(parser, TOKEN_ID);
            list_push(&_if->conditions, parser_parse_expr(parser));
        } else {
            prev_had_if = 0;
        }
        list_push(&_if->bodies, parser_parse_scope(parser));
    }

    return ast;
}

struct AST * parser_parse_for(struct Parser * parser) {
    struct AST * ast = init_ast(AST_FOR, parser->current_scope);
    struct a_for * _for = &ast->value._for;

    parser_eat(parser, TOKEN_ID);
    _for->condition = parser_parse_expr(parser);

    if (parser_token_is_type(parser, TOKEN_COLON)) {
        parser_eat(parser, TOKEN_COLON);
        _for->_continue = parser_parse_expr(parser);
    }

    _for->body = parser_parse_scope(parser);

    return ast;
}


struct AST * parser_parse_return(struct Parser * parser) {
    struct AST * ast = init_ast(AST_RETURN, parser->current_scope);
    struct a_return * _return = &ast->value._return;
    parser_eat(parser, TOKEN_ID);

    _return->expr = parser_parse_expr(parser);

    return ast;
}

struct AST * parser_parse_statement(struct Parser * parser) {
    char * src = parser->lexer.token.value.start;
    size_t length = parser->lexer.token.value.length;

    if (!strncmp(src, "let", length)) {
        return parser_parse_declaration(parser);
    } else if (!strncmp(src, "for", length)) {
        return parser_parse_for(parser);
    } else if (!strncmp(src, "if", length)) {
        return parser_parse_if(parser);
    } else if (!strncmp(src, "return", length)) {
        return parser_parse_return(parser);
    } else {
        return parser_parse_expr(parser);
    }
}

struct AST * parser_parse_scope(struct Parser * parser) {
    struct AST * ast = init_ast(AST_SCOPE, parser->current_scope);
    struct a_scope * scope = &ast->value.scope;
    parser->current_scope = ast;
    
    size_t line = parser->lexer.line, pos = parser->lexer.pos;

    parser_eat(parser, TOKEN_LBRACE);

    while (1) {
        if (parser_token_is_type(parser, TOKEN_LINEBREAK)) {
            parser_eat(parser, TOKEN_LINEBREAK);
            continue;
        } else if (parser_token_is_type(parser, TOKEN_RBRACE)) {
            break;
        } else if (parser_token_is_type(parser, TOKEN_EOF)) {
            logger_log(format("Unclosed scope at {2i::}", line, pos), PARSER, ERROR);
            ASSERT1(0);
        }

        list_push(&scope->nodes, parser_parse_statement(parser));
    }
    parser_eat(parser, TOKEN_RBRACE);
    
    parser->current_scope = ast->scope;
    return ast;
}

struct AST * parser_parse_function(struct Parser * parser) {
    struct AST * ast = init_ast(AST_FUNCTION, parser->current_scope),
               * node;

    struct a_function * function = &ast->value.function;

    parser_eat(parser, TOKEN_ID);
    function->name = parser->lexer.token.value;
    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LPAREN);

    while (parser_token_is_type(parser, TOKEN_ID)) {
        list_push(&function->arguments, parser_parse_variable(parser));

        if (parser_token_is_type(parser, TOKEN_COMMA)) {
            parser_eat(parser, TOKEN_COMMA);
        }
    }

    parser_eat(parser, TOKEN_RPAREN);
    
    function->body = parser_parse_scope(parser);

    parser->current_scope = ast->scope;
    return ast;
}

void parser_parse_root(struct Parser * parser) {
    parser->current_scope = parser->root;
    struct a_root * root = &parser->root->value.root;
    root->nodes = init_list(sizeof(struct AST *));

    while (!parser_token_is_type(parser, TOKEN_EOF)) {
        if (parser_token_is_type(parser, TOKEN_LINEBREAK)) {
            parser_eat(parser, TOKEN_LINEBREAK);
            continue;
        } else if (!parser_token_is_type(parser, TOKEN_ID)) {
            logger_log("Error: Top level identifier", LEXER, ERROR);
            exit(1);
        }

        struct AST * node = parser_parse_function(parser);
        ASSERT1(node != NULL);

        list_push(&root->nodes, node);
    }
}

struct AST * parser_parse(char * filepath) {
    struct Parser parser = init_parser(filepath);

    parser_parse_root(&parser);

    return parser.root;
}
