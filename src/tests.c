#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interpreter.h"

int tests_passed = 0;
int tests_failed = 0;

void assert_int(long expected, sExp *result, const char *msg) {
    if (result->type == TYPE_INT && result->intVal == expected) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s (expected %ld, got ", msg, expected);
        print_sexp(result);
        printf(")\n");
        tests_failed++;
    }
}

void assert_double(double expected, sExp *result, const char *msg) {
    if (result->type == TYPE_DOUBLE && fabs(result->doubleVal - expected) < 1e-6) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s (expected %f, got ", msg, expected);
        print_sexp(result);
        printf(")\n");
        tests_failed++;
    }
}

void assert_symbol(const char *expected, sExp *result, const char *msg) {
    if (result->type == TYPE_SYMBOL && strcmp(result->strVal, expected) == 0) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s (expected symbol \"%s\", got ", msg, expected);
        print_sexp(result);
        printf(")\n");
        tests_failed++;
    }
}

void assert_string(const char *expected, sExp *result, const char *msg) {
    if (result->type == TYPE_STRING && strcmp(result->strVal, expected) == 0) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s (expected string \"%s\", got ", msg, expected);
        print_sexp(result);
        printf(")\n");
        tests_failed++;
    }
}

int sexp_equal(sExp *a, sExp *b) {
    if (a->type != b->type) return 0;

    switch (a->type) {
        case TYPE_NIL:
            return 1;
        case TYPE_INT:
            return a->intVal == b->intVal;
        case TYPE_DOUBLE:
            return a->doubleVal == b->doubleVal;
        case TYPE_SYMBOL:
        case TYPE_STRING:
            return strcmp(a->strVal, b->strVal) == 0;
        case TYPE_CONS:
            return sexp_equal(a->cons.car, b->cons.car) &&
                   sexp_equal(a->cons.cdr, b->cons.cdr);
    }
    return 0; 
}

void assert_list(sExp *expected, sExp *result, const char *msg) {
    if (sexp_equal(expected, result)) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s (expected ", msg);
        print_sexp(expected);
        printf(", got ");
        print_sexp(result);
        printf(")\n");
        tests_failed++;
    }
}

void assert_true(sExp *result, const char *msg) {
    if (result == TRUE) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s (expected TRUE, got ", msg);
        print_sexp(result);
        printf(")\n");
        tests_failed++;
    }
}

void assert_nil(sExp *result, const char *msg) {
    if (result == NIL) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s (expected NIL, got ", msg);
        print_sexp(result);
        printf(")\n");
        tests_failed++;
    }
}

void test_constructors() {
    assert_int(42, make_int(42), "make_int works");
    assert_double(3.14, make_double(3.14), "make_double works");
    assert_symbol("foo", make_symbol("foo"), "make_symbol works");
    assert_string("bar", make_string("bar"), "make_string works");

    sExp *list = cons(make_int(1), cons(make_int(2), NIL));
    sExp *expected = cons(make_int(1), cons(make_int(2), NIL));
    assert_list(expected, list, "cons works");

    printf("\n");

    assert_nil(NIL, "NIL singleton");

    assert_true(TRUE, "TRUE singleton");
    printf("\n");
}

void test_parser() {
    {
        const char *src = "123";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        assert_int(123, result, "parse int 123");
        fclose(f);
    }

    {
        const char *src = "hello";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        assert_symbol("hello", result, "parse symbol hello");
        fclose(f);
    }

    {
        const char *src = "\"world\"";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        assert_string("world", result, "parse string \"world\"");
        fclose(f);
    }

    {
        const char *src = "(1 2 3)";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        sExp *expected = cons(make_int(1), cons(make_int(2), cons(make_int(3), NIL)));
        assert_list(expected, result, "parse list (1 2 3)");
        fclose(f);
    }

    {
        const char *src = "(a (b c) d)";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        sExp *expected = cons(make_symbol("a"), cons(cons(make_symbol("b"), cons(make_symbol("c"), NIL)), cons(make_symbol("d"), NIL)));
        assert_list(expected, result, "parse nested list (a (b c) d)");
        fclose(f);
    }

    printf("\n");
}

void test_predicates() {

    sExp *i = make_int(10);
    sExp *d = make_double(2.5);
    sExp *sym = make_symbol("abc");
    sExp *str = make_string("xyz");
    sExp *list = cons(i, cons(d, NIL));

    assert_true(is_number(i) ? TRUE : NIL, "is_number detects int");
    assert_true(is_number(d) ? TRUE : NIL, "is_number detects double");
    assert_nil(is_number(str) ? TRUE : NIL, "is_number rejects string");

    assert_true(is_symbol(sym) ? TRUE : NIL, "is_symbol detects symbol");
    assert_nil(is_symbol(i) ? TRUE : NIL, "is_symbol rejects int");

    assert_true(is_string(str) ? TRUE : NIL, "is_string detects string");
    assert_nil(is_string(d) ? TRUE : NIL, "is_string rejects double");

    assert_true(is_list(list) ? TRUE : NIL, "is_list detects cons");
    assert_true(is_list(NIL) ? TRUE : NIL, "is_list detects NIL as list");
    assert_nil(is_list(i) ? TRUE : NIL, "is_list rejects int");
}

void test_bool() {
    assert_true(sexp_to_bool(TRUE) ? TRUE : NIL, "sexp_to_bool(TRUE) is true");
    assert_nil(sexp_to_bool(NIL) ? TRUE : NIL, "sexp_to_bool(NIL) is false");
    printf("\n");
}

void test_car_cdr() {
    sExp *one = make_int(1);
    sExp *two = make_int(2);
    sExp *pair = cons(one, two);

    assert_int(1, car(pair), "car of (1 . 2) = 1");
    assert_int(2, cdr(pair), "cdr of (1 . 2) = 2");

    sExp *list = cons(make_int(1), cons(make_int(2), cons(make_int(3), NIL)));

    assert_int(1, car(list), "car of (1 2 3) = 1");

    sExp *rest = cdr(list);
    sExp *expected = cons(make_int(2), cons(make_int(3), NIL));
    assert_list(expected, rest, "cdr of (1 2 3) = (2 3)");

    assert_nil(car(NIL), "car of NIL = NIL");
    assert_nil(cdr(NIL), "cdr of NIL = NIL");

    assert_nil(car(one), "car of int = NIL");
    assert_nil(cdr(one), "cdr of int = NIL");

    printf("\n");
}


void test_arithmetic() {
    sExp *a = make_int(5);
    sExp *b = make_int(3);

    assert_int(8, add(a,b), "5 + 3 = 8");
    assert_int(2, sub(a,b), "5 - 3 = 2");
    assert_int(15, mul(a,b), "5 * 3 = 15");
    assert_double(1.666667, divide(a,b), "5 / 3 ≈ 1.666667");
    assert_int(2, mod(a,b), "5 % 3 = 2");
}

void test_relations() {
    sExp *a = make_int(5);
    sExp *b = make_int(3);
    sExp *c = make_int(5);

    assert_true(gt(a,b), "5 > 3");
    assert_true(lt(b,a), "3 < 5");
    assert_true(gte(a,c), "5 >= 5");
    assert_true(lte(b,a), "3 <= 5");
}

void test_equality() {
    sExp *a = make_int(5);
    sExp *b = make_int(5);
    sExp *c = make_int(7);

    assert_true(eq(a,b), "5 == 5");
    assert_nil(eq(a,c), "5 != 7");
    assert_true(eq(make_symbol("x"), make_symbol("x")), "symbols equal");
}

void test_logical() {
    assert_true(logical_not(NIL), "not NIL = TRUE");
    assert_nil(logical_not(TRUE), "not TRUE = NIL");
}

void test_arithmetic_errors() {
    sExp *a = make_int(5);
    sExp *zero = make_int(0);
    sExp *str = make_string("oops");

    // Division by zero
    assert_symbol("DivisionByZero", divide(a, zero), "divide by zero error");

    // Mod by zero
    assert_symbol("DivisionByZero", mod(a, zero), "mod by zero error");

    // Mod with non-int
    assert_symbol("NotAnInteger", mod(a, make_double(3.14)), "mod with double should fail");

    // Add with non-number
    assert_symbol("NotANumber", add(a, str), "add with string should fail");

    // Sub with non-number
    assert_symbol("NotANumber", sub(str, a), "sub with string should fail");

    // Mul with non-number
    assert_symbol("NotANumber", mul(a, str), "mul with string should fail");

    // Divide with non-number
    assert_symbol("NotANumber", divide(a, str), "divide with string should fail");
}


int main() {
    NIL = malloc(sizeof(sExp));
    NIL->type = TYPE_NIL;

    TRUE = malloc(sizeof(sExp));
    TRUE->type = TYPE_SYMBOL;
    TRUE->strVal = strdup("t");

    printf("Running Tests...\n\n");

    printf("=== Sprint 1: Core ===\n");
    test_constructors();
    test_parser();

    printf("=== Sprint 2: Predicates ===\n");
    test_predicates();
    test_bool();
     test_car_cdr();
     
    printf("=== Sprint 3: Arithmetic ===\n");
    test_arithmetic();
    test_relations();
    test_equality();
    test_logical();
    test_arithmetic_errors();
    
    printf("\nTests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
