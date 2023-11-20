#include <stdio.h>
#include <stdbool.h>
#include "transpiler.h"
#include "parser.h"
#include "util.h"

static void gen_expr(FILE *fp, Node *expr) {
    switch (expr->kind) {
    case ND_INT:
        fprintf(fp, "%d", expr->as.num);
        return;
    case ND_NEG:
        fprintf(fp, "-(");
        gen_expr(fp, expr->as.una.body);
        fprintf(fp, ")");
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
        switch (expr->as.bop.op) {
        case OP_ADD:
            fprintf(fp, " + ");
            break;
        case OP_SUB:
            fprintf(fp, " - ");
            break;
        case OP_MUL:
            fprintf(fp, " * ");
            break;
        case OP_DIV:
            fprintf(fp, " / ");
            break;
        default:
            printf("Error: unknown operator for binop expr: %d\n", expr->as.bop.op);
        }
        gen_expr(fp, expr->as.bop.right);
    }
}

// 将AST编译成C代码
static void codegen_c(Node *expr) {
    bool is_call = expr->kind == ND_CALL;
    bool is_print = strcmp(expr->as.call.fname->as.str, "print") == 0;
    Node *val = is_call ? expr->as.call.args[0] : expr;
    
    // 打开输出文件
    FILE *fp = fopen("app.c", "w");
    // 引入标准库
    if (is_call) {
        fprintf(fp, "#include <stdio.h>\n");
        if (!is_print) {
            fprintf(fp, "#include \"std.h\"\n");
        }
    }
    // main函数
    fprintf(fp, "int main(void) {\n");
    // 调用printf函数
    if (is_call) {
        if (is_print) {
            if (val->kind == ND_INT) {
                fprintf(fp, "    printf(\"%%d\\n\", %d);\n", val->as.num);
            } else {
                fprintf(fp, "    printf(\"%s\");\n", val->as.str);
            }
        } else { // 调用普通函数
            fprintf(fp, "    %s(", expr->as.call.fname->as.str);
            for (int i = 0; i < expr->as.call.argc; ++i) {
                Node *arg = expr->as.call.args[i];
                if (arg->kind == ND_INT) {
                    fprintf(fp, "%d", arg->as.num);
                } else {
                    fprintf(fp, "\"%s\"", arg->as.str);
                }
                if (i < expr->as.call.argc - 1) {
                    fprintf(fp, ", ");
                }
            }
            fprintf(fp, ");\n");
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
    Parser *parser = new_parser(code);
    Node *expr = parse(parser);
    trace_node(expr);
    // 输出C代码
    codegen_c(expr);
}

// 将AST编译成Python代码
static void codegen_py(Node *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.py", "w");
    if (expr->kind == ND_CALL) {
        Node *arg = expr->as.call.args[0];
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
    Parser *parser = new_parser(code);
    Node *expr = parse(parser);
    // 输出Python代码
    codegen_py(expr);
}

// 将AST编译成JS代码
static void codegen_js(Node *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.js", "w");
    if (expr->kind == ND_CALL) {
        Node *arg = expr->as.call.args[0];
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
    Parser *parser = new_parser(code);
    Node *expr = parse(parser);
    // 输出JS代码
    codegen_js(expr);
}
