#include <stdio.h>
#include "interp.h"
#include "parser.h"
#include "util.h"

// 对表达式求值
int eval(Node *expr) {
    switch (expr->kind) {
    case ND_INT:
        return expr->as.num;
    case ND_BINOP:
        BinOp *bop = &expr->as.bop;
        int res = 0;
        switch (bop->op) {
        case OP_ADD:
            res = eval(bop->left) + eval(bop->right);
            break;
        case OP_SUB:
            res = eval(bop->left) - eval(bop->right);
            break;
        case OP_MUL:
            res = eval(bop->left) * eval(bop->right);
            break;
        case OP_DIV:
            res = eval(bop->left) / eval(bop->right);
            break;
        default:
            printf("Unknown operator: %d\n", bop->op);
        }
        return res;
    default:
        printf("Wrong NodeKind to eval: %d\n", expr->kind);
        return 0;
    }
}

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
    default:
        int val = eval(expr);
        printf("%d\n", val);
        break;
    }
    
}

// 解释并执行代码
void interp(char *code) {
    log_trace("Interpreting %s...\n", code);
    // 解析源码
    Parser *parser = new_parser(code);
    Node *expr = parse(parser);
    execute(expr);
}

