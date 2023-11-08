#include <stdio.h>
#include <stdbool.h>
#include "transpiler.h"
#include "parser.h"
#include "util.h"

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
        if (expr->kind == ND_ADD) {
            fprintf(fp, "    return %d + %d;\n", expr->as.bop.left->as.num, expr->as.bop.right->as.num);
        } else {
            // 返回
            fprintf(fp, "    return %d;\n", val->as.num);
        }
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
    } else if (expr->kind == ND_ADD) {
        fprintf(fp, "%d + %d\n", expr->as.bop.left->as.num, expr->as.bop.right->as.num);
    } else { // kind == ND_INT，直接输出数字
        fprintf(fp, "%d\n", expr->as.num);
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
    } else if (expr->kind == ND_ADD) {
        // 直接输出数字
        fprintf(fp, "%d + %d\n", expr->as.bop.left->as.num, expr->as.bop.right->as.num);
    } else {
        // 直接输出数字
        fprintf(fp, "%d\n", expr->as.num);
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
