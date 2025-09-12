#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"

int main(int argc, char *argv[]) {
    FILE *input = NULL;

    NIL = malloc(sizeof(sExp));
    NIL->type = TYPE_NIL;

    TRUE = malloc(sizeof(sExp));
    TRUE->type = TYPE_SYMBOL;
    TRUE->strVal = strdup("t");

    if (argc == 1) {
        printf("Reading from stdin. Enter a S-Expression:\n");
        printf("> ");
        input = stdin;
    } 
    else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            printf("Bad File: %s\n", argv[1]);
            return 1;
        }
    }

    sExp *exp;
    while ((exp = read_sexp(input)) != NULL) {
        // printf(": ");
        print_sexp(exp);
        printf("\n> ");
    }

    if (input != stdin) fclose(input);
    return 0;
}