#include <stdio.h>
#include "interp.h"
#include "parser.h"
#include "util.h"
#include "stdz.h"
#include "hash.h"

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

// 用来模拟存量的查找
// 需要用一个hashtable来替代。
// static int a = 0;
static HashTable *table;
// static ValueArray *values;

static void set_val(char *name, int val) {
    hash_set_int(table, name, val);
    // array_set(values, name, val);
    // if (strcmp(name, "a") == 0) {
        // a = val;
    // }
}

static int get_val(char *name) {
    return hash_get_int(table, name);
    // return array_get(values, name);
    // if (strcmp(name, "a") == 0) {
        // return a;
    // }
    // return 0;
}

// 对表达式求值
int eval(Node *expr) {
    switch (expr->kind) {
    case ND_INT:
        return expr->as.num;
    case ND_NAME:
        return get_val(expr->as.str);
    case ND_NEG:
        return -eval(expr->as.una.body);
    case ND_LET: {
        int val = eval(expr->as.asn.value);
        char *name = expr->as.asn.name->as.str;
        set_val(name, val);
        return val;
    }
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

bool call_builtin(Node *expr) {
    char *name = expr->as.call.name->as.str;
    if (strcmp(name, "print") == 0) {
        print(expr->as.call.args[0]);
        return true;
    } else if (strcmp(name, "pwd") == 0) {
        pwd();
        return true;
    } else if (strcmp(name, "ls") == 0) {
        ls(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(name, "cd") == 0) {
        cd(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(name, "cat") == 0) {
        cat(expr->as.call.args[0]->as.str);
        return true;
    }
    return false;
}

bool call_stdlib(Node *expr) {
    char *name = expr->as.call.name->as.str;
    if (strcmp(name, "read_file") == 0) {
        read_file(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(name, "write_file") == 0) {
        write_file(expr->as.call.args[0]->as.str, expr->as.call.args[1]->as.str);
        return true;
    }
    return false;
}

// 执行AST
int execute(Node *expr) {
    switch (expr->kind) {
    case ND_PROG:
        int last = 0;
        for (int i = 0; i < expr->as.exprs.count; i++) {
            last = execute(expr->as.exprs.list[i]);
        }
        // 返回最后一个表达式的值
        return last;
    case ND_CALL:
        if (call_builtin(expr)) break;
        if (call_stdlib(expr)) break;
        printf("Unknown function: %s\n", expr->as.call.name->as.str);
        return 0; // 函数调用暂时不返回值
    default:
        int val = eval(expr);
        return val;
    }
    return 0;
}

// 解释并执行代码
void interp(char *code) {
    table = new_hash_table();
    // values = new_value_array();
    log_trace("Interpreting %s...\n", code);
    // 解析源码
    Parser *parser = new_parser(code);
    Node *prog = parse(parser);
    log_trace("Executing ...\n------------------\n");
    int ret = execute(prog);
    printf("%d\n", ret);
}

