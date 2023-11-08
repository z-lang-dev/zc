#include <stdio.h>
#include "interp.h"
#include "parser.h"
#include "util.h"

// 执行AST
void execute(Node *expr) {
    log_trace("Executing ...\n------------------\n");
    switch (expr->kind) {
    case ND_CALL:
        // 打印call.arg
        Node *arg = expr->as.call.arg;
        if (arg->kind == ND_INT) {
            printf("%d\n", arg->as.num);
        } else {
            printf("%s\n", arg->as.str);
        }
        break;
    case ND_INT:
        printf("%d\n", expr->as.num);
        break;
    case ND_ADD:
        BinOp *bop = &expr->as.bop;
        printf("%d\n", bop->left->as.num + bop->right->as.num);
        break;
    }
    
}

// 解释并执行代码
void interp(char *code) {
    log_trace("Interpreting %s...\n", code);
    // 解析源码
    Node *expr = parse_expr(code);
    execute(expr);
}

