#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { TYPE_NIL, TYPE_INT, TYPE_DOUBLE, TYPE_SYMBOL, TYPE_STRING, TYPE_CONS } sType;
typedef struct sExp {
    sType type;
    union 
    {
        long intVal;
        double doubleVal;
        char *strVal;
        struct 
        {
            struct sExp *car;
            struct sExp *cdr;
        } cons;
    };
} sExp;

sExp *NIL;   
sExp *TRUE; 

sExp *read_sexp(FILE *in);
void print_sexp(sExp *exp);

int peek(FILE *input) {
    int character = fgetc(input);
    if (character != EOF) ungetc(character, input);
    return character;
}

void skip_ws(FILE *input) {
    int charcter;
    while ((charcter = fgetc(input)) != EOF) {
        if (!isspace(charcter)) {
            ungetc(charcter, input);
            return;
        }
    }
}

// sExp *make_nil() { 
    // sExp *exp = malloc(sizeof(sExp)); 
    // exp->type = TYPE_NIL; 
    // return exp; 
    // }

// sExp *make_true() { return TRUE; } 

sExp *make_int(long val) {
    sExp *exp = malloc(sizeof(sExp));
    exp->type = TYPE_INT;
    exp->intVal = val;
    return exp;
}

sExp *make_double(double val) {
    sExp *exp = malloc(sizeof(sExp));
    exp->type = TYPE_DOUBLE;
    exp->doubleVal = val;
    return exp;
}

sExp *make_symbol(const char *s) {
    sExp *exp = malloc(sizeof(sExp));
    exp->type = TYPE_SYMBOL;
    exp->strVal = strdup(s);
    return exp;
}

sExp *make_string(const char *s) {
    sExp *exp = malloc(sizeof(sExp));
    exp->type = TYPE_STRING;
    exp->strVal = strdup(s);
    return exp;
}

sExp *cons(sExp *car, sExp *cdr) {
    sExp *exp = malloc(sizeof(sExp));
    exp->type = TYPE_CONS;
    exp->cons.car = car;
    exp->cons.cdr = cdr;
    return exp;
}

sExp *car(sExp *exp) {
    return (exp->type == TYPE_CONS) ? exp->cons.car : NIL;
}

sExp *cdr(sExp *exp) {
    return (exp->type == TYPE_CONS) ? exp->cons.cdr : NIL;
}

// --- Predicates ---
int is_nil(sExp *exp)    { return exp->type == TYPE_NIL; }

int is_symbol(sExp *exp) { return exp->type == TYPE_SYMBOL; }

int is_number(sExp *exp) { return exp->type == TYPE_INT || exp->type == TYPE_DOUBLE; }

int is_string(sExp *exp) { return exp->type == TYPE_STRING; }

int is_list(sExp *exp) {
    if (exp->type == TYPE_NIL) return 1;
    if (exp->type == TYPE_CONS) return 1;
    return 0;
}

int sexp_to_bool(sExp *exp) { return exp != NIL; }

sExp *read_atom(FILE *input) {
    char buffer[256];
    int i =0;
    int character;

    while ((character = fgetc(input)) != EOF && !isspace(character) && character != '(' && character != ')') {
        buffer[i++] = character;
    }

    if (character != EOF) ungetc(character, input);
    buffer[i] = '\0';

    char *end;
    long val = strtol(buffer, &end, 10);
    if (*end == '\0') return make_int(val);

    char *end2;
    double dval = strtod(buffer, &end2);
    if (*end2 == '\0') return make_double(dval);

    return make_symbol(buffer);
}

sExp *read_string(FILE *input) {
    char buffer[256];
    int i = 0;
    int character;

    while ((character = fgetc(input)) != EOF && character != '"') {
        buffer[i++] = character;
    }

    buffer[i] = '\0';
    return make_string(buffer);
}

sExp *read_list(FILE *input) {
    skip_ws(input);

    int character = peek(input);

    if (character == ')') {
        fgetc(input);
        // return make_nil();
        return NIL;
    }

    sExp *car = read_sexp(input);
    sExp *cdr = read_list(input);

    return cons(car, cdr); 
}

sExp *read_sexp(FILE *input) {
    skip_ws(input);
    int character = fgetc(input);

    if (character == EOF) return NULL;

    if (character == '(') return read_list(input);
    else if (character == '"') return read_string(input);
    else {
        ungetc(character, input);
        return read_atom(input);
    }
}

void  print_sexp(sExp *exp) {
    switch (exp->type) {
        case TYPE_NIL:
            printf("()");
            break;
        case TYPE_INT:
            printf("%ld", exp->intVal);
            break;
        case TYPE_DOUBLE:
            printf("%f", exp->doubleVal);
            break;
        case TYPE_SYMBOL:
            printf("%s", exp->strVal);
            break;
        case TYPE_STRING:
            printf("\"%s\"", exp->strVal);
            break;
        case TYPE_CONS: {
            printf("(");
            sExp *cur = exp;
            while (cur->type == TYPE_CONS) {
                print_sexp(cur->cons.car);
                if (cur->cons.cdr->type == TYPE_NIL) break;
                else if (cur->cons.cdr->type == TYPE_CONS) {
                    printf(" ");
                    cur = cur->cons.cdr;
                } else {
                    printf(" . ");
                    print_sexp(cur->cons.cdr);
                    break;
                }
            }
            printf(")");
            break;
        }
    }
}

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