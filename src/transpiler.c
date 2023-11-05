#include <stdio.h>
#include "transpiler.h"
#include "parser.h"
#include "util.h"

// 将AST编译成C代码
static void codegen_c(CallExpr *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.c", "w");
    // 引入标准库
    fprintf(fp, "#include <stdio.h>\n");
    // main函数
    fprintf(fp, "int main(void) {\n");
    // 调用printf函数
    fprintf(fp, "    printf(\"%s\");\n", expr->arg);
    // 返回
    fprintf(fp, "    return 0;\n");
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
    CallExpr *expr = parse_expr(code);
    // 输出C代码
    codegen_c(expr);
}

// 将AST编译成Python代码
static void codegen_py(CallExpr *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.py", "w");
    // main函数
    fprintf(fp, "print(\"%s\")\n", expr->arg);
    // 保存并关闭文件
    fclose(fp);
}

void trans_py(char *file) {
    log_trace("Transpiling %s to Python\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    CallExpr *expr = parse_expr(code);
    // 输出Python代码
    codegen_py(expr);
}

// 将AST编译成JS代码
static void codegen_js(CallExpr *expr) {
    // 打开输出文件
    FILE *fp = fopen("app.js", "w");
    // main函数
    fprintf(fp, "console.log(\"%s\")\n", expr->arg);
    // 保存并关闭文件
    fclose(fp);
}

void trans_js(char *file) {
    log_trace("Transpiling %s to JS\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    CallExpr *expr = parse_expr(code);
    // 输出JS代码
    codegen_js(expr);
}
