#include <stdlib.h>
#include "front.h"
#include "util.h"
#include "builtin.h"
#include "parser.h"

Front *new_front() {
    Front *front = calloc(1, sizeof(Front));
    front->sources = calloc(1, sizeof(SourceQueue));
    front->sources->count = 0;
    front->sources->cap = 4;
    front->sources->list = calloc(4, sizeof(Source *));
    front->mods = new_hash_table();
    return front;
}

static Mod *process_src(Front *front, Source *src) {
    Parser *parser = new_parser(src->code);
    parser->front = front;
    make_builtins(global_scope());
    use_stdz(global_scope());
    Node *prog = parse(parser);
    Mod *mod = calloc(1, sizeof(Mod));
    mod->prog = prog;
    mod->scope = parser->scope;
    mod->name = remove_ext(src->name);
    return mod;
}

Mod *do_file(Front *front, const char *path) {
    // 先读取文件
    Source *src = load_source(front, path);
    char *code = src->code;
    Mod *mod = process_src(front, src);
    hash_set(front->mods, mod->name, mod);
    return mod;
}

// TODO: code需要指定对应的mod，而不是次次都新建mod
Mod *do_code(Front *front, const char *code) {
    // 解析出AST
    Source *src = add_source(front, code);
    Mod *mod = process_src(front, src);
    hash_set(front->mods, mod->name, mod);
}

static void append_source(SourceQueue *sq, Source *src) {
    if (sq->count >= sq->cap) {
        sq->cap = sq->cap * 2 + 1;
        sq->list = realloc(sq->list, sizeof(Source *) * sq->cap);
    }
    sq->list[sq->count++] = src;
}

static Source *new_source(const char *path) {
    Source *src = calloc(1, sizeof(Source));
    src->name = path;
    return src;
}

Source *load_source(Front *front, const char *path) {
    Source *src = new_source(path);
    src->code = read_src(path);
    append_source(front->sources, src);
    return src;
}

Source *add_source(Front *front, const char *code) {
    Source *src = new_source("<code>"); // TODO: 未来添加更有意义的名称
    append_source(front->sources, src);
    src->code = code;
    return src;
}

Meta *name_lookup(Front *front, const char *name) {
    return NULL;
}

Meta *path_lookup(Front *front, const char *path) {
    return NULL;
}