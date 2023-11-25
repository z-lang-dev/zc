#pragma once
#include <stdbool.h>

/**
 * @brief 存值的种类
 */
typedef enum {
    TP_INT, /**< 整数值 */
    TP_BOOL /**< 布尔值 */
} ValueKind;

/**
 * @brief 存值
 */
typedef struct Value Value;
struct Value {
    ValueKind kind; /**< 存值的种类 */
    union {
        int num; /**< 整数值 */
        bool bul; /**< 布尔值 */
    } as;
};