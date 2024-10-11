#pragma once
#include "common/common.h"
#include "parser/token.h"

struct Lexer {
    char * src;
    size_t size;
    size_t index;
    size_t line, pos;
    char c;
    struct Token token;
};

struct Lexer init_lexer(char * src);
void lexer_dump(struct Lexer * lexer);

char lexer_peek(struct Lexer * lexer, size_t offset);
void lexer_advance(struct Lexer * lexer);
void lexer_advance_as(struct Lexer * Lexer, enum token_t type);
void lexer_skip_whitespace(struct Lexer * lexer);

void lexer_parse_operator(struct Lexer * lexer);
void lexer_parse_id(struct Lexer * lexer);
void lexer_parse_int(struct Lexer * lexer);

void lexer_next_token(struct Lexer * lexer);
