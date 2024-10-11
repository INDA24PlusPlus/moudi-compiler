#include "parser/lexer.h"
#include "common/io.h"
#include "common/logger.h"
#include "parser/token.h"
#include "utils/slice.h"
#include "parser/operators.h"
#include "fmt.h"

#include <ctype.h>

struct Lexer init_lexer(char * filepath) {
    size_t length = 0;
    char * src = read_file(filepath, &length);

    struct Lexer lexer = {
        .src = src,
        .size = length,
        .line = 1,
        .pos = 1,
        .c = src[0],
        .token = init_token(),
    };
    
    lexer_next_token(&lexer);

    return lexer;
}

void lexer_dump(struct Lexer * lexer) {
    println("Last token -> {s}", token_to_string(&lexer->token));
}

void lexer_advance(struct Lexer * lexer) {
    if (!lexer->c || lexer->size <= lexer->index) {
        println("[Lexer]: End of file found while lexing: ");
        lexer_dump(lexer);
        exit(1);
    }

    lexer->pos += 1;
    lexer->index += 1;
    if ((lexer->c = lexer->src[lexer->index]) == '\n') {
        lexer->line += 1;
        lexer->pos = 0;
    }
}

void lexer_update(struct Lexer * lexer, size_t forward) {
    while (forward--) {
        lexer_advance(lexer);
    }
}

void lexer_advance_as(struct Lexer * lexer, enum token_t type) {
    set_token(&lexer->token, init_slice(lexer->src + lexer->index, 1), type, lexer->line, lexer->pos);
    lexer_advance(lexer);
}

void lexer_skip_whitespace(struct Lexer * lexer) {
    while (1) {
        switch (lexer->c) {
            case ' ':
            case '\t': 
                break;
            case '\r': 
                lexer->pos = 0; break;
            default: return;
        }
        lexer_advance(lexer);
    }
}

char lexer_peek(struct Lexer * lexer, size_t offset) {
    return lexer->src[lexer->index + offset];
}

char is_characther_operator(char c) {
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '|':
        case '=':
        case '<':
        case '>':
        case '(':
        case ')':
            return 1;
        default:
            return 0;
    }
}

void lexer_skip_comment(struct Lexer * lexer) {
    while (lexer_advance(lexer), lexer->c != '\n');
}

void lexer_parse_operator(struct Lexer * lexer) { 
    struct Operator op;
    char * start_of_token = lexer->token.value.start;
    // arr is an buf keeping track of the indeces of all possible operators matching the string so far and that are longer than current matching
    size_t arr_index = sizeof(op_conversion) / sizeof(op_conversion[0]), arr_size;
    int arr[sizeof(op_conversion) / sizeof(op_conversion[0])] = {0};
    size_t length = 0, length_found = 0, start_line = lexer->line, start_pos = lexer->pos;
    char c = lexer->src[lexer->index - 1];

    while (is_characther_operator(c)) {
        arr_size = arr_index; // number of possible results
        arr_index = 0;

        for (int i = 0; i < arr_size; ++i) {
            int op_index = (arr[i] != 0) ? arr[i] : i;
            op = op_conversion[op_index];
            if (c == op.str[length]) {
                // if operator has more characthers
                if (op.str[length + 1]) {
                    arr[arr_index++] = op_index;
                } else {
                    length_found = length + 1;
                }
            } else if (op.enclosed && c == op.str[op.enclosed_offset + length]) {
                if (op.str[op.enclosed_offset + length + 1]) {
                    arr[arr_index++] = op_index;
                } else {
                    length_found = length + 1;
                }
            }
        }

        c = lexer->src[lexer->index + length];
        length += 1;
    }

    struct Slice slice = init_slice(start_of_token, length_found);

    lexer_update(lexer, length_found - 1);

    // if length is 0 then there was no match as length is the length of an existing operator
    if (length_found == 0) {
        struct Slice slice = init_slice(start_of_token, lexer->index - (start_of_token - lexer->src));
        logger_log(format("{2i::} Invalid operator '{s}'", lexer->line, lexer->pos, slice_to_string(&slice)), LEXER, ERROR);
        ASSERT1(0);
    }
    
    set_token(&lexer->token, init_slice(start_of_token, length_found), TOKEN_OP, start_line, start_pos);
}

void lexer_parse_id(struct Lexer * lexer) {
    size_t line = lexer->line, pos = lexer->pos;
    char * start_of_token = lexer->src + lexer->index;

    // skip first since that must already be valid since this function has been called
    lexer_advance(lexer);
    while (isalpha(lexer->c) || isdigit(lexer->c) || lexer->c == '_') {
        lexer_advance(lexer);
    }

    // not +1 since lexer_advance will always go one more than the id length
    struct Slice token_slice = init_slice(start_of_token, lexer->index - (start_of_token - lexer->src));
    set_token(&lexer->token, token_slice, TOKEN_ID, line, pos);
}

void lexer_parse_int(struct Lexer * lexer) {
    size_t line = lexer->line, pos = lexer->pos;
    char * start_of_token = lexer->src + lexer->index;
    while (isdigit(lexer->c) || lexer->c == '_') {
        lexer_advance(lexer);
    }

    // not +1 since lexer_advance will always go one more than the id length
    struct Slice token_slice = init_slice(start_of_token, lexer->index - (start_of_token - lexer->src));
    set_token(&lexer->token, token_slice, TOKEN_NUMBER, line, pos);
}

void lexer_next_token(struct Lexer * lexer) {
    lexer_skip_whitespace(lexer);

    switch (lexer->c) {
        case '\0':
            set_token(&lexer->token, init_slice(lexer->src, 0), TOKEN_EOF, lexer->line, lexer->pos);
            break;
        case '\n':
            lexer_advance_as(lexer, TOKEN_LINEBREAK);
            break;
        case '{':
            lexer_advance_as(lexer, TOKEN_LBRACE); break;
        case '}':
            lexer_advance_as(lexer, TOKEN_RBRACE); break;
        case '/':
            if (lexer_peek(lexer, 1) == '/') {
                lexer_skip_comment(lexer);
            } else {
                lexer_advance_as(lexer, TOKEN_DIVIDE);
            }
            break;
        case '(':
            lexer_advance_as(lexer, TOKEN_LPAREN); break;
        case ')':
            lexer_advance_as(lexer, TOKEN_RPAREN); break;
        case '-':
            lexer_advance_as(lexer, TOKEN_MINUS); break;
        case '+':
            lexer_advance_as(lexer, TOKEN_PLUS); break;
        case '*':
            lexer_advance_as(lexer, TOKEN_MULTIPLY); break;
        case '<':
            lexer_advance_as(lexer, TOKEN_LT); break;
        case '>':
            lexer_advance_as(lexer, TOKEN_GT); break;
        case '=':
            lexer_advance_as(lexer, TOKEN_EQUAL); break;
        case '|':
            lexer_advance_as(lexer, TOKEN_VERTICALLINE); break;
        case ',':
            lexer_advance_as(lexer, TOKEN_COMMA); break;
        case ':':
            lexer_advance_as(lexer, TOKEN_COLON); break;
        case '#':
            lexer_parse_id(lexer);
            lexer->token.type = TOKEN_INTRINSIC;
            break;
        default:
            if (isalpha(lexer->c)) {
                lexer_parse_id(lexer);
            } else if (isdigit(lexer->c)) {
                lexer_parse_int(lexer);
            } else {
                println("[Lexer]: Unexpected characther: '{c}'(ASCII {u})", lexer->c, lexer->c);
                exit(1);
            }
    }
}
