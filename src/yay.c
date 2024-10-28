#include "yay.h"
#include "fmt.h"
#include "parser/AST.h"
#include "parser/parser.h"
#include "codegen/checker.h"
#include "codegen/generator.h"
#include <stdlib.h>
#include <sys/time.h>

struct timeval t_start, t_stop;

void start_timer() {
    gettimeofday(&t_start, NULL);
}

unsigned long stop_timer() {
    gettimeofday(&t_stop, NULL);
    return 1000000 * (t_stop.tv_sec - t_start.tv_sec) + (t_stop.tv_usec - t_start.tv_usec);
}

void yay_compile(char * filepath) {
    long time = 0, total = 0;
    char * parser_time,
         * checker_time,
         * gen_time;

    start_timer();

    struct AST * ast = parser_parse(filepath);

    time = stop_timer();
    total += time;
    asprintf(&parser_time, "Time for parser:\t%.3fms", (double)time / 1000);

    start_timer();

    checker_check(ast);
    
    time = stop_timer();
    total += time;
    asprintf(&checker_time, "Time for checker:\t%.3fms", (double)time / 1000);

    print_ast_tree(ast);

    start_timer();

    generate_qed(ast);
    
    time = stop_timer();
    total += time;
    asprintf(&gen_time, "Time for generator:\t%.3fms", (double)time / 1000);

    int res = system("qbe out.qbe > out.s");
    if (res != 256) {
        system("cc out.s");
    }

    puts("-------------------------------------");
    puts(parser_time);
    puts(checker_time);
    puts(gen_time);
    puts("-------------------------------------");
    printf("Total: %.3fms\n", (double)total / 1000);

}
