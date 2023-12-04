#include "compiler.h"
#include "util.h"
#include "parser.h"
#include "codegen.h"
#include "meta.h"
#include "builtin.h"


void build(char *file) {
    printf("Building %s\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Parser *parser = new_parser(code);
    make_builtins(global_scope());
    use_stdz(global_scope());
    Node *prog = parse(parser);
    log_trace("Parsed total meta size: %d\n", total_meta_size());

    // 生成汇编
#ifdef _WIN32
    codegen_win(prog);
#else
    codegen_linux(prog);
#endif
}
