#include "compiler.h"
#include "util.h"
#include "parser.h"
#include "codegen.h"

void build(char *file) {
    printf("Building %s\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    CallExpr *expr = parse_expr(code);
    // 输出汇编代码
#ifdef _WIN32
    codegen_win(expr);
#else
    codegen_linux(expr);
#endif
}

void run(char *file) {
    printf("TODO: Running %s\n", file);
}