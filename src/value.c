#include <stdlib.h>
#include "value.h"

Value *new_int(int num) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_INT;
    val->as.num = num;
    return val;
}

Value *new_str(char *str) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_STR;
    val->as.str = str;
    return val;
}

Value *new_float(float num) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_FLOAT;
    val->as.float_num = num;
    return val;
}

Value *new_double(double num) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_DOUBLE;
    val->as.double_num = num;
    return val;
}

Value *new_array_val(int count) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_ARRAY;
    val->as.array = calloc(count, sizeof(ValArray));
    val->as.array->cap = count > 4 ? count : 4;
    val->as.array->size = count;
    val->as.array->items = calloc(val->as.array->cap, sizeof(Value *));
    return val;
}

Value *new_dict_val(HashTable *entries) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_DICT;
    val->as.dict = calloc(1, sizeof(ValDict));
    val->as.dict->entries = entries;
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

Value *new_fn(Fn *fn) {
    Value *val = calloc(1, sizeof(Value));
    val->kind = VAL_FN;
    val->as.fn = fn;
    return val;
}

Value *not(Value *val) {
    return new_bool(!val->as.bul);
}

Value *neg_val(Value *val) {
    switch (val->kind) {
    case VAL_INT:
        return new_int(-val->as.num);
    case VAL_FLOAT:
        return new_float(-val->as.float_num);
    case VAL_DOUBLE:
        return new_double(-val->as.double_num);
    case VAL_BOOL:
        return new_bool(!val->as.bul);
    default:
        return new_nil();
    }
}

Value *add_val(Value *a, Value *b) {
    if (a->kind != b->kind) {
        return new_nil();
    }
    switch (a->kind) {
    case VAL_INT:
        return new_int(a->as.num + b->as.num);
    case VAL_FLOAT:
        return new_float(a->as.float_num + b->as.float_num);
    case VAL_DOUBLE:
        return new_double(a->as.double_num + b->as.double_num);
    case VAL_BOOL:
        return new_bool(a->as.bul || b->as.bul);
    default:
        return new_nil();
    }
}

Value *mul_val(Value *a, Value *b) {
    if (a->kind != b->kind) {
        return new_nil();
    }
    switch (a->kind) {
    case VAL_INT:
        return new_int(a->as.num * b->as.num);
    case VAL_FLOAT:
        return new_float(a->as.float_num * b->as.float_num);
    case VAL_DOUBLE:
        return new_double(a->as.double_num * b->as.double_num);
    case VAL_BOOL:
        return new_bool(a->as.bul && b->as.bul);
    default:
        return new_nil();
    }
}

Value *div_val(Value *a, Value *b) {
    if (a->kind != b->kind) {
        return new_nil();
    }
    switch (a->kind) {
    case VAL_INT:
        return new_int(a->as.num / b->as.num);
    case VAL_FLOAT:
        return new_float(a->as.float_num / b->as.float_num);
    case VAL_DOUBLE:
        return new_double(a->as.double_num / b->as.double_num);
    case VAL_BOOL:
        return new_bool(a->as.bul && !b->as.bul);
    default:
        return new_nil();
    }
}

void print_val(Value *val) {
    if (val == NULL) {
        return;
    }
    switch (val->kind) {
    case VAL_INT:
        printf("%d", val->as.num);
        break;
    case VAL_FLOAT:
        printf("%f", val->as.float_num);
        break;
    case VAL_DOUBLE:
        printf("%lf", val->as.double_num);
        break;
    case VAL_BOOL:
        printf("%s", val->as.bul ? "true" : "false");
        break;
    case VAL_NIL:
        printf("nil");
        break;
    case VAL_FN:
        printf("fn %s", val->as.fn->name);
        break;
    case VAL_STR:
        printf("%s", val->as.str);
        break;
    case VAL_ARRAY: {
        printf("[");
        for (int i = 0; i < val->as.array->size; i++) {
            print_val(val->as.array->items[i]);
            if (i < val->as.array->size - 1) {
                printf(", ");
            }
        }
        printf("]");
        break;
    }
    case VAL_DICT:
        printf("{");
        HashIter *i = hash_iter(val->as.dict->entries);
        bool is_tail = false;
        while (hash_next(val->as.dict->entries, i)) {
            if (is_tail) printf(", ");
            is_tail = true;
            printf(i->key);
            printf(": ");
            print_val((Value*)(i->value));
        }
        printf("}");
        break;
    default:
        printf("Unknown value kind: %d\n", val->kind);
    }
}