#pragma once

// 函数调用表达式
typedef struct CallExpr CallExpr;
struct CallExpr {
    char *fn; // 函数名
    char *arg; // 参数
};