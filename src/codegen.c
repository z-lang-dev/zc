#include <stdio.h>
#include <inttypes.h>
#include "codegen.h"

static void gen_expr(FILE *fp, Node *expr) {
    if (expr->kind == ND_INT) {
        fprintf(fp, "    mov rax, %d\n", expr->as.num);
        return;
    }

    if (expr->kind == ND_NEG) {
        gen_expr(fp, expr->as.una.body);
        fprintf(fp, "    neg rax\n");
        return;
    }

    // 错误情况：
    if (expr->kind != ND_BINOP) {
        printf("Error: unknown node kind for gen_expr: %d\n", expr->kind);
        return;
    }

    // 处理二元表达式

    // 左膀，gen_expr_win完成之后，结果存在rax中
    gen_expr(fp, expr->as.bop.left);
    // 右臂，现在的语境下，右臂只可能是整数，因此可以当做立即数直接参与计算
    if (expr->as.bop.right->kind == ND_INT) {
        int right = expr->as.bop.right->as.num;
        // 具体的二元运算
        switch (expr->as.bop.op) {
        case OP_ADD:
            fprintf(fp, "    add rax, %d\n", right);
            break;
        case OP_SUB:
            fprintf(fp, "    sub rax, %d\n", right);
            break;
        case OP_MUL:
            fprintf(fp, "    imul rax, %d\n", right);
            break;
        case OP_DIV:
            fprintf(fp, "    mov rdi, %d\n", right);
            fprintf(fp, "    cqo\n");
            fprintf(fp, "    idiv rdi\n");
            break;
        default:
            printf("Error: unknown operator for binop expr: %d\n", expr->as.bop.op);
        }
    } else {
        fprintf(fp, "    push rax\n");
        // 右臂
        gen_expr(fp, expr->as.bop.right);
        fprintf(fp, "    push rax\n");
        // 取出结果，进行运算操作
        fprintf(fp, "    pop rdi\n");
        fprintf(fp, "    pop rax\n");

        // 具体的二元运算
        switch (expr->as.bop.op) {
        case OP_ADD:
            fprintf(fp, "    add rax, rdi\n");
            break;
        case OP_SUB:
            fprintf(fp, "    sub rax, rdi\n");
            break;
        case OP_MUL:
            fprintf(fp, "    imul rax, rdi\n");
            break;
        case OP_DIV:
            fprintf(fp, "    cqo\n");
            fprintf(fp, "    idiv rdi\n");
            break;
        default:
            printf("Error: unknown operator for binop expr: %d\n", expr->as.bop.op);
        }
    }
    return;
}


char *LINUX_REGS[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// 将AST编译成汇编代码：linux/gas
void codegen_linux(Node *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.s", "w");
    // 首行配置
    fprintf(fp, "    .intel_syntax noprefix\n");

    fprintf(fp, "    .text\n");
    fprintf(fp, "    .global main\n");
    fprintf(fp, "main:\n");

    if (expr->kind != ND_CALL) {
        gen_expr(fp, expr);
        fprintf(fp, "    ret\n");
        fclose(fp);
        return;
    }

    CallExpr *call = &expr->as.call;
    // prolog
    fprintf(fp, "    push rbp\n");
    fprintf(fp, "    mov rbp, rsp\n");
    // 调用printf函数。
    fprintf(fp, "    lea rdi, [rip+fmt]\n");
    Node *arg = call->args[0];
    if (arg->kind == ND_INT) {
        fprintf(fp, "    mov rsi, %d\n", arg->as.num);
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
        fprintf(fp, "    .asciz \"%%d\\n\"\n");
    } else {
        fprintf(fp, "    .asciz \"%s\"\n", arg->as.str);
    }

    // 保存并关闭文件
    fclose(fp);
}

char *WIN_REGS[4] = {"rcx", "rdx", "r8", "r9"};

// 将AST编译成汇编代码：windows/masm64
void codegen_win(Node *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.asm", "w");
    // 导入标准库
    fprintf(fp, "includelib msvcrt.lib\n");

    // 如果是整数
    if (expr->kind == ND_INT) {
        fprintf(fp, ".code\n");
        fprintf(fp, "main proc\n");
        fprintf(fp, "    mov rax, %d\n", expr->as.num);
        fprintf(fp, "    ret\n");

        // 结束
        fprintf(fp, "main endp\n");
        fprintf(fp, "end\n");

        fclose(fp);
        return;
    } else if (expr->kind == ND_BINOP || expr->kind == ND_NEG) {
        fprintf(fp, ".code\n");
        fprintf(fp, "main proc\n");
        gen_expr(fp, expr);
        // fprintf(fp, "    mov rax, %d\n", expr->as.bop.left->as.num);
        // fprintf(fp, "    add rax, %d\n", expr->as.bop.right->as.num);
        fprintf(fp, "    ret\n");

        fprintf(fp, "main endp\n");
        fprintf(fp, "end\n");

        fclose(fp);
        return;
    }

    CallExpr *call = &expr->as.call;
    fprintf(fp, "includelib legacy_stdio_definitions.lib\n");
    fprintf(fp, "includelib std.lib\n");
    fprintf(fp, ".data\n");
    // 现在支持的参数都是常亮，因此直接写到.data字段就行了
    char *fname = call->fname->as.str;
    printf("fname: %s\n", fname);
    for (int i = 0; i < call->argc; ++i) {
        Node *arg = call->args[i];
        if (strcmp(fname, "print") == 0) { // printf 要单独处理，加上'\n'
            if (arg->kind == ND_INT) {
                fprintf(fp, "    fmt db '%%d', 10, 0\n");
            } else {
                fprintf(fp, "    fmt db '%s', 10, 0\n", arg->as.str);
            }
        } else { // 普通函数
            if (arg->kind == ND_INT) {
                fprintf(fp, "    arg%d db '%%d', 0\n", i);
            } else {
                fprintf(fp, "    arg%d db '%s', 0\n", i, arg->as.str);
            }
        }
    }
     
    fprintf(fp, ".code\n");
    // 声明函数
    fprintf(fp, "    externdef %s:proc\n", fname);

    // main函数
    fprintf(fp, "main proc\n");
    // prolog
    fprintf(fp, "    push rbp\n");
    fprintf(fp, "    mov rbp, rsp\n");
    // reserve stack for shadow space
    fprintf(fp, "    sub rsp, 20h\n");

    // 准备参数

    if (strcmp(fname, "print") == 0) { // printf 要单独处理，加上'\n'
        fprintf(fp, "    lea rcx, fmt\n");
        fprintf(fp, "    mov rdx, %d\n", call->args[0]->as.num);
        fprintf(fp, "    call printf\n");
    } else {
        for (int i = 0; i < call->argc; ++i) {
            Node *arg = call->args[i];
            if (arg->kind == ND_INT) {
                fprintf(fp, "    mov %s, %d\n", WIN_REGS[i], arg->as.num);
            } else {
                fprintf(fp, "    lea %s, arg%d\n", WIN_REGS[i], i);
            }
        }
        fprintf(fp, "    call %s\n", fname);
    }

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

