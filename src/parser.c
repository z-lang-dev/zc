#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "util.h"
#include "lexer.h"

static void advance(Parser *parser) {
    parser->cur = parser->next;
    parser->next = next_token(parser->lexer);
}

static Node *call(Parser *parser) {
    char *code = parser->code;
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

static char *extract_token_text(Parser *parser) {
    char *code = parser->code;
    int len = parser->cur->len;
    char *result = calloc(len + 1, sizeof(char));
    int i = 0;
    while (i < len) {
        result[i] = parser->cur->pos[i];
        i++;
    }
    result[i] = '\0';
    return result;
}

static Node *integer(Parser *parser) {
    Node *expr = calloc(1, sizeof(Node));
    expr->kind = ND_INT;
    char *num_text = extract_token_text(parser);
    log_trace("Parsing int text: %s\n", num_text);
    expr->as.num = atoll(num_text);
    // 打印出AST
    trace_node(expr);
    advance(parser);
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

static Precedence get_prec(TokenKind kind) {
    switch (kind) {
    case TK_ADD:
    case TK_SUB:
        return PREC_ADDSUB;
    case TK_MUL:
    case TK_DIV:
        return PREC_MULDIV;
    case TK_EOF:
        return PREC_NONE;
    default:
        printf("Unknown operator: %d\n", kind);
        return PREC_NONE;
    }
}

static bool is_higher_prec(Parser *parser, Precedence prec) {
    return get_prec(parser->cur->kind) > prec;
}

static Node *binop(Parser *parser, Node *left, Precedence base_prec) {
    Token *cur = parser->cur;
    // 如果下一个词符是运算符，那么应当是一个二元表达式
    if (cur->kind == TK_ADD || cur->kind == TK_SUB || cur->kind == TK_MUL || cur->kind == TK_DIV) {
        Precedence cur_prec = get_prec(cur->kind);
        Node *bop = calloc(1, sizeof(Node));
        bop->kind = ND_BINOP;
        bop->as.bop.op = get_op(cur->kind);
        bop->as.bop.left = left;
        advance(parser);
        Node *right = integer(parser);
        // peek
        if (is_higher_prec(parser, cur_prec)) {
            // 如果下一个运算符的优先级更高，那么就递归调用binop
            Node *right_bop = binop(parser, right, get_prec(parser->cur->kind));
            bop->as.bop.right = right_bop;
            Node *res = binop(parser, bop, base_prec);
            echo_node(res);
            return res;
        } else if (is_higher_prec(parser, base_prec)) {
            bop->as.bop.right = right;
            // 打印出AST
            Node *res = binop(parser, bop, get_prec(parser->cur->kind));
            // echo_node(bop);
            return res;
        } else {
            // 不再递归
            bop->as.bop.right = right;
            return bop;
        }
    } else {
        // 否则就直接返回左子节点。
        echo_node(left);
        return left;
    }
}

// 解析一个表达式
static void expression(Parser *parser) {
    // 表达式可以是一个整数、一系列运算，或者一个调用
    if (parser->cur->kind == TK_INT) {
        // 如果是整数
        Node *num = integer(parser);
        return binop(parser, num, PREC_NONE);
    } else {
        // 否则就是一个函数调用
        return call(parser);
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

    expression(parser);
}

Parser *new_parser(char *code) {
    Parser *parser = calloc(1, sizeof(Parser));
    parser->lexer = new_lexer(code);
    parser->code = code;
    parser->cur = next_token(parser->lexer);
    parser->next = next_token(parser->lexer);
    return parser;
}