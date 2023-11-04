#include <stdio.h>
#include "codegen.h"

// 将AST编译成汇编代码：linux/gas
void codegen_linux(CallExpr *expr) {
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
    // 调用puts函数。注：这里还没有做好内置函数print和C标准库函数puts的映射，所以先直接用puts。未来会在内置函数功能中添加这种映射。
    fprintf(fp, "    lea rdi, [rip+msg]\n");
    fprintf(fp, "    call %s\n", "puts");
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

// 将AST编译成汇编代码：windows/masm64
void codegen_win(CallExpr *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.asm", "w");
    // 导入标准库
    fprintf(fp, "includelib msvcrt.lib\n");
    fprintf(fp, "includelib legacy_stdio_definitions.lib\n");
    // 要打印的信息参数
    fprintf(fp, ".data\n");
    fprintf(fp, "    msg db '%s', 10, 0\n", expr->arg);
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
    fprintf(fp, "    lea rcx, msg\n");
    fprintf(fp, "    call printf\n");

    // restore stack
    fprintf(fp, "    add rsp, 20h\n");
    // epilog
    fprintf(fp, "    pop rbp\n");
    // 返回
    fprintf(fp, "    mov rcx, 0\n");
    fprintf(fp, "    ret\n");

    // 结束
    fprintf(fp, "main endp\n");
    fprintf(fp, "end\n");

    // 保存并关闭文件
    fclose(fp);
}

