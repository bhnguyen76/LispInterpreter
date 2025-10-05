#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "interpreter.h"

sExp *NIL;   
sExp *TRUE; 
sExp *global_env = NULL;

sExp *read_sexp(FILE *in);
void print_sexp(sExp *exp);

void init_runtime() {
    // Initialize NIL
    NIL = malloc(sizeof(sExp));
    NIL->type = TYPE_NIL;

    // Initialize TRUE
    TRUE = malloc(sizeof(sExp));
    TRUE->type = TYPE_SYMBOL;
    TRUE->strVal = strdup("t");

    // Initialize global_env
    global_env = make_env();
}

// -- Global Environment Functions --
sExp *make_env() {
    return cons(NIL, cons(NIL, NIL));
}

sExp *find_symbol(sExp *target, sExp *symbols, sExp *values) {
    if (is_nil(symbols)) return cons(make_symbol("undefined"), cons(target, NIL));

    if (sexp_to_bool(eq(car(symbols), target))) return car(values);

    return find_symbol(target, cdr(symbols), cdr(values));
}

sExp *lookup(sExp *target) {
    if (target->type == TYPE_SYMBOL) {
        if (strcmp(target->strVal, "t") == 0) return TRUE;
        if (strcmp(target->strVal, "nil") == 0) return NIL;
    }

    sExp *symbols = car(global_env);
    sExp *values = car(cdr(global_env));

    return find_symbol(target, symbols, values);
}

sExp *set_symbol(sExp *symbol, sExp *value) {
    sExp *symbols = car(global_env);
    sExp *values  = car(cdr(global_env));

    sExp *new_symbols = cons(symbol, symbols);
    sExp *new_values  = cons(value, values);

    global_env = cons(new_symbols, cons(new_values, NIL));

    return value;
}


// sExp* lookup(sExp* sym, sExp* env) {
//     while (!is_nil(env)) {
//         sExp* pair = car(env);
//         if (strcmp(car(pair)->strVal, sym->strVal) == 0) {
//             return cdr(pair);
//         }
//         env = cdr(env);
//     }
//     return NIL; // not found
// }

// void env_set(sExp* sym, sExp* val, sExp** env) {
//     sExp* e = *env;
//     while (!is_nil(e)) {
//         sExp* pair = car(e);
//         if (strcmp(car(pair)->strVal, sym->strVal) == 0) {
//             // update existing
//             pair->cons.cdr = val;
//             return;
//         }
//         e = cdr(e);
//     }
//     // not found → prepend new binding
//     *env = cons(cons(sym, val), *env);
// }

// -- Core Read and Write Functions --
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

// -- Predicates --
int is_nil(sExp *exp)    { return exp->type == TYPE_NIL; }

int is_symbol(sExp *exp) { return exp->type == TYPE_SYMBOL; }

int is_number(sExp *exp) { return exp->type == TYPE_INT || exp->type == TYPE_DOUBLE; }

int is_string(sExp *exp) { return exp->type == TYPE_STRING; }

int is_list(sExp *exp) {
    return exp->type == TYPE_NIL ||
           (exp->type == TYPE_CONS && is_list(exp->cons.cdr));
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

// -- eval function --
sExp *eval(sExp *sexp) {
    if (is_nil(sexp)) {
        return NIL;
    }

    if (sexp->type == TYPE_INT || sexp->type == TYPE_DOUBLE || sexp->type == TYPE_STRING) {
        return sexp;
    }

    if (sexp->type == TYPE_SYMBOL) {
        return lookup(sexp); 
    }

    if (is_list(sexp)) {
        sExp *fn = car(sexp);
        sExp *args = cdr(sexp);

        if (fn->type == TYPE_SYMBOL && strcmp(fn->strVal, "quote") == 0) {
            return car(args);
        }

        if (strcmp(fn->strVal, "set") == 0) {
            sExp *symbol = car(args);
            sExp *value = eval(car(cdr(args)));
            // return env_set(symbol, value, &global_env);
            return set_symbol(symbol, value);
        }

        if (strcmp(fn->strVal, "add") == 0) {
            return add(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "sub") == 0) {
            return sub(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "mul") == 0) {
            return mul(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "div") == 0) {
            return divide(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "mod") == 0) {
            return mod(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "eq") == 0) {
            return eq(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "not") == 0) {
            return logical_not(eval(car(args)));
        }
        if (strcmp(fn->strVal, "lt") == 0) {
            return lt(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "gt") == 0) {
            return gt(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "lte") == 0) {
            return lte(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "gte") == 0) {
            return gte(eval(car(args)), eval(car(cdr(args))));
        }
        if (strcmp(fn->strVal, "and") == 0) {
            sExp *first = eval(car(args));
            if (is_nil(first)) return NIL;  
            return eval(car(cdr(args)));
        }
        if (strcmp(fn->strVal, "or") == 0) {
            sExp *first = eval(car(args));
            if (!is_nil(first)) return TRUE;  
            return eval(car(cdr(args)));
        }
        if (strcmp(fn->strVal, "if") == 0) {
            sExp *test = eval(car(args));
            if (!is_nil(test)) {
                return eval(car(cdr(args)));  
            } else {
                return eval(car(cdr(cdr(args)))); 
            }
        }
        if (strcmp(fn->strVal, "cond") == 0) {
            return eval_cond(args);
        }
    }

    return make_symbol("UnknownFunction");
}

sExp *eval_cond (sExp *clauses) {
    if (is_nil(clauses)) {
        return NIL;
    } 

    sExp *clause = car(clauses);   

    if (is_nil(clause)) return eval_cond(cdr(clauses));

    sExp *test   = car(clause);
    sExp *result = car(cdr(clause));

    // Evaluate test
    sExp *test_eval;
    if (test->type == TYPE_SYMBOL && strcmp(test->strVal, "t") == 0)
        test_eval = TRUE;
    else
        test_eval = eval(test);

    // If test is true → evaluate and return result
    if (!is_nil(test_eval)) {
        if (is_nil(result)) return TRUE;   // handle case like (cond ((t)))
        return eval(result);
    }

    return eval_cond(cdr(clauses));
}
