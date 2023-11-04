#include <stdio.h>
#include "interp.h"
#include "parser.h"

// 执行AST
void execute(CallExpr *call) {
    printf("Executing %s(%s)...\n", call->fn, call->arg);
    // 打印call.arg
    printf("%s\n", call->arg);
}

// 解释并执行代码
void interp(char *code) {
    printf("Interpreting %s...\n", code);
    // 解析源码
    CallExpr *call = parse_expr(code);
    execute(call);
}

