#include <stdlib.h>
#include "value.h"


Value *new_int(int num) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_INT;
    val->as.num = num;
    return val;
}

Value *new_bool(bool bul) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_BOOL;
    val->as.bul = bul;
    return val;
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
    if (val->kind == VAL_INT) {
        printf("%d\n", val->as.num);
    } else {
        printf("%s\n", val->as.bul ? "true" : "false");
    }
}