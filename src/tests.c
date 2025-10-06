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

static void run_str_expect_int(const char *src, long expected) {
    FILE *f = fmemopen((void*)src, strlen(src), "r");
    sExp *expr = read_sexp(f);
    sExp *result = eval(expr);
    fclose(f);
    assert_int(expected, result, src, src);
}

static void run_str_noassert(const char *src) {
    FILE *f = fmemopen((void*)src, strlen(src), "r");
    sExp *expr = read_sexp(f);
    (void)eval(expr);
    fclose(f);
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

void test_short_circuit_and_conditionals() {
    // ---------- AND ----------
    const char *and_tests[][2] = {
        {"(and t t)", "t"},
        {"(and nil t)", "nil"},
        {"(and t nil)", "nil"},
        {"(and)", "t"}, // empty and should return true
    };

    for (int i = 0; i < 4; i++) {
        FILE *f = fmemopen((void*)and_tests[i][0], strlen(and_tests[i][0]), "r");
        sExp *expr = read_sexp(f);
        sExp *result = eval(expr);
        if (strcmp(and_tests[i][1], "t") == 0)
            assert_true(result, "and", and_tests[i][0]);
        else
            assert_nil(result, "and", and_tests[i][0]);
        fclose(f);
    }

    // ---------- OR ----------
    const char *or_tests[][2] = {
        {"(or nil nil)", "nil"},
        {"(or t nil)", "t"},
        {"(or nil t)", "t"},
        {"(or)", "nil"}, // empty or should return nil
    };

    for (int i = 0; i < 4; i++) {
        FILE *f = fmemopen((void*)or_tests[i][0], strlen(or_tests[i][0]), "r");
        sExp *expr = read_sexp(f);
        sExp *result = eval(expr);
        if (strcmp(or_tests[i][1], "t") == 0)
            assert_true(result, "or", or_tests[i][0]);
        else
            assert_nil(result, "or", or_tests[i][0]);
        fclose(f);
    }

    // ---------- IF ----------
    const char *if_tests[][3] = {
        {"(if t 1 2)", "1", "true branch"},
        {"(if nil 1 2)", "2", "false branch"},
        {"(if t 1)", "1", "no else, true"},
        {"(if nil 1)", "nil", "no else, false"},
    };

    for (int i = 0; i < 4; i++) {
        FILE *f = fmemopen((void*)if_tests[i][0], strlen(if_tests[i][0]), "r");
        sExp *expr = read_sexp(f);
        sExp *result = eval(expr);

        if (strcmp(if_tests[i][1], "nil") == 0)
            assert_nil(result, if_tests[i][2], if_tests[i][0]);
        else if (strcmp(if_tests[i][1], "t") == 0)
            assert_true(result, if_tests[i][2], if_tests[i][0]);
        else
            assert_int(atol(if_tests[i][1]), result, if_tests[i][2], if_tests[i][0]);

        fclose(f);
    }

    // ---------- COND ----------
    const char *cond_tests[][3] = {
        {"(cond (nil 1) (t 2))", "2", "second true branch"},
        {"(cond (t 42))", "42", "first branch true"},
        {"(cond (nil 1) (nil 2))", "nil", "no branch true"},
        {"(cond ((eq 1 2) 5) (t 99))", "99", "eq fails, t true"},
        {"(cond)", "nil", "empty cond"},
    };

    for (int i = 0; i < 5; i++) {
        FILE *f = fmemopen((void*)cond_tests[i][0], strlen(cond_tests[i][0]), "r");
        sExp *expr = read_sexp(f);
        sExp *result = eval(expr);

        if (strcmp(cond_tests[i][1], "nil") == 0)
            assert_nil(result, cond_tests[i][2], cond_tests[i][0]);
        else
            assert_int(atol(cond_tests[i][1]), result, cond_tests[i][2], cond_tests[i][0]);

        fclose(f);
    }

    printf("\n");
}

void test_user_defined_functions() {
    // --- (define square (x) (mul x x)) ---
    const char *src1 = "(define square (x) (mul x x))";
    FILE *f1 = fmemopen((void*)src1, strlen(src1), "r");
    sExp *expr1 = read_sexp(f1);
    sExp *result1 = eval(expr1);
    fclose(f1);

    assert_symbol("square", result1, "define function square", "(define square (x) (mul x x))");

    // --- (square 5) = 25 ---
    const char *src2 = "(square 5)";
    FILE *f2 = fmemopen((void*)src2, strlen(src2), "r");
    sExp *expr2 = read_sexp(f2);
    sExp *result2 = eval(expr2);
    fclose(f2);

    assert_int(25, result2, "square function call", "(square 5)");

    // --- Nested call: (square (square 2)) = 16 ---
    const char *src3 = "(square (square 2))";
    FILE *f3 = fmemopen((void*)src3, strlen(src3), "r");
    sExp *expr3 = read_sexp(f3);
    sExp *result3 = eval(expr3);
    fclose(f3);

    assert_int(16, result3, "nested square calls", "(square (square 2))");

    // --- (define addmul (a b c) (mul (add a b) c)) ---
    const char *src4 = "(define addmul (a b c) (mul (add a b) c))";
    FILE *f4 = fmemopen((void*)src4, strlen(src4), "r");
    sExp *expr4 = read_sexp(f4);
    sExp *result4 = eval(expr4);
    fclose(f4);

    assert_symbol("addmul", result4, "define multi-arg function addmul", "(define addmul (a b c) (mul (add a b) c))");

    // --- (addmul 1 2 3) = (mul 3 3) = 9 ---
    const char *src5 = "(addmul 1 2 3)";
    FILE *f5 = fmemopen((void*)src5, strlen(src5), "r");
    sExp *expr5 = read_sexp(f5);
    sExp *result5 = eval(expr5);
    fclose(f5);

    assert_int(9, result5, "call addmul", "(addmul 1 2 3)");

    // --- Local scope test ---
    const char *src6 = "(set x 10)";
    FILE *f6 = fmemopen((void*)src6, strlen(src6), "r");
    sExp *expr6 = read_sexp(f6);
    sExp *result6 = eval(expr6);
    fclose(f6);
    assert_int(10, result6, "set x globally", "(set x 10)");

    const char *src7 = "(define usex (x) (add x 5))";
    FILE *f7 = fmemopen((void*)src7, strlen(src7), "r");
    sExp *expr7 = read_sexp(f7);
    sExp *result7 = eval(expr7);
    fclose(f7);
    assert_symbol("usex", result7, "define usex", "(define usex (x) (add x 5))");

    const char *src8 = "(usex 3)";
    FILE *f8 = fmemopen((void*)src8, strlen(src8), "r");
    sExp *expr8 = read_sexp(f8);
    sExp *result8 = eval(expr8);
    fclose(f8);
    assert_int(8, result8, "usex local scope", "(usex 3)");

    // x should still be 10 globally
    assert_int(10, eval(make_symbol("x")), "x remains unchanged after function call", "x");

    // --- Define a function that calls another function ---
    const char *src9 = "(define quad (x) (square (square x)))";
    FILE *f9 = fmemopen((void*)src9, strlen(src9), "r");
    sExp *expr9 = read_sexp(f9);
    sExp *result9 = eval(expr9);
    fclose(f9);
    assert_symbol("quad", result9, "define nested function quad", "(define quad (x) (square (square x)))");

    const char *src10 = "(quad 2)";
    FILE *f10 = fmemopen((void*)src10, strlen(src10), "r");
    sExp *expr10 = read_sexp(f10);
    sExp *result10 = eval(expr10);
    fclose(f10);
    assert_int(16, result10, "quad(2) = 16", "(quad 2)");

    // --- Recursive function (factorial) ---
    const char *src11 =
        "(define fact (n) (if (lte n 1) 1 (mul n (fact (sub n 1)))))";
    FILE *f11 = fmemopen((void*)src11, strlen(src11), "r");
    sExp *expr11 = read_sexp(f11);
    sExp *result11 = eval(expr11);
    fclose(f11);
    assert_symbol("fact", result11, "define recursive function fact", "(define fact (n) (if (lte n 1) 1 (mul n (fact (sub n 1)))))");

    const char *src12 = "(fact 5)";
    FILE *f12 = fmemopen((void*)src12, strlen(src12), "r");
    sExp *expr12 = read_sexp(f12);
    sExp *result12 = eval(expr12);
    fclose(f12);
    assert_int(120, result12, "factorial of 5", "(fact 5)");

    printf("\n");
}

void test_lambda_functions() {
    run_str_expect_int("((lambda (x) (add x 1)) 5)", 6);

    run_str_noassert("(set inc (lambda (y) (add y 1)))");
    run_str_expect_int("(inc 10)", 11);

    // 3) Lambda as an argument (via a simple higher-order 'apply1' we define)
    run_str_noassert("(define apply1 (f x) (f x))");
    run_str_expect_int("(apply1 (lambda (z) (mul z z)) 7)", 49);

    // 4) Closures: make-adder returns a lambda that captures n
    run_str_noassert("(define make-adder (n) (lambda (x) (add x n)))");
    run_str_expect_int("((make-adder 5) 3)", 8);
    run_str_noassert("(set add10 (make-adder 10))");
    run_str_expect_int("(add10 7)", 17);

    // 5) Nested closures
    run_str_noassert("(define mk (a) (lambda (b) (lambda (c) (add (add a b) c))))");
    run_str_expect_int("(((mk 1) 2) 3)", 6);

    // 6) Lambda can be returned and immediately applied
    run_str_expect_int("(( (lambda (n) (lambda (x) (add x n))) 4) 9)", 13);

    // 7) Shadowing parameter names (ensure local params override outer)
    run_str_noassert("(define shadow (x) (lambda (x) (add x 1)))");
    run_str_expect_int("((shadow 100) 5)", 6);

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
    
    printf("=== Sprint 5: Eval ===\n");
    test_eval();

    printf("=== Sprint 6: Short-Circuiting & Conditionals ===\n");
    test_short_circuit_and_conditionals();

    printf("=== Sprint 7: User-Defined Functions ===\n");
    test_user_defined_functions();
    
    printf("=== Sprint 8: Lambda Functions ===\n");
    test_lambda_functions();

    printf("\nTests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
