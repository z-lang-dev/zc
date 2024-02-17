#pragma once
#include <stdbool.h>
#include "zast.h"

/**
 * @brief 存值的种类
 */
typedef enum {
    VAL_INT, /**< 整数值 */
    VAL_FLOAT, /**< 32位浮点数 */
    VAL_DOUBLE, /**< 64位浮点数 */
    VAL_BOOL, /**< 布尔值 */
    VAL_FN, /**< 函数定义 */
    VAL_NIL /**< 空值 */
} ValueKind;

/**
 * @brief 存值
 */
typedef struct Value Value;
struct Value {
    ValueKind kind; /**< 存值的种类 */
    union {
        int num; /**< 整数值 */
        float float_num; /**< 32位浮点数 */
        float double_num; /**< 64位浮点数 */
        bool bul; /**< 布尔值 */
        Fn *fn; /**< 函数定义 */
    } as;
};

Value *new_int(int num);
Value *new_float(float num);
Value *new_double(double num);
Value *new_bool(bool bul);
Value *new_nil();
Value *new_fn(Fn *fn);

Value *neg_val(Value *val);
Value *add_val(Value *a, Value *b);
Value *mul_val(Value *a, Value *b);
Value *div_val(Value *a, Value *b);

Value *not(Value *val);

void print_val(Value *val);