#include "fmt.h"
#include "common/common.h"
#include "parser/token.h"

GENERATE_ENUM_TO_STR_FUNC_BODY(token_t, TOKEN_FOREACH)

struct Token init_token() {
    return (struct Token) {};
}

char * token_to_string(struct Token * token) {
    return format("Token({s}): \"{s}\" at {2i::}", token_t_to_string(token->type), slice_to_string(&token->value), token->line, token->pos);
}

void set_token(struct Token * dest, struct Slice slice, enum token_t type, size_t line, size_t pos) {
    dest->value = slice;
    dest->type = type;
    dest->line = line;
    dest->pos = pos;
}

void copy_token(struct Token * dest, struct Token * src) {
    memcpy(dest, src, sizeof(struct Token));
}
