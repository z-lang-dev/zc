#include <stdio.h>
#include <inttypes.h>
#include "codegen.h"

// 将AST编译成汇编代码：linux/gas
void codegen_linux(Node *expr) {
    CallExpr *call = &expr->as.call;
    // 打开输出文件
    FILE *fp = fopen("app.s", "w");
    // 首行配置
    fprintf(fp, "    .intel_syntax noprefix\n");
    fprintf(fp, "    .text\n");
    fprintf(fp, "    .global main\n");
    fprintf(fp, "main:\n");
    // prolog
    fprintf(fp, "    push rbp\n");
    fprintf(fp, "    mov rbp, rsp\n");
    // 调用printf函数。
    fprintf(fp, "    lea rdi, [rip+fmt]\n");
    Node *arg = call->arg;
    if (arg->kind == ND_INT) {
        fprintf(fp, "    mov rsi, %" PRId64 "\n", arg->as.num);
    }
    fprintf(fp, "    call printf\n");
    // epilog
    fprintf(fp, "    pop rbp\n");
    // 返回
    fprintf(fp, "    xor rax, rax\n");
    fprintf(fp, "    ret\n");
    // 设置参数字符串
    fprintf(fp, "fmt:\n");
    if (arg->kind == ND_INT) {
        fprintf(fp, "    .asciz \"%%lld\\n\"\n");
    } else {
        fprintf(fp, "    .asciz \"%s\"\n", arg->as.str);
    }

    // 保存并关闭文件
    fclose(fp);
}

// 将AST编译成汇编代码：windows/masm64
void codegen_win(Node *expr) {
    CallExpr *call = &expr->as.call;
    // 打开输出文件
    FILE *fp = fopen("app.asm", "w");
    // 导入标准库
    fprintf(fp, "includelib msvcrt.lib\n");
    fprintf(fp, "includelib legacy_stdio_definitions.lib\n");
    // 要打印的信息参数
    fprintf(fp, ".data\n");
    Node *arg = call->arg;
    if (arg->kind == ND_INT) {
        fprintf(fp, "    fmt db '%%lld', 10, 0\n");
    } else {
        fprintf(fp, "    fmt db '%s', 10, 0\n", arg->as.str);
    } 
    fprintf(fp, ".code\n");
    // 声明printf函数
    fprintf(fp, "    externdef printf:proc\n");

    // main函数

    fprintf(fp, "main proc\n");
    // prolog
    fprintf(fp, "    push rbp\n");
    fprintf(fp, "    mov rbp, rsp\n");
    // reserve stack for shadow space
    fprintf(fp, "    sub rsp, 20h\n");

    // 准备printf参数
    fprintf(fp, "    lea rcx, fmt\n");
    if (arg->kind == ND_INT) {
        fprintf(fp, "    mov rdx, %" PRId64 "\n", arg->as.num);
    }
    fprintf(fp, "    call printf\n");

    // restore stack
    fprintf(fp, "    add rsp, 20h\n");
    // epilog
    fprintf(fp, "    pop rbp\n");
    // 返回
    fprintf(fp, "    xor eax, eax\n");
    fprintf(fp, "    ret\n");

    // 结束
    fprintf(fp, "main endp\n");
    fprintf(fp, "end\n");

    // 保存并关闭文件
    fclose(fp);
}

