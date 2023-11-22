#include "compiler.h"
#include "util.h"
#include "parser.h"
#include "codegen.h"

void build(char *file) {
    printf("Building %s\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Parser *parser = new_parser(code);
    Node *prog = parse(parser);
    // 生成汇编
#ifdef _WIN32
    codegen_win(prog);
#else
    codegen_linux(prog);
#endif
}
