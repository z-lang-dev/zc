#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "util.h"

// 打印AST
static void print_ast(CallExpr *expr) {
    printf("CallExpr {\n");
    printf("  fn: %s\n", expr->fn);
    printf("  arg: \"%s\"\n", expr->arg);
    printf("}\n");
}

// 解析表达式
CallExpr *parse_expr(char *code) {
    log_trace("Parsing %s...\n", code);
    // 解析源码
    CallExpr *expr = calloc(1, sizeof(CallExpr));
    // 从代码开头到'('之间的就是函数名称
    int index_lparen = index_of(code, '(');
    char *fn = substr(code, 0, index_lparen);
    expr->fn = fn;
    // 从'"'到'"'之间的就是参数
    char *arg = substr(code, index_lparen + 2, strlen(code)-2);
    expr->arg = arg;
    // 打印出AST
#ifdef LOG_TRACE
    print_ast(expr);
#endif
    return expr;
}