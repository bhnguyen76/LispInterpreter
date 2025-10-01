#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interpreter.h"

int tests_passed = 0;
int tests_failed = 0;

void assert_int(long expected, sExp *result, const char *description, const char *input) {
    printf("Input: %s\n", input);
    printf("Expected: %ld\n", expected);

    if (result->type == TYPE_INT) {
        printf("Result: %ld\n", result->intVal);
        if (result->intVal == expected) {
            printf("[PASS]\n\n");
            tests_passed++;
            return;
        }
    } else {
        printf("Result: ");
        print_sexp(result);
        printf("\n");
    }

    printf("[FAIL]\n\n");
    tests_failed++;
}

void assert_double(double expected, sExp *result, const char *description, const char *input) {
    printf("Input: %s\n", input);
    printf("Expected: %f\n", expected);

    if (result->type == TYPE_DOUBLE) {
        printf("Result: %f\n", result->doubleVal);
        if (fabs(result->doubleVal - expected) < 1e-6) {
            printf("[PASS]\n\n");
            tests_passed++;
            return;
        }
    } else {
        printf("Result: ");
        print_sexp(result);
        printf("\n");
    }

    printf("[FAIL]\n\n");
    tests_failed++;
}

void assert_symbol(const char *expected, sExp *result, const char *description, const char *input) {
    printf("Input: %s\n", input);
    printf("Expected: %s\n", expected);

    if (result->type == TYPE_SYMBOL) {
        printf("Result: %s\n", result->strVal);
        if (strcmp(result->strVal, expected) == 0) {
            printf("[PASS]\n\n");
            tests_passed++;
            return;
        }
    } else {
        printf("Result: ");
        print_sexp(result);
        printf("\n");
    }
    printf("[FAIL]\n\n");
    tests_failed++;
}

void assert_string(const char *expected, sExp *result, const char *description, const char *input) {
    printf("Input: %s\n", input);
    printf("Expected: \"%s\"\n", expected);
    if (result->type == TYPE_STRING) {
        printf("Result: \"%s\"\n", result->strVal);
        if (strcmp(result->strVal, expected) == 0) {
            printf("[PASS]\n\n");
            tests_passed++;
            return;
        }
    } else {
        printf("Result: ");
        print_sexp(result);
        printf("\n");
    }
    printf("[FAIL]\n\n");
    tests_failed++;
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


void assert_list(sExp *expected, sExp *result, const char *description, const char *input) {
    printf("Input: %s\n", input);
    printf("Expected: ");
    print_sexp(expected);
    printf("\nResult: ");
    print_sexp(result);
    printf("\n");
    if (sexp_equal(expected, result)) {
        printf("[PASS]\n\n");
        tests_passed++;
    } else {
        printf("[FAIL]\n\n");
        tests_failed++;
    }
}

void assert_true(sExp *result, const char *description, const char *input) {
    printf("Input: %s\n", input);
    printf("Expected: TRUE\nResult: ");
    print_sexp(result);
    printf("\n");
    if (result == TRUE) {
        printf("[PASS]\n\n");
        tests_passed++;
    } else {
        printf("[FAIL]\n\n");
        tests_failed++;
    }
}

void assert_nil(sExp *result, const char *description, const char *input) {
    printf("Input: %s\n", input);
    printf("Expected: NIL\nResult: ");
    print_sexp(result);
    printf("\n");
    if (result == NIL) {
        printf("[PASS]\n\n");
        tests_passed++;
    } else {
        printf("[FAIL]\n\n");
        tests_failed++;
    }
}

void test_constructors() {
    assert_int(42, make_int(42), "int constructor", "make_int(42)");
    assert_double(3.14, make_double(3.14), "double constructor", "make_double(3.14)");
    assert_symbol("foo", make_symbol("foo"), "symbol constructor", "make_symbol(\"foo\")");
    assert_string("bar", make_string("bar"), "string constructor", "make_string(\"bar\")");

    sExp *list = cons(make_int(1), cons(make_int(2), NIL));
    sExp *expected = cons(make_int(1), cons(make_int(2), NIL));
    assert_list(expected, list, "cons constructor", "cons(make_int(1), cons(make_int(2), NIL))");

    printf("\n");

    assert_nil(NIL, "NIL singleton", "NIL");

    assert_true(TRUE, "TRUE singleton", "TRUE");
    printf("\n");
}

void test_parser() {
    {
        const char *src = "123";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        assert_int(123, result, "parse int 123", "123");
        fclose(f);
    }

    {
        const char *src = "hello";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        assert_symbol("hello", result, "parse symbol hello", "hello");
        fclose(f);
    }

    {
        const char *src = "\"world\"";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        assert_string("world", result, "parse string \"world\"", "\"world\"");
        fclose(f);
    }

    {
        const char *src = "(1 2 3)";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        sExp *expected = cons(make_int(1), cons(make_int(2), cons(make_int(3), NIL)));
        assert_list(expected, result, "parse list (1 2 3)", "(1 2 3)");
        fclose(f);
    }

    {
        const char *src = "(a (b c) d)";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *result = read_sexp(f);
        sExp *expected = cons(make_symbol("a"), cons(cons(make_symbol("b"), cons(make_symbol("c"), NIL)), cons(make_symbol("d"), NIL)));
        assert_list(expected, result, "parse nested list (a (b c) d)", "(a (b c) d)");
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

    assert_true(is_number(i) ? TRUE : NIL, "is_number detects int", "is_number(10)");
    assert_true(is_number(d) ? TRUE : NIL, "is_number detects double", "is_number(2.5)");
    assert_nil(is_number(str) ? TRUE : NIL, "is_number rejects string", "is_number(\"xyz\")");

    assert_true(is_symbol(sym) ? TRUE : NIL, "is_symbol detects symbol", "is_symbol(abc)");
    assert_nil(is_symbol(i) ? TRUE : NIL, "is_symbol rejects int", "is_symbol(10)");

    assert_true(is_string(str) ? TRUE : NIL, "is_string detects string", "is_string(\"xyz\")");
    assert_nil(is_string(d) ? TRUE : NIL, "is_string rejects double", "is_string(2.5)");

    assert_true(is_list(list) ? TRUE : NIL, "is_list detects cons", "is_list(10 2.5)");
    assert_true(is_list(NIL) ? TRUE : NIL, "is_list detects NIL as list", "is_list(NIL)");
    assert_nil(is_list(i) ? TRUE : NIL, "is_list rejects int", "is_list(10)");
}

void test_bool() {
    assert_true(sexp_to_bool(TRUE) ? TRUE : NIL, "sexp_to_bool(TRUE) is true", "sexp_to_bool(TRUE)");
    assert_nil(sexp_to_bool(NIL) ? TRUE : NIL, "sexp_to_bool(NIL) is false", "sexp_to_bool(NIL)");
    printf("\n");
}

void test_car_cdr() {
    sExp *one = make_int(1);
    sExp *two = make_int(2);
    sExp *pair = cons(one, two);

    assert_int(1, car(pair), "car of (1 . 2) = 1", "car((1 2))");
    assert_int(2, cdr(pair), "cdr of (1 . 2) = 2", "cdr((1 2))");

    sExp *list = cons(make_int(1), cons(make_int(2), cons(make_int(3), NIL)));

    assert_int(1, car(list), "car of (1 2 3) = 1", "car((1 2 3))");

    sExp *rest = cdr(list);
    sExp *expected = cons(make_int(2), cons(make_int(3), NIL));
    assert_list(expected, rest, "cdr of (1 2 3) = (2 3)", "cdr((1 2 3))");

    assert_nil(car(NIL), "car of NIL = NIL", "car(NIL)");
    assert_nil(cdr(NIL), "cdr of NIL = NIL", "cdr((NIL))");

    assert_nil(car(one), "car of int = NIL", "car(1)");
    assert_nil(cdr(one), "cdr of int = NIL", "cdr(1)");

    printf("\n");
}


void test_arithmetic() {
    sExp *a = make_int(5);
    sExp *b = make_int(3);

    assert_int(8, add(a,b), "5 + 3 = 8", "add(5,3)");
    assert_int(2, sub(a,b), "5 - 3 = 2", "sub(5,3)");
    assert_int(15, mul(a,b), "5 * 3 = 15", "mul(5,3)");
    assert_double(1.666667, divide(a,b), "5 / 3 ≈ 1.666667", "divide(5,3)");
    assert_int(2, mod(a,b), "5 % 3 = 2", "mod(5,3)");
}

void test_relations() {
    sExp *a = make_int(5);
    sExp *b = make_int(3);
    sExp *c = make_int(5);

    assert_true(gt(a,b), "5 > 3", "gt(5,3)");
    assert_true(lt(b,a), "3 < 5", "lt(3,5)");
    assert_true(gte(a,c), "5 >= 5", "gte(5,5)");
    assert_true(lte(b,a), "3 <= 5", "lte(3,5)");
}

void test_equality() {
    sExp *a = make_int(5);
    sExp *b = make_int(5);
    sExp *c = make_int(7);

    assert_true(eq(a,b), "5 == 5", "eq(5,5)");
    assert_nil(eq(a,c), "5 != 7", "eq(5, 7)");
    assert_true(eq(make_symbol("x"), make_symbol("x")), "symbols equal", "eq(x,x)");
}

void test_logical() {
    assert_true(logical_not(NIL), "not NIL = TRUE", "not(NIL)");
    assert_nil(logical_not(TRUE), "not TRUE = NIL", "not(TRUE)");
}

void test_arithmetic_errors() {
    sExp *a = make_int(5);
    sExp *zero = make_int(0);
    sExp *str = make_string("oops");

    assert_symbol("DivisionByZero", divide(a, zero), "divide by zero error", "divide(5,0)");
    assert_symbol("DivisionByZero", mod(a, zero), "mod by zero error", "mod(5,0)");
    assert_symbol("NotAnInteger", mod(a, make_double(3.14)), "mod with double should fail", "mod(5,3.14)");
    assert_symbol("NotANumber", add(a, str), "add with string should fail", "add(5,oops)");
    assert_symbol("NotANumber", sub(str, a), "sub with string should fail", "sub(5,oops)");
    assert_symbol("NotANumber", mul(a, str), "mul with string should fail", "mul(5,oops)");
    assert_symbol("NotANumber", divide(a, str), "divide with string should fail", "divide(5,oops)");
}

void test_eval() {
    // --- Atoms ---
    assert_int(42, eval(make_int(42)), "eval int", "eval 42");
    assert_double(3.14, eval(make_double(3.14)), "eval double", "eval 3.14");
    assert_symbol("x", eval(make_symbol("x")), "eval unbound symbol", "eval x");
    assert_string("hello", eval(make_string("hello")), "eval string", "eval \"hello\"");

    // --- Quote ---
    sExp *quotedInt = cons(make_symbol("quote"), cons(make_int(42), NIL));
    assert_int(42, eval(quotedInt), "quote int", "(quote 42)");

    sExp *quotedList = cons(make_symbol("quote"),
        cons(cons(make_int(1),
            cons(make_int(2),
                cons(make_int(3), NIL))),
        NIL));
    sExp *expectedList = cons(make_int(1), cons(make_int(2), cons(make_int(3), NIL)));
    assert_list(expectedList, eval(quotedList), "quote list", "(quote (1 2 3))");

    // --- Set & Symbol Lookup ---
    sExp *setExpr = cons(make_symbol("set"), cons(make_symbol("x"), cons(make_int(10), NIL)));
    assert_int(10, eval(setExpr), "set variable x", "(set x 10)");
    assert_int(10, eval(make_symbol("x")), "eval bound symbol x", "eval x");

    sExp *setExpr2 = cons(make_symbol("set"), cons(make_symbol("x"), cons(make_int(99), NIL)));
    assert_int(99, eval(setExpr2), "update variable x", "(set x 99)");
    assert_int(99, eval(make_symbol("x")), "eval updated symbol x", "eval x");

    // --- Builtins ---
    sExp *addExpr = cons(make_symbol("add"), cons(make_int(1), cons(make_int(2), NIL)));
    assert_int(3, eval(addExpr), "eval add", "(add 1 2)");

    sExp *subExpr = cons(make_symbol("sub"), cons(make_int(10), cons(make_int(7), NIL)));
    assert_int(3, eval(subExpr), "eval sub", "(sub 10 7)");

    sExp *mulExpr = cons(make_symbol("mul"), cons(make_int(4), cons(make_int(5), NIL)));
    assert_int(20, eval(mulExpr), "eval mul", "(mul 4 5)");

    sExp *divExpr = cons(make_symbol("div"), cons(make_int(11), cons(make_int(2), NIL)));
    assert_double(5.5, eval(divExpr), "eval div", "(div 11 2)");

    sExp *eqExpr = cons(make_symbol("eq"),
        cons(make_int(3),
            cons(cons(make_symbol("add"),
                cons(make_int(1), cons(make_int(2), NIL))), NIL)));
    assert_true(eval(eqExpr), "eval eq with nested add", "(eq 3 (add 1 2))");

    // --- Parsed expressions ---
    {
        const char *src = "(eq 3 (add 1 2))";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *expr = read_sexp(f);
        sExp *result = eval(expr);
        assert_true(result, "parse+eval eq", "(eq 3 (add 1 2))");
        fclose(f);
    }

    {
        const char *src = "(div 11 2)";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *expr = read_sexp(f);
        sExp *result = eval(expr);
        assert_double(5.5, result, "parse+eval div", "(div 11 2)");
        fclose(f);
    }

    {
        const char *src = "(set y 123)";
        FILE *f = fmemopen((void*)src, strlen(src), "r");
        sExp *expr = read_sexp(f);
        sExp *result = eval(expr);
        assert_int(123, result, "parse+eval set", "(set y 123)");
        fclose(f);

        assert_int(123, eval(make_symbol("y")), "eval bound symbol y", "y");
    }

    printf("\n");
}


int main() {
    init_runtime();

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
    
    printf("=== Sprint 4: Eval ===\n");
    test_eval();

    printf("\nTests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
