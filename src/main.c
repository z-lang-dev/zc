#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zast.h"

static void help(void) {
  printf("【用法】：./z <源码>|build <hello.z>|run <hello.z>\n");
}

// 打印AST
static void print_ast(CallExpr *expr) {
    printf("CallExpr {\n");
    printf("  fn: %s\n", expr->fn);
    printf("  arg: \"%s\"\n", expr->arg);
    printf("}\n");
}

// 获取字符c在字符串中的位置
int index_of(char *str, char c) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == c) {
            return i;
        }
        i++;
    }
    return -1;
}

// 提取一个子串
char *substr(char *str, int start, int end) {
    int len = end - start;
    char *result = calloc(len + 1, sizeof(char));
    int i = 0;
    while (i < len) {
        result[i] = str[start + i];
        i++;
    }
    result[i] = '\0';
    return result;
}

// 解析表达式
static CallExpr *parse_expr(char *code) {
    printf("Parsing %s...\n", code);
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
    print_ast(expr);
    return expr;
}

// 执行AST
static void execute(CallExpr *call) {
    printf("Executing %s(%s)...\n", call->fn, call->arg);
    // 打印call.arg
    printf("%s\n", call->arg);
}

// 解释并执行代码
static void interp(char *code) {
    printf("Interpreting %s...\n", code);
    // 解析源码
    CallExpr *call = parse_expr(code);
    execute(call);
}

static void repl(void) {
    printf("TODO: repl\n");
}

static char *read_src(char *file) {
    // 打开文件
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        printf("似乎无法打开文件：%s\n", file);
        exit(1);
    }
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // 读取文件内容
    char *code = calloc(size + 1, sizeof(char));
    fread(code, sizeof(char), size, fp);
    // 关闭文件
    fclose(fp);
    return code;
}

// 将AST编译成汇编代码
static void codegen(CallExpr *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.s", "w");
    // 首行配置
    fprintf(fp, "    .intel_syntax noprefix\n");
    fprintf(fp, "    .text\n");
    fprintf(fp, ".global main\n");
    fprintf(fp, "main:\n");
    // prolog
    fprintf(fp, "    push rbp\n");
    fprintf(fp, "    mov rbp, rsp\n");
    // 调用expr->fn函数
    fprintf(fp, "    lea rdi, [rip+msg]\n");
    fprintf(fp, "    call %s\n", expr->fn);
    // epilog
    fprintf(fp, "    pop rbp\n");
    // 返回
    fprintf(fp, "    xor rax, rax\n");
    fprintf(fp, "    ret\n");
    // 设置参数字符串
    fprintf(fp, "msg:\n");
    fprintf(fp, "    .asciz \"%s\"\n", expr->arg);

    // 保存并关闭文件
    fclose(fp);
}


static void build(char *file) {
    printf("Building %s\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    CallExpr *expr = parse_expr(code);
    // 输出汇编代码
    codegen(expr);
    // 调用clang生成可执行文件
    system("clang -o app.exe *.s");
}

static void run(char *file) {
    printf("TODO: run %s\n", file);
}

int main(int argc, char** argv) {
    if (argc == 1) {
        repl();
        return 0;
    }
    printf("Hello from Z!\n");

    // 第一个参数是命令名称：interp|repl|build|run
    char *cmd = argv[1];

    // 如果命令是repl，直接进入repl()交互环境
    if (strcmp(cmd, "repl") == 0) {
        repl();
        return 0;
    }

    // 剩下的命令都需要提供内容（代码或文件名称）
    if (argc < 3) {
        help();
        return 1;
    }

    // 根据命令执行不同的操作
    if (strcmp(cmd, "interp") == 0) {
        interp(argv[2]);
    } else if (strcmp(cmd, "build") == 0) {
        build(argv[2]);
    } else if (strcmp(cmd, "run") == 0) {
        run(argv[2]);
    } else {
        help();
    }

    return 0;
}
