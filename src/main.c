#include "fmt.h"
#include "yay.h"
#include <string.h>

#define LINE_BREAKER "-------------------------\n"

int main(int argc, char ** args){
    char ast_flag = 0;
    char time_flag = 0;

    if (argc == 1) {
        println("Please specify a source file");
        exit(1);
    }

    for (size_t i = 2; i < argc; ++i) {
        if (args[i][0] != '-' || args[i][1] != '-') {
            continue;
        }

        if (strcmp("ast", args[i] + 2) == 0) {
            ast_flag = 1;
        } else if (strcmp("time", args[i] + 2) == 0) {
            time_flag = 1;
        }
    }

    yay_compile(args[1], ast_flag, time_flag);
}
