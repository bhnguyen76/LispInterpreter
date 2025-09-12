#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    if (result->type == TYPE_DOUBLE && result->doubleVal == expected) {
        printf("[PASS] %s\n", msg);
        tests_passed++;
    } else {
        printf("[FAIL] %s (expected %f, got ", msg, expected);
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

// --- Sprint 3 Tests ---
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

int main() {
    NIL = malloc(sizeof(sExp));
    NIL->type = TYPE_NIL;

    TRUE = malloc(sizeof(sExp));
    TRUE->type = TYPE_SYMBOL;
    TRUE->strVal = strdup("t");

    printf("Running Tests...\n\n");

    test_arithmetic();
    test_relations();
    test_equality();
    test_logical();

    printf("\nTests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
