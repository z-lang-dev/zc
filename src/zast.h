#pragma once
#include <stdio.h>
#include <stdbool.h>

typedef struct Node Node;
typedef struct CallExpr CallExpr;
typedef struct BinOp BinOp;
typedef struct Unary Unary;
typedef struct Exprs Exprs;
typedef struct Use Use;
typedef struct Asn Asn;
typedef struct IfElse IfElse;
typedef struct For For;
typedef struct Params Params;
typedef struct Fn Fn;

typedef enum {
    ND_PROG, // 一段程序
    ND_BLOCK, // 一段代码块
    ND_USE, // 导入模块
    ND_CALL, // 函数调用
    ND_FN, // 函数定义
    ND_LET, // 定量声明
    ND_MUT, // 变量声明
    ND_ASN, // 赋值
    ND_IF, // if-else语句
    ND_FOR, // for语句
    ND_INT, // 整数
    ND_BOOL, // 布尔值
    ND_NOT, // 非
    ND_NEG, // 负数
    ND_STR, // 字符串
    ND_NAME, // 名称，包括函数名、存量名、类名等
    ND_LNAME,  // 左值名称
    ND_BINOP, // 二元运算
} NodeKind;

struct CallExpr {
    Node *name; // 函数名
    int argc; // 参数个数
    Node *args[]; // 参数
};

typedef enum {
    OP_ADD, // 加法
    OP_SUB, // 减法
    OP_MUL, // 乘法
    OP_DIV, // 除法
    OP_GT, // 大于
    OP_LT, // 小于
    OP_GE, // 大于等于
    OP_LE, // 小于等于
    OP_EQ, // 等于
    OP_NE, // 不等于
    OP_AND, // 逻辑与
    OP_OR, // 逻辑或
    OP_NOT, // 逻辑非
    OP_ASN, // 赋值
    OP_ILL, // 非法操作符
} Op;

char *op_to_str(Op op);

struct BinOp {
    Node *left;
    Node *right;
    Op op;
};

struct Unary {
  Op op;
  Node *body;
};

// Exprs是一个动态的数组
struct Exprs {
    int count;
    int cap;
    Node **list;
};

Node *new_node(NodeKind kind);
Node *new_prog();
Node *new_block();
void append_expr(Node *parent, Node *expr);

struct Use {
    char *box;
    char *name;
};

struct Asn {
    Node *name; // ND_NAME
    Node *value; // EXPR
};

struct IfElse {
    Node *cond;
    Node *then;
    Node *els;
};

struct For {
    Node *cond;
    Node *body;
};

struct Params {
    int count;
    int cap;
    Node **list;
};

struct Fn {
    char *name;
    Params *params;
    Node *body;
};

// AST节点
struct Node {
    NodeKind kind;
    void* meta;
    union {
        CallExpr call;
        int num;
        bool bul;
        char *str;
        BinOp bop;
        Unary una;
        Exprs exprs;
        Use use;
        Asn asn;
        IfElse if_else;
        For loop;
        Fn fn;
    } as;
};


// 打印AST节点信息，输出到stdout
void print_node(Node *node);
// 打印AST节点信息，输出到f
void fprint_node(FILE* f, Node *node);
// 在开启LOG_TRACE开关时，打印节点信息。用于开发阶段辅助调试
void trace_node(Node *node);
// 在开启LOG_TRACE开关时，打印节点信息。用于开发阶段辅助调试
void echo_node(Node *node);