#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "codegen.h"

#define MAX_INCLUDE 100
#define MAX_CONSTS 100
#define MAX_EXTERN 100

// 常量信息
typedef struct ConstData ConstData;
struct ConstData {
    int idx;
    bool has_newline;
    char *value;
};

// 元信息
struct CodegenMeta {
    // includes
    int inc_count;
    char *includes[MAX_INCLUDE];

    // data
    int data_count;
    ConstData data[MAX_CONSTS];

    // externdef
    int extern_count;
    char *externs[MAX_CONSTS];
};

static struct CodegenMeta META;

static char *WIN_REGS[4] = {"rcx", "rdx", "r8", "r9"};
static char *LINUX_REGS[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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

    if (expr->kind == ND_CALL) {
        // 处理函数名称
        CallExpr *call = &expr->as.call;
        char *name = call->name->as.str;
        bool is_print = strcmp(name, "print") == 0;
#ifdef _WIN32
        if (is_print) { // printf 要单独处理，加上'\n'
            Node *arg = call->args[0];
            ConstData *data = (ConstData*)arg->meta;
            if (arg->kind == ND_INT) {
                fprintf(fp, "    lea rcx, ct%d\n", data->idx);
                fprintf(fp, "    mov rdx, %d\n", call->args[0]->as.num);
            } else {
                fprintf(fp, "    lea rcx, ct%d\n", data->idx);
            }
            fprintf(fp, "    call printf\n");
        } else {
            for (int i = 0; i < call->argc; ++i) {
                Node *arg = call->args[i];
                ConstData *data = (ConstData*)arg->meta;
                if (arg->kind == ND_INT) {
                    fprintf(fp, "    mov %s, %d\n", WIN_REGS[i], arg->as.num);
                } else {
                    fprintf(fp, "    lea %s, ct%d\n", WIN_REGS[i], data->idx);
                }
            }
            fprintf(fp, "    call %s\n", name);
        }
#else
        if (is_print) {
            Node *arg = call->args[0];
            ConstData *data = (ConstData*)arg->meta;
            fprintf(fp, "    lea rdi, [rip+ct%d]\n", data->idx);
            if (arg->kind == ND_INT) {
                fprintf(fp, "    mov rsi, %d\n", arg->as.num);
            }
            fprintf(fp, "    call printf\n");
        } else {
            for (int i = 0; i < call->argc; ++i) {
                Node *arg = call->args[i];
                ConstData *data = (ConstData*)arg->meta;
                if (arg->kind == ND_INT) {
                    fprintf(fp, "    mov %s, %d\n", LINUX_REGS[i], arg->as.num);
                } else {
                    fprintf(fp, "    lea %s, [rip+ct%d]\n", LINUX_REGS[i], i);
                }
            }
            fprintf(fp, "    call %s\n", name);
        }
#endif
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



static void do_meta(Node *prog) {
    META.includes[META.inc_count++] = "msvcrt.lib";
    bool has_print = false;
    bool need_stdz = false;
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_CALL) {
            char *name = expr->as.call.name->as.str;
            bool is_print = strcmp(name, "print") == 0;
            if (is_print) {
                META.externs[META.extern_count++] = "printf";
                has_print = true;
            } else {
                need_stdz = true;
                META.externs[META.extern_count++] = name;
            }

            for (int j = 0; j < expr->as.call.argc; ++j) {
                Node *arg = expr->as.call.args[j];
                META.data[META.data_count].idx = META.data_count;
                if (is_print) {
                    META.data[META.data_count].has_newline = true;
                    if (arg->kind == ND_INT) {
                        META.data[META.data_count].value = "%d";
                    } else {
                        META.data[META.data_count].value = arg->as.str;
                    }
                } else {
                    if (arg->kind == ND_STR) {
                        META.data[META.data_count].value = arg->as.str;
                    }
                }
                arg->meta = &META.data[META.data_count];
                META.data_count++;
            }
        }
    }
    if (has_print) {
        META.includes[META.inc_count++] = "legacy_stdio_definitions.lib";
    }
    if (need_stdz) {
        META.includes[META.inc_count++] = "stdz.lib";
    }
}

// 根据META信息输出include语句
static void do_includes(FILE *fp, Node *prog) {
    for (int i = 0; i < META.inc_count; ++i) {
        fprintf(fp, "includelib %s\n", META.includes[i]);
    }
}

// 根据META信息输出.data字段
static void do_data(FILE *fp) {
    if (META.data_count > 0) {
        fprintf(fp, ".data\n");
    }
    for (int i = 0; i < META.data_count; ++i) {
        ConstData *data = &META.data[i];
        fprintf(fp, "    ct%d db '%s'", data->idx, data->value);
        if (data->has_newline) {
            fprintf(fp, ", 10");
        }
        fprintf(fp, ", 0\n");
    }
}

// 根据META信息输出externdef语句
static bool do_externdef(FILE *fp, Node *prog) {
    for (int i = 0; i < META.extern_count; ++i) {
        fprintf(fp, "    externdef %s:proc\n", META.externs[i]);
    }
    return META.extern_count > 0;
}

static void do_data_linux(FILE *fp) {
    for (int i = 0; i < META.data_count; ++i) {
        ConstData *data = &META.data[i];
        fprintf(fp, "ct%d:\n", data->idx);
        fprintf(fp, "    .asciz \"%s%s\"\n", data->value, data->has_newline ? "\\n" : "");
    }
}


// 将AST编译成汇编代码：linux/gas
void codegen_linux(Node *prog) {
    do_meta(prog);
    printf("start writing .s\n");
    print_node(prog);
    // 打开输出文件
    FILE *fp = fopen("app.s", "w");
    // 首行配置
    fprintf(fp, "    .intel_syntax noprefix\n");
    printf("start writing .s\n");

    fprintf(fp, "    .text\n");
    fprintf(fp, "    .global main\n");
    fprintf(fp, "main:\n");
    bool has_call = META.extern_count > 0;
    if (has_call) {
        // prolog
        fprintf(fp, "    push rbp\n");
        fprintf(fp, "    mov rbp, rsp\n");
    }

    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        gen_expr(fp, expr);
    }

    if (has_call) {
        // epilog
        fprintf(fp, "    pop rbp\n");
        // 返回
        fprintf(fp, "    xor rax, rax\n");
    }
    fprintf(fp, "    ret\n");

    do_data_linux(fp);

    // 保存并关闭文件
    fclose(fp);
}

// 将AST编译成汇编代码：windows/masm64
void codegen_win(Node *prog) {
    do_meta(prog);
    // 打开输出文件
    FILE *fp = fopen("app.asm", "w");
    // 导入标准库
    do_includes(fp, prog);

    // 数据段
    do_data(fp);

    // 代码段
    fprintf(fp, ".code\n");

    bool has_call = do_externdef(fp, prog);

    fprintf(fp, "main proc\n");

    if (has_call) {
        // prolog
        fprintf(fp, "    push rbp\n");
        fprintf(fp, "    mov rbp, rsp\n");
        // reserve stack for shadow space
        fprintf(fp, "    sub rsp, 20h\n");
    }

    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        gen_expr(fp, expr);
    }

    if (has_call) {
        // restore stack
        fprintf(fp, "    add rsp, 20h\n");
        // epilog
        fprintf(fp, "    pop rbp\n");
        // 返回
        fprintf(fp, "    xor eax, eax\n");
    }
    fprintf(fp, "    ret\n");

    // 结束
    fprintf(fp, "main endp\n");
    fprintf(fp, "end\n");
    fclose(fp);
    return;
}
