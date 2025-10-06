#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"

static void repl(FILE *in, int show_prompt) {
    if (show_prompt) { printf("> "); fflush(stdout); }
    for (sExp *expr = read_sexp(in); expr; expr = read_sexp(in)) {
        sExp *val = eval(expr);
        print_sexp(val);
        printf("\n");
        if (should_quit) break;
        if (show_prompt) { printf("> "); fflush(stdout); }
    }
}

static void load_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Bad File: %s\n", path); exit(1); }
    for (sExp *expr = read_sexp(f); expr; expr = read_sexp(f)) (void)eval(expr);
    fclose(f);
}

int main(int argc, char *argv[]) {
    init_runtime();
    if (argc == 2) {
        load_file(argv[1]);   
        return 0;             
    }
    printf("Reading from stdin. Enter S-expressions:\n");
    repl(stdin, 1);
    return 0;
}

