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

// -- Core Read and Write Functions
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

// -- Arithmetic --
sExp *add(sExp *a, sExp *b) {
    if (!is_number(a) || !is_number(b)) return make_symbol("NotANumber");

    if (a->type == TYPE_DOUBLE || b->type == TYPE_DOUBLE) {
        double x = (a->type == TYPE_DOUBLE ? a->doubleVal : a->intVal);
        double y = (b->type == TYPE_DOUBLE ? b->doubleVal : b->intVal);
        return make_double(x + y);
    }

    return make_int(a->intVal + b->intVal);
}

sExp *sub(sExp *a, sExp *b) {
    if (!is_number(a) || !is_number(b)) return make_symbol("NotANumber");

    if (a->type == TYPE_DOUBLE || b->type == TYPE_DOUBLE) {
        double x = (a->type == TYPE_DOUBLE ? a->doubleVal : a->intVal);
        double y = (b->type == TYPE_DOUBLE ? b->doubleVal : b->intVal);
        return make_double(x - y);
    }

    return make_int(a->intVal - b->intVal);
}

sExp *mul(sExp *a, sExp *b) {
    if (!is_number(a) || !is_number(b)) return make_symbol("NotANumber");

    if (a->type == TYPE_DOUBLE || b->type == TYPE_DOUBLE) {
        double x = (a->type == TYPE_DOUBLE ? a->doubleVal : a->intVal);
        double y = (b->type == TYPE_DOUBLE ? b->doubleVal : b->intVal);
        return make_double(x * y);
    }

    return make_int(a->intVal * b->intVal);
}

sExp *divide(sExp *a, sExp *b) {
    if (!is_number(a) || !is_number(b)) return make_symbol("NotANumber");
    double y = (b->type == TYPE_DOUBLE ? b->doubleVal : b->intVal);
    if (y == 0) return make_symbol("DivisionByZero");

    double x = (a->type == TYPE_DOUBLE ? a->doubleVal : a->intVal);
    return make_double(x / y);
}

sExp *mod(sExp *a, sExp *b) {
    if (a->type != TYPE_INT || b->type != TYPE_INT) return make_symbol("NotAnInteger");
    if (b->intVal == 0) return make_symbol("DivisionByZero");
    return make_int(a->intVal % b->intVal);
}

sExp *lt(sExp *a, sExp *b) {
    if (!is_number(a) || !is_number(b)) return make_symbol("NotANumber");
    double x = (a->type == TYPE_DOUBLE ? a->doubleVal : a->intVal);
    double y = (b->type == TYPE_DOUBLE ? b->doubleVal : b->intVal);
    return (x < y) ? TRUE : NIL;
}

sExp *gt(sExp *a, sExp *b) {
    if (!is_number(a) || !is_number(b)) return make_symbol("NotANumber");
    double x = (a->type == TYPE_DOUBLE ? a->doubleVal : a->intVal);
    double y = (b->type == TYPE_DOUBLE ? b->doubleVal : b->intVal);
    return (x > y) ? TRUE : NIL;
}

sExp *lte(sExp *a, sExp *b) {
    if (!is_number(a) || !is_number(b)) return make_symbol("NotANumber");
    double x = (a->type == TYPE_DOUBLE ? a->doubleVal : a->intVal);
    double y = (b->type == TYPE_DOUBLE ? b->doubleVal : b->intVal);
    return (x <= y) ? TRUE : NIL;
}

sExp *gte(sExp *a, sExp *b) {
    if (!is_number(a) || !is_number(b)) return make_symbol("NotANumber");
    double x = (a->type == TYPE_DOUBLE ? a->doubleVal : a->intVal);
    double y = (b->type == TYPE_DOUBLE ? b->doubleVal : b->intVal);
    return (x >= y) ? TRUE : NIL;
}

sExp *eq(sExp *a, sExp *b) {
    if (a->type != b->type) return NIL;
    switch (a->type) {
        case TYPE_INT:    return (a->intVal == b->intVal) ? TRUE : NIL;
        case TYPE_DOUBLE: return (a->doubleVal == b->doubleVal) ? TRUE : NIL;
        case TYPE_STRING:
        case TYPE_SYMBOL: return (strcmp(a->strVal, b->strVal) == 0) ? TRUE : NIL;
        case TYPE_NIL:    return TRUE;
        default:          return NIL;
    }
}

sExp *logical_not(sExp *a) {
    return (a == NIL) ? TRUE : NIL;
}