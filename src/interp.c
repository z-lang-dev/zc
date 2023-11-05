#include <stdio.h>
#include "interp.h"
#include "parser.h"
#include "util.h"

// 执行AST
void execute(CallExpr *call) {
    log_trace("Executing %s(%s)...\n", call->fn, call->arg);
    // 打印call.arg
    printf("%s\n", call->arg);
}

// 解释并执行代码
void interp(char *code) {
    log_trace("Interpreting %s...\n", code);
    // 解析源码
    CallExpr *call = parse_expr(code);
    execute(call);
}

