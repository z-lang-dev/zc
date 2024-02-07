#include "compiler.h"
#include "util.h"
#include "front.h"
#include "codegen.h"

void build(char *file) {
    printf("Building %s\n", file);
    Node *prog = do_file(new_front(), file);
    log_trace("Parsed total meta size: %d\n", total_meta_size());

    // 生成汇编
#ifdef _WIN32
    codegen_win(prog);
#else
    codegen_linux(prog);
#endif
}
