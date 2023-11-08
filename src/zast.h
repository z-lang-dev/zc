#pragma once
#include <stdio.h>


typedef struct Node Node;
typedef struct Value Value;
typedef struct CallExpr CallExpr;
typedef struct BinOp BinOp;

typedef enum {
    ND_CALL, // 函数调用
    ND_INT, // 整数
    ND_STR, // 字符串
    ND_FNAME, // 函数名称
    ND_BINOP, // 二元运算
} NodeKind;

struct CallExpr {
    Node *fname; // 函数名
    Node *arg; // 参数
};

typedef enum {
    OP_ADD, // 加法
    OP_SUB, // 减法
    OP_MUL, // 乘法
    OP_DIV, // 除法
    OP_ILL, // 非法操作符
} Op;

struct BinOp {
    Node *left;
    Node *right;
    Op op;
};

struct Node {
    NodeKind kind;
    union {
        CallExpr call;
        int num;
        char *str;
        BinOp bop;
    } as;
};

// 函数调用表达式


// 打印AST节点信息，输出到stdout
void print_node(Node *node);
// 打印AST节点信息，输出到f
void fprint_node(FILE* f, Node *node);
// 在开启LOG_TRACE开关时，打印节点信息。用于开发阶段辅助调试
void trace_node(Node *node);