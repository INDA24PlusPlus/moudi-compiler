#include "yay.h"

#define LINE_BREAKER "-------------------------\n"

int main(int argc, char ** args){
    if (argc > 1) {
        yay_compile(args[1]);
    } else {
        println("Please specifify a source file");
    }
}
