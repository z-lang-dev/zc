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
            printf("%lld\n", arg->as.num);
        } else {
            printf("%s\n", arg->as.str);
        }
        break;
    case ND_INT:
        printf("%lld\n", expr->as.num);
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

