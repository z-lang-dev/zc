#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "codegen.h"
#include "meta.h"

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

static int count() {
    static int i = 0;
    return ++i;
}

static void gen_expr(FILE *fp, Node *expr) {
    switch(expr->kind) {
    case ND_BLOCK:
        for (int i = 0; i < expr->as.exprs.count; ++i) {
            gen_expr(fp, expr->as.exprs.list[i]);
        }
        return;
    case ND_MUT: // 这里和ND_LET一样
    case ND_LET: {
        gen_expr(fp, expr->as.asn.value);
#ifdef _WIN32
        fprintf(fp, "    mov %s$[rbp], eax\n", get_name(expr->as.asn.name));
#else
        Meta *m = expr->meta;
        fprintf(fp, "    mov dword ptr[rbp-%d], eax\n", m->offset);
#endif
        return;
    }
    case ND_IF: {
        int c = count();
        gen_expr(fp, expr->as.if_else.cond);
        fprintf(fp, "    cmp rax, 0\n");
        fprintf(fp, "    je _L_else%d\n", c);
        gen_expr(fp, expr->as.if_else.then);
        fprintf(fp, "    jmp _L_end%d\n", c);
        fprintf(fp, "_L_else%d:\n", c);
        if (expr->as.if_else.els != NULL) {
            gen_expr(fp, expr->as.if_else.els);
        }
        fprintf(fp, "_L_end%d:\n", c);
        return;
    }
    case ND_FOR: {
        int c = count();
        fprintf(fp, "_L_begin%d:\n", c);
        gen_expr(fp, expr->as.loop.cond);
        fprintf(fp, "    cmp rax, 0\n");
        fprintf(fp, "    je _L_end%d\n", c);
        gen_expr(fp, expr->as.loop.body);
        fprintf(fp, "    jmp _L_begin%d\n", c);
        fprintf(fp, "_L_end%d:\n", c);
        return;
    }
    case ND_IDENT: {
#ifdef _WIN32
        // 变量名，需要获取其值
        fprintf(fp, "    mov rax, %s$[rbp]\n", expr->as.path.names[0].name);
#else
        // 先获取变量地址
        Meta *m = expr->meta;
        // 再加载变量值
        fprintf(fp, "    mov eax, [rbp-%d]\n", m->offset);
#endif
        return;
    }
    case ND_LNAME: {
        Meta *m = expr->meta;
        fprintf(fp, "    mov rax, rbp\n");
        if (m->offset != 0) {
            fprintf(fp, "    sub rax, %d\n", m->offset);
        }
        return;
    }
    case ND_INT: {
        fprintf(fp, "    mov rax, %d\n", expr->as.num.val);
        return;
    }
    case ND_NEG: {
        gen_expr(fp, expr->as.una.body);
        fprintf(fp, "    neg rax\n");
        return;
    }
    case ND_NOT: {
        gen_expr(fp, expr->as.una.body);
        fprintf(fp, "    xor al, -1\n");
        fprintf(fp, "    and al, 1\n");
        fprintf(fp, "    movzx rax, al\n");
        return;
    }
    case ND_BOOL: {
        if (expr->as.bul) {
            fprintf(fp, "    mov rax, 1\n");
        } else {
            fprintf(fp, "    mov rax, 0\n");
        }
        return;
    }
    case ND_CALL: {
        // 处理函数名称
        CallExpr *call = &expr->as.call;
        char *name = get_name(call->name);
        bool is_print = strcmp(name, "print") == 0;
#ifdef _WIN32
        if (is_print) { // printf 要单独处理，加上'\n'
            Node *arg = call->args[0];
            ConstData *data = (ConstData*)arg->meta;
            if (arg->kind == ND_INT) {
                fprintf(fp, "    lea rcx, ct%d\n", data->idx);
                fprintf(fp, "    mov rdx, %d\n", call->args[0]->as.num.val);
            } else {
                fprintf(fp, "    lea rcx, ct%d\n", data->idx);
            }
            fprintf(fp, "    call printf\n");
        } else {
            for (int i = 0; i < call->argc; ++i) {
                Node *arg = call->args[i];
                ConstData *data = (ConstData*)arg->meta;
                if (arg->kind == ND_INT) {
                    fprintf(fp, "    mov %s, %d\n", WIN_REGS[i], arg->as.num.val);
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
                fprintf(fp, "    mov rsi, %d\n", arg->as.num.val);
            }
            fprintf(fp, "    call printf\n");
        } else {
            for (int i = 0; i < call->argc; ++i) {
                Node *arg = call->args[i];
                ConstData *data = (ConstData*)arg->meta;
                if (arg->kind == ND_INT) {
                    fprintf(fp, "    mov %s, %d\n", LINUX_REGS[i], arg->as.num.val);
                } else {
                    fprintf(fp, "    lea %s, [rip+ct%d]\n", LINUX_REGS[i], i);
                }
            }
            fprintf(fp, "    call %s\n", name);
        }
#endif
        return;
    }
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
        int right = expr->as.bop.right->as.num.val;
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
        case OP_EQ:
            fprintf(fp, "    cmp rax, %d\n", right);
            fprintf(fp, "    sete al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_NE:
            fprintf(fp, "    cmp rax, %d\n", right);
            fprintf(fp, "    setne al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_LT:
            fprintf(fp, "    cmp rax, %d\n", right);
            fprintf(fp, "    setl al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_LE: 
            fprintf(fp, "    cmp rax, %d\n", right);
            fprintf(fp, "    setle al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_GT:
            fprintf(fp, "    cmp rax, %d\n", right);
            fprintf(fp, "    setg al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_GE:
            fprintf(fp, "    cmp rax, %d\n", right);
            fprintf(fp, "    setge al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_ASN:
            fprintf(fp, "    mov dword ptr [rax], %d\n", right);
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
        case OP_EQ:
            fprintf(fp, "    cmp rax, rdi\n");
            fprintf(fp, "    sete al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_NE:
            fprintf(fp, "    cmp rax, rdi\n");
            fprintf(fp, "    setne al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_LT:
            fprintf(fp, "    cmp rax, rdi\n");
            fprintf(fp, "    setl al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_LE: 
            fprintf(fp, "    cmp rax, rdi\n");
            fprintf(fp, "    setle al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_GT:
            fprintf(fp, "    cmp rax, rdi\n");
            fprintf(fp, "    setg al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_GE:
            fprintf(fp, "    cmp rax, rdi\n");
            fprintf(fp, "    setge al\n");
            fprintf(fp, "    movzx rax, al\n");
            break;
        case OP_AND:
            fprintf(fp, "    and rax, rdi\n");
            break;
        case OP_OR:
            fprintf(fp, "    or rax, rdi\n");
            break;
        case OP_ASN:
            fprintf(fp, "    mov dword ptr[rax], edi\n");
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
            char *name = get_name(expr->as.call.name);
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
                        META.data[META.data_count].value = get_name(arg);
                    }
                } else {
                    if (arg->kind == ND_STR) {
                        META.data[META.data_count].value = get_name(arg);
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

static int align16(int n) {
    return n % 16 == 0 ? n : n + 16 - n % 16;
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
    int locals_size = total_meta_size();
    if (has_call || locals_size > 0) {
        // prolog
        fprintf(fp, "    push rbp\n");
        fprintf(fp, "    mov rbp, rsp\n");
    }
    if (locals_size > 0) {
        // size+8之后要16字节对齐，其中单独的8时`push rbp`导致的。
        locals_size = align16(locals_size + 8) - 8;
        fprintf(fp, "    sub rsp, %d\n", locals_size);
    }

    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        gen_expr(fp, expr);
    }

    if (has_call || locals_size > 0) {
        // epilog
        fprintf(fp, "    pop rbp\n");
    }
    if (locals_size > 0) {
        fprintf(fp, "    add rsp, %d\n", locals_size);
    }
    fprintf(fp, "    ret\n");

    do_data_linux(fp);

    // 保存并关闭文件
    fclose(fp);
}

static bool do_locals(FILE *fp) {
    HashTable *table = global_scope()->metas;
    HashIter *i = hash_iter(table);
    bool has_locals = false;
    while (hash_next(table, i)) {
        Meta *meta = (Meta*)i->value;
        if (meta->kind == ND_FN) {
            // TODO: 处理自定义函数
            continue;
        }
        fprintf(fp, "%s$ = -%d\n", get_name(meta->node), meta->offset);
        has_locals = true;
    }
    return has_locals;
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

    bool has_locals = do_locals(fp);

    int reserve = align16(total_meta_size()) + 8;

    fprintf(fp, "main proc\n");

    if (has_call || has_locals) {
        // prolog
        fprintf(fp, "    push rbp\n");
        fprintf(fp, "    mov rbp, rsp\n");
    }
    if (has_locals) {
        // reserve stack for shadow space
        fprintf(fp, "    sub rsp, %d\n", reserve);
    }

    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        gen_expr(fp, expr);
    }

    if (has_locals) {
        // restore stack
        fprintf(fp, "    add rsp, %d\n", reserve);
    }
    if (has_call || has_locals) {
        // epilog
        fprintf(fp, "    pop rbp\n");
    }
    fprintf(fp, "    ret\n");

    // 结束
    fprintf(fp, "main endp\n");
    fprintf(fp, "end\n");
    fclose(fp);

    return;
}
