#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "util.h"
#include "lexer.h"

static Node *parse_call(Lexer *lexer, char *code) {
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

static Node *parse_int(Lexer *lexer, char *code) {
    Node *expr = calloc(1, sizeof(Node));
    expr->kind = ND_INT;
    char *num_text = substr(code, lexer->start - code, lexer->cur - code);
    log_trace("Parsing int text: %s\n", num_text);
    expr->as.num = atoll(num_text);
    // 打印出AST
    trace_node(expr);
    return expr;
}

// 解析表达式
Node *parse_expr(char *code) {
    log_trace("Parsing %s...\n", code);
    // 解析源码
    size_t len = strlen(code);
    if (len == 0) {
        return NULL;
    }

    // 先新建一个词法分析器
    Lexer *lexer = new_lexer(code);
    // 获得第一个词符
    Token *token = next_token(lexer);

    if (token->kind == TK_INT) {
        // 如果是整数
        Node * num = parse_int(lexer, code);
        // 如果下一个词符是'+'，那么应当是一个加法表达式
        token = next_token(lexer);
        if (token->kind == TK_ADD) {
            Node *add = calloc(1, sizeof(Node));
            add->kind = ND_ADD;
            add->as.bop.op = OP_ADD;
            add->as.bop.left = num;
            token = next_token(lexer);
            add->as.bop.right = parse_int(lexer, code);
            // 打印出AST
            trace_node(add);
            return add;
        } else {
            // 否则就是一个整数
            return num;
        }
    } else {
        // 否则就是一个函数调用
        return parse_call(lexer, code);
    }
}
