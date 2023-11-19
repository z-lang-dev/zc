#include <stdio.h>
#include "interp.h"
#include "parser.h"
#include "util.h"
#include "std.h"

// 内置函数

// print
static void print(Node *arg) {
    if (arg->kind == ND_INT) {
        printf("%d\n", arg->as.num);
    } else {
        printf("%s\n", arg->as.str);
    }
}

// pwd
static void pwd() {
    char buf[1024];
    getcwd(buf, sizeof(buf));
    printf("%s\n", buf);
}

// ls
static void ls(char *path) {
    char cmd[1024];
    sprintf(cmd, "ls %s", path);
    system(cmd);
}

// cd
static void cd(char *path) {
    if (chdir(path) != 0) {
        perror("chdir");
    }
}

// cat
static void cat(char *path) {
    read_file(path);
}


// 对表达式求值
int eval(Node *expr) {
    switch (expr->kind) {
    case ND_INT:
        return expr->as.num;
    case ND_NEG:
        return -eval(expr->as.una.body);
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

bool do_builtin(Node *expr) {
    char *fname = expr->as.call.fname->as.str;
    if (strcmp(fname, "print") == 0) {
        print(expr->as.call.args[0]);
        return true;
    } else if (strcmp(fname, "pwd") == 0) {
        pwd();
        return true;
    } else if (strcmp(fname, "ls") == 0) {
        ls(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(fname, "cd") == 0) {
        cd(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(fname, "cat") == 0) {
        cat(expr->as.call.args[0]->as.str);
        return true;
    }
    return false;
}

bool do_stdlib(Node *expr) {
    char *fname = expr->as.call.fname->as.str;
    if (strcmp(fname, "read_file") == 0) {
        read_file(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(fname, "write_file") == 0) {
        write_file(expr->as.call.args[0]->as.str, expr->as.call.args[1]->as.str);
        return true;
    }
    return false;
}

// 执行AST
void execute(Node *expr) {
    log_trace("Executing ...\n------------------\n");
    switch (expr->kind) {
    case ND_CALL:
        if (do_builtin(expr)) break;
        if (do_stdlib(expr)) break;
        printf("Unknown function: %s\n", expr->as.call.fname->as.str);
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

