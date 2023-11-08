#include <stdio.h>
#include <stdbool.h>
#include "transpiler.h"
#include "parser.h"
#include "util.h"

static void gen_expr(FILE *fp, Node *expr) {
    if (expr->kind == ND_INT) {
        fprintf(fp, "%d", expr->as.num);
        return;
    }

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
            fprintf(fp, " + %d", right);
            break;
        case OP_SUB:
            fprintf(fp, " - %d", right);
            break;
        case OP_MUL:
            fprintf(fp, " * %d", right);
            break;
        case OP_DIV:
            fprintf(fp, " / %d", right);
            break;
        default:
            printf("Error: unknown operator for binop expr: %d\n", expr->as.bop.op);
        }
    } else {
        printf("Error: unsupported right operand for binop expr: %d\n", expr->as.bop.right->kind);
    }
}

// 将AST编译成C代码
static void codegen_c(Node *expr) {
    bool is_call = expr->kind == ND_CALL;
    Node *val = is_call ? expr->as.call.arg : expr;
    
    // 打开输出文件
    FILE *fp = fopen("app.c", "w");
    // 引入标准库
    if (is_call) {
        fprintf(fp, "#include <stdio.h>\n");
    }
    // main函数
    fprintf(fp, "int main(void) {\n");
    // 调用printf函数
    if (is_call) {
        if (val->kind == ND_INT) {
            fprintf(fp, "    printf(\"%%d\\n\", %d);\n", val->as.num);
        } else {
            fprintf(fp, "    printf(\"%s\");\n", val->as.str);
        }
        // 返回
        fprintf(fp, "    return 0;\n");
    } else {
        fprintf(fp, "    return ");
        gen_expr(fp, expr);
        fprintf(fp, ";\n");
    }
    
    // 结束
    fprintf(fp, "}\n");
    // 保存并关闭文件
    fclose(fp);
}

void trans_c(char *file) {
    log_trace("Transpiling %s to C\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Node *expr = parse_expr(code);
    // 输出C代码
    codegen_c(expr);
}

// 将AST编译成Python代码
static void codegen_py(Node *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.py", "w");
    if (expr->kind == ND_CALL) {
        Node *arg = expr->as.call.arg;
        // main函数
        if (arg->kind == ND_INT) {
            fprintf(fp, "print(%d)\n", arg->as.num);
        } else {
            fprintf(fp, "print(\"%s\")\n", arg->as.str);
        }
    }  else {
        gen_expr(fp, expr);
        fprintf(fp, "\n");
    }   
    // 保存并关闭文件
    fclose(fp);
}

void trans_py(char *file) {
    log_trace("Transpiling %s to Python\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Node *expr = parse_expr(code);
    // 输出Python代码
    codegen_py(expr);
}

// 将AST编译成JS代码
static void codegen_js(Node *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.js", "w");
    if (expr->kind == ND_CALL) {
        Node *arg = expr->as.call.arg;
        // main函数
        if (arg->kind == ND_INT) {
            fprintf(fp, "console.log(%d)\n", arg->as.num);
        } else {
            fprintf(fp, "console.log(\"%s\")\n", arg->as.str);
        }
    }  else {
        gen_expr(fp, expr);
        fprintf(fp, "\n");
    }
    // 保存并关闭文件
    fclose(fp);
}

void trans_js(char *file) {
    log_trace("Transpiling %s to JS\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Node *expr = parse_expr(code);
    // 输出JS代码
    codegen_js(expr);
}
