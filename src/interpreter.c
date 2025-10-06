#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "interpreter.h"

sExp *NIL;   
sExp *TRUE; 
sExp *global_env = NULL;
int should_quit = 0;    
int exit_code   = 0; 
sExp *exit_value = NULL;

sExp *read_sexp(FILE *in);
void print_sexp(sExp *exp);

static inline sExp *make_env_frame(void) { return cons(NIL, NIL); }

void init_runtime() {
    NIL = malloc(sizeof(sExp));
    NIL->type = TYPE_NIL;

    TRUE = malloc(sizeof(sExp));
    TRUE->type = TYPE_SYMBOL;
    TRUE->strVal = strdup("t");

    exit_value = NIL;

    global_env = make_env();
}

// -- Global Environment Functions --
sExp *make_env() {
    return cons(make_env_frame(), NIL);
}

sExp *extend_env(sExp *parent) {
    return cons(make_env_frame(), parent);
}

void set_car(sExp *pair, sExp *new_car) {
    if (pair->type != TYPE_CONS) {
        fprintf(stderr, "Error: set_car called on non-cons cell.\n");
        return;
    }
    pair->cons.car = new_car;
}

void set_cdr(sExp *pair, sExp *new_cdr) {
    if (pair->type != TYPE_CONS) {
        fprintf(stderr, "Error: set_cdr called on non-cons cell.\n");
        return;
    }
    pair->cons.cdr = new_cdr;
}

sExp *find_symbol(sExp *target, sExp *symbols, sExp *values) {
    if (is_nil(symbols)) return cons(make_symbol("undefined"), cons(target, NIL));

    if (sexp_to_bool(eq(car(symbols), target))) return car(values);

    return find_symbol(target, cdr(symbols), cdr(values));
}

static int is_symbol_with(sExp *x) { return x && x->type == TYPE_SYMBOL && x->strVal; }

sExp *lookup_in_env(sExp *symbol, sExp *env) {
    while (!is_nil(env)) {
        sExp *frame = car(env);
        if (frame && frame->type == TYPE_CONS) {
            sExp *symbols = car(frame);  
            sExp *values  = cdr(frame);  
            while (!is_nil(symbols) && !is_nil(values)) {
                sExp *key = car(symbols);
                if (is_symbol_with(key) && is_symbol_with(symbol) &&
                    strcmp(key->strVal, symbol->strVal) == 0) {
                    return car(values);
                }
                symbols = cdr(symbols);
                values  = cdr(values);
            }
        }
        env = cdr(env);
    }

    if (is_symbol_with(symbol) && strcmp(symbol->strVal, "t") == 0)  return TRUE;
    if (is_symbol_with(symbol) && strcmp(symbol->strVal, "nil") == 0) return NIL;

    return symbol;
}

sExp *set_symbol_in_env(sExp *symbol, sExp *value, sExp *env) {
    sExp *frame = car(env);
    if (!frame || frame->type != TYPE_CONS) {
        frame = make_env_frame();
        set_car(env, frame);
    }

    sExp *symbols = car(frame);
    sExp *values  = cdr(frame);

    sExp *s = symbols;
    sExp *v = values;
    while (!is_nil(s) && !is_nil(v)) {
        sExp *key = car(s);
        if (is_symbol_with(key) && is_symbol_with(symbol) &&
            strcmp(key->strVal, symbol->strVal) == 0) {
            set_car(v, value);
            return value;
        }
        s = cdr(s);
        v = cdr(v);
    }

    set_car(frame, cons(symbol, symbols));   
    set_cdr(frame, cons(value,  values)); 
    return value;
}


// -- Core Read and Write Functions --
int peek(FILE *input) {
    int character = fgetc(input);
    if (character != EOF) ungetc(character, input);
    return character;
}

void skip_ws_and_comments(FILE *input) {
    int character;
    for (;;) {
        character = fgetc(input);
        if (character == EOF) return;
        if (isspace(character)) continue;
        if (character == ';') { while ((character = fgetc(input)) != EOF && character != '\n'){} continue; }
        ungetc(character, input); return;
    }
}

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

sExp *make_lambda(sExp *params, sExp *body, sExp *env) {
    sExp *lambda = malloc(sizeof(sExp));
    lambda->type = TYPE_LAMBDA;
    lambda->cons.car = params;           
    lambda->cons.cdr = cons(body, env);  
    return lambda;
}

static inline sExp *closure_params(sExp *fn) { return fn->cons.car; }
static inline sExp *closure_body  (sExp *fn) { return car(fn->cons.cdr); }
static inline sExp *closure_env   (sExp *fn) { return cdr(fn->cons.cdr); }

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

    while ((character = fgetc(input)) != EOF && !isspace(character) && character != '(' && character != ')' && character != ';') {
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
    skip_ws_and_comments(input);

    int character = peek(input);

    if (character == ')') {
        fgetc(input);
        return NIL;
    }

    sExp *car = read_sexp(input);
    sExp *cdr = read_list(input);

    return cons(car, cdr); 
}

sExp *read_sexp(FILE *input) {
    skip_ws_and_comments(input);
    int character = fgetc(input);

    if (character == EOF) return NULL;

    if (character == '(') return read_list(input);
    else if (character == '"') return read_string(input);
    else if (character == '\'') {
        sExp *q = read_sexp(input);
        return cons(make_symbol("quote"), cons(q, NIL));
    } else {
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
    return eval_in_env(sexp, global_env);
}

sExp *eval_in_env(sExp *sexp, sExp *env) {
    if (is_nil(sexp)) return NIL;

    if (sexp->type == TYPE_INT || sexp->type == TYPE_DOUBLE || sexp->type == TYPE_STRING)
        return sexp;

    if (sexp->type == TYPE_SYMBOL)
        return lookup_in_env(sexp, env);

    if (is_list(sexp)) {
        sExp *fn = car(sexp);
        sExp *args = cdr(sexp);

        if (fn->type == TYPE_SYMBOL) {
            const char *name = fn->strVal;

            if (strcmp(name, "quote") == 0)
                return car(args);
            if (strcmp(name, "lambda") == 0) {
                sExp *params = car(args);
                sExp *body   = car(cdr(args));
                return make_lambda(params, body, env);
            }
            if (strcmp(name, "set") == 0) {
                sExp *symbol = car(args);
                sExp *value = eval_in_env(car(cdr(args)), env);
                return set_symbol_in_env(symbol, value, env);
            }
            if (strcmp(name, "define") == 0) {
                sExp *name   = car(args);
                sExp *params = car(cdr(args));
                sExp *body   = car(cdr(cdr(args)));

                sExp *lambda = make_lambda(params, body, env);
                set_symbol_in_env(name, lambda, global_env);
                return name; 
            }
            if (strcmp(name, "if") == 0) {
                sExp *test = eval_in_env(car(args), env);
                if (!is_nil(test))
                    return eval_in_env(car(cdr(args)), env);
                else
                    return eval_in_env(car(cdr(cdr(args))), env);
            }
            if (strcmp(name, "cond") == 0)
                return eval_cond_in_env(args, env);
            if (strcmp(name, "and") == 0) {
                sExp *xs = args;
                if (is_nil(xs)) return TRUE;       
                sExp *last = TRUE;
                while (!is_nil(xs)) {
                    last = eval_in_env(car(xs), env);
                    if (is_nil(last)) return NIL; 
                    xs = cdr(xs);
                }
                return last;
            }
            if (strcmp(name, "or") == 0) {
                sExp *xs = args;
                while (!is_nil(xs)) {
                    sExp *v = eval_in_env(car(xs), env);
                    if (!is_nil(v)) return TRUE;   // short-circuit true
                    xs = cdr(xs);
                }
                return NIL;  
            }
            if ((strcmp(name, "add") == 0) || (strcmp(name, "+") == 0))
                return add(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "sub") == 0) || (strcmp(name, "-") == 0))
                return sub(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "mul") == 0) || (strcmp(name, "*") == 0))
                return mul(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "div") == 0) || (strcmp(name, "/") == 0))
                return divide(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "mod") == 0) || (strcmp(name, "%") == 0))
                return mod(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "eq") == 0) || (strcmp(name, "==") == 0))
                return eq(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "lt") == 0) || (strcmp(name, "<") == 0))
                return lt(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "gt") == 0) || (strcmp(name, ">") == 0))
                return gt(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "lte") == 0) || (strcmp(name, "<=") == 0))
                return lte(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "gte") == 0) || (strcmp(name, ">=") == 0))
                return gte(eval_in_env(car(args), env), eval_in_env(car(cdr(args)), env));
            if ((strcmp(name, "not") == 0) || (strcmp(name, "!") == 0))
                return logical_not(eval_in_env(car(args), env));
            if (strcmp(name, "car") == 0) {
                sExp *lst = eval_in_env(car(args), env);
                return car(lst);  
            }
            if (strcmp(name, "cdr") == 0) {
                sExp *lst = eval_in_env(car(args), env);
                return cdr(lst);
            }
            if (strcmp(name, "cons") == 0) {
                sExp *a = eval_in_env(car(args), env);
                sExp *d = eval_in_env(car(cdr(args)), env);
                return cons(a, d);
            }
            if (strcmp(name, "nil?") == 0) {
                sExp *v = eval_in_env(car(args), env);
                return is_nil(v) ? TRUE : NIL;
            }
            if (strcmp(name, "print") == 0) {
                sExp *v = eval_in_env(car(args), env);
                print_sexp(v);
                printf("\n");
                return v;  
            }
            if (strcmp(name, "exit") == 0) {
                int code = 0;
                sExp *val = NIL;

                if (is_nil(args)) {
                    val = make_string("Goodbye!");
                } else {
                    val = eval_in_env(car(args), env);
                    if (val && val->type == TYPE_INT) code = (int)val->intVal;
                }

                should_quit = 1;
                exit_code   = code;
                exit_value  = val;                         
                return val ? val : NIL;                   
            }
        }

        sExp *func = eval_in_env(fn, env);
        if (func->type == TYPE_LAMBDA) {
            sExp *params    = closure_params(func);
            sExp *body_form = closure_body(func);
            sExp *def_env   = closure_env(func);
            sExp *local_env = extend_env(def_env);

            sExp *arg_list = args;
            sExp *param_list = params;

            while (!is_nil(arg_list) && !is_nil(param_list)) {
                sExp *value = eval_in_env(car(arg_list), env); 
                set_symbol_in_env(car(param_list), value, local_env);
                arg_list    = cdr(arg_list);
                param_list  = cdr(param_list);
            }
            return eval_in_env(body_form, local_env);
        }
        return make_symbol("UnknownFunction");
    }

    return make_symbol("EvalError");
}

sExp *eval_cond_in_env (sExp *clauses, sExp *env) {
    if (is_nil(clauses)) return NIL;

    sExp *clause = car(clauses);
    if (is_nil(clause)) return eval_cond_in_env(cdr(clauses), env);

    sExp *test = car(clause);
    sExp *rest = cdr(clause);           
    sExp *test_eval;

    if (test->type == TYPE_SYMBOL && strcmp(test->strVal, "t") == 0) test_eval = TRUE;
    else test_eval = eval_in_env(test, env);

    if (!is_nil(test_eval)) {
        if (is_nil(rest)) return TRUE;

        sExp *result = car(rest);
        return eval_in_env(result, env);
    }

    return eval_cond_in_env(cdr(clauses), env);
}

