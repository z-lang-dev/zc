#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "util.h"
#include "lexer.h"

static Node *call(Lexer *lexer, char *code) {
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

static Node *integer(Lexer *lexer, char *code) {
    Node *expr = calloc(1, sizeof(Node));
    expr->kind = ND_INT;
    char *num_text = substr(code, lexer->start - code, lexer->cur - code);
    log_trace("Parsing int text: %s\n", num_text);
    expr->as.num = atoll(num_text);
    // 打印出AST
    trace_node(expr);
    return expr;
}

static Op get_op(TokenKind kind) {
    switch (kind) {
    case TK_ADD:
        return OP_ADD;
    case TK_SUB:
        return OP_SUB;
    case TK_MUL:
        return OP_MUL;
    case TK_DIV:
        return OP_DIV;
    default:
        printf("Unknown operator: %d\n", kind);
        return OP_ILL;
    }
}

static Node *binop(Lexer *lexer, char *code, Node *left) {
    Token *token = next_token(lexer);
    // 如果下一个词符是运算符，那么应当是一个二元表达式
    if (token->kind == TK_ADD || token->kind == TK_SUB || token->kind == TK_MUL || token->kind == TK_DIV) {
        Node *bop = calloc(1, sizeof(Node));
        bop->kind = ND_BINOP;
        bop->as.bop.op = get_op(token->kind);
        bop->as.bop.left = left;
        token = next_token(lexer);
        bop->as.bop.right = integer(lexer, code);
        // 打印出AST
        trace_node(bop);
        return binop(lexer, code, bop);
    } else {
        // 否则就直接返回左子节点。
        return left;
    }
}

// 解析表达式
Node *parse(Parser *parser) {
    char *code = parser->code;
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
        Node * num = integer(lexer, code);
        return binop(lexer, code, num);
    } else {
        // 否则就是一个函数调用
        return call(lexer, code);
    }
}

Parser *new_parser(char *code) {
    Parser *parser = calloc(1, sizeof(Parser));
    parser->lexer = new_lexer(code);
    parser->code = code;
    parser->cur = next_token(parser->lexer);
    parser->next = next_token(parser->lexer);
    return parser;
}