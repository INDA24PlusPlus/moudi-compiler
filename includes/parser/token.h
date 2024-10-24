#pragma once
#include "utils/slice.h"
#include "utils/macro.h"

#ifndef TOKEN_FOREACH
#define TOKEN_FOREACH(f) \
    f(TOKEN_EOF) \
    f(TOKEN_ID) \
    f(TOKEN_OP) \
    f(TOKEN_PLUS) \
    f(TOKEN_MINUS) \
    f(TOKEN_MULTIPLY) \
    f(TOKEN_DIVIDE) \
    f(TOKEN_EQUAL) \
    f(TOKEN_LT) \
    f(TOKEN_GT) \
    f(TOKEN_LPAREN) \
    f(TOKEN_RPAREN) \
    f(TOKEN_LBRACE) \
    f(TOKEN_RBRACE) \
    f(TOKEN_COMMA) \
    f(TOKEN_COLON) \
    f(TOKEN_VERTICALLINE) \
    f(TOKEN_INTRINSIC) \
    f(TOKEN_NUMBER) \
    f(TOKEN_STRING) \
    f(TOKEN_COMMENT) \
    f(TOKEN_LINEBREAK) 

GENERATE_ENUM(token_t, TOKEN_FOREACH);
GENERATE_ENUM_TO_STR_FUNC(token_t);
#endif

struct Token {
    struct Slice value;
    size_t line, pos;
    enum token_t type;
};

struct Token init_token();
char * token_to_string(struct Token * token);

void set_token(struct Token * dest, struct Slice slice, enum token_t type, size_t line, size_t pos);
void copy_token(struct Token * dest, struct Token * src);
