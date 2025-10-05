#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>

typedef enum { TYPE_NIL, TYPE_INT, TYPE_DOUBLE, TYPE_SYMBOL, TYPE_STRING, TYPE_CONS } sType;

typedef struct sExp {
    sType type;
    union {
        long intVal;
        double doubleVal;
        char *strVal;
        struct {
            struct sExp *car;
            struct sExp *cdr;
        } cons;
    };
} sExp;

extern sExp *NIL;
extern sExp *TRUE;
extern sExp *global_env;

void init_runtime();

// -- Environment --
sExp *make_env();
sExp *lookup(sExp *symbol);
sExp *set_symbol(sExp *symbol, sExp *value);
// sExp* lookup(sExp* sym, sExp* env);
// void  env_set(sExp* sym, sExp* val, sExp** env);

// -- Constructors --
sExp *make_int(long val);
sExp *make_double(double val);
sExp *make_symbol(const char *s);
sExp *make_string(const char *s);
sExp *cons(sExp *car, sExp *cdr);

// -- List Helpers --
sExp *car(sExp *exp);
sExp *cdr(sExp *exp);

// -- Predicates --
int is_nil(sExp *exp);
int is_symbol(sExp *exp);
int is_number(sExp *exp);
int is_string(sExp *exp);
int is_list(sExp *exp);
int sexp_to_bool(sExp *exp);

// -- Reader and Write --
sExp *read_sexp(FILE *in);
void  print_sexp(sExp *exp);

// -- Arithmetic --
sExp *add(sExp *a, sExp *b);
sExp *sub(sExp *a, sExp *b);
sExp *mul(sExp *a, sExp *b);
sExp *divide(sExp *a, sExp *b);
sExp *mod(sExp *a, sExp *b);

sExp *lt(sExp *a, sExp *b);
sExp *gt(sExp *a, sExp *b);
sExp *lte(sExp *a, sExp *b);
sExp *gte(sExp *a, sExp *b);

sExp *eq(sExp *a, sExp *b);
sExp *logical_not(sExp *a);

// -- evalutaion --
sExp *eval(sExp *sexp);
sExp *eval_cond (sExp *clauses);

#endif
