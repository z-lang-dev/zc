#include "builtin.h"

Meta *new_builtin(char *name) {
    Node *fn = new_node(ND_FN);
    fn->as.str = name;
    return new_meta(fn);
}

void make_builtins(Scope *scope) {
    global_set("print", new_builtin("print"));
    global_set("pwd", new_builtin("pwd"));
    global_set("ls", new_builtin("ls"));
    global_set("cd", new_builtin("cd"));
    global_set("cat", new_builtin("cat"));
}


Meta *new_stdfn(char *name) {
    Node *fn = new_node(ND_FN);
    fn->as.str = name;
    Meta *m = new_meta(fn);
    m->name = name;
    m->kind = MT_FN;
    return m;
}

void use_stdz(Scope *scope) {
   global_set("read_file", new_stdfn("read_file"));
   global_set("write_file", new_stdfn("write_file"));
}