#include <stdlib.h>
#include "value.h"


Value *new_int(int num) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_INT;
    val->as.num = num;
    return val;
}

const Value TRUE_VAL = {VAL_BOOL, {true}};
const Value FALSE_VAL = {VAL_BOOL, {false}};
const Value NIL_VAL = {VAL_NIL, {0}};

Value *new_bool(bool bul) {
    return bul ? &TRUE_VAL : &FALSE_VAL;
}

Value *new_nil() {
    return &NIL_VAL;
}

Value *not(Value *val) {
    return new_bool(!val->as.bul);
}

Value *neg_val(Value *val) {
    if (val->kind == VAL_INT) {
        return new_int(-val->as.num);
    } else {
        return new_bool(!val->as.bul);
    }
}

Value *add_val(Value *a, Value *b) {
    if (a->kind == VAL_INT && b->kind == VAL_INT) {
        return new_int(a->as.num + b->as.num);
    } else {
        return new_bool(a->as.bul || b->as.bul);
    }
}

Value *mul_val(Value *a, Value *b) {
    if (a->kind == VAL_INT && b->kind == VAL_INT) {
        return new_int(a->as.num * b->as.num);
    } else {
        return new_bool(a->as.bul && b->as.bul);
    }
}

Value *div_val(Value *a, Value *b) {
    if (a->kind == VAL_INT && b->kind == VAL_INT) {
        return new_int(a->as.num / b->as.num);
    } else {
        return new_bool(a->as.bul && !b->as.bul);
    }
}

void print_val(Value *val) {
    if (val == NULL) {
        return;
    }
    switch (val->kind) {
    case VAL_INT:
        printf("%d\n", val->as.num);
        break;
    case VAL_BOOL:
        printf("%s\n", val->as.bul ? "true" : "false");
        break;
    case VAL_NIL:
        printf("nil\n");
        break;
    default:
        printf("Unknown value kind: %d\n", val->kind);
    }
}