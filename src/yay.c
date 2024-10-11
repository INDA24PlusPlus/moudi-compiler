#include "yay.h"
#include "parser/parser.h"

void yay_compile(char * filepath) {
    struct AST ast = parser_parse(filepath);

}
