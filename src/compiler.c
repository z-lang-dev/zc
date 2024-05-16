#include "compiler.h"
#include "util.h"
#include "front.h"
#include "codegen.h"

void build(char *file) {
    printf("Building %s\n", file);
    init_global_scope(SC_BLOCK);
    Front *front = new_front();
    Mod *mod = do_file(front, file);
    // TODO: 需要真正的全局视野，而不只是模块视野
    GlobalScope = mod->scope;
    Node *prog = mod->prog;
    log_trace("Parsed total meta size: %d\n", total_meta_size());

    // 生成汇编
#ifdef _WIN32
    codegen_win(prog);
#else
    codegen_linux(prog);
#endif
}
