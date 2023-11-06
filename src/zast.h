#pragma once
#include <stdint.h>


typedef struct Node Node;
typedef struct Value Value;
typedef struct CallExpr CallExpr;

typedef enum {
    ND_CALL,
    ND_INT,
    ND_STR,
    ND_FNAME, // 函数名称
} NodeKind;

struct CallExpr {
    Node *fname; // 函数名
    Node *arg; // 参数
};

struct Node {
    NodeKind kind;
    union {
        CallExpr call;
        int64_t num;
        char *str;
    } as;
};

// 函数调用表达式


// 打印AST节点信息，输出到stdout
void print_node(Node *node);
// 打印AST节点信息，输出到f
void fprint_node(FILE* f, Node *node);
// 在开启LOG_TRACE开关时，打印节点信息。用于开发阶段辅助调试
void trace_node(Node *node);