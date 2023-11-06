#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "util.h"

// 解析表达式
Node *parse_expr(char *code) {
    log_trace("Parsing %s...\n", code);
    // 解析源码
    Node *expr = calloc(1, sizeof(Node));
    expr->kind = ND_CALL;
    CallExpr *call = &expr->as.call;
    // 从代码开头到'('之间的就是函数名称
    int index_lparen = index_of(code, '(');
    char *name = substr(code, 0, index_lparen);
    Node *fname = calloc(1, sizeof(Node));
    fname->kind = ND_FNAME;
    fname->as.str = name;
    call->fname = fname;
    // 读取'('之后的第一个字符
    char c = code[index_lparen + 1];
    Node *arg = calloc(1, sizeof(Node));
    if (c == '"') {
        // 如果是'"'，则是字符串参数
        arg->kind = ND_STR;
        arg->as.str = substr(code, index_lparen + 2, strlen(code)-2);
    } else {
        // 否则是整数参数
        arg->kind = ND_INT;
        arg->as.num = atoll(substr(code, index_lparen + 1, strlen(code)-1));
    }
    call->arg = arg;
    // 打印出AST
    trace_node(expr);
    return expr;
}