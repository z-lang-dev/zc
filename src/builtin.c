#include "builtin.h"
#include "type.h"

Meta *new_builtin(char *name) {
    Node *fn = new_node(ND_FN);
    fn->as.str = name;
    return new_meta(fn);
}

void make_builtins(Scope *scope) {
    scope_set(scope, "print", new_builtin("print"));
    scope_set(scope, "pwd", new_builtin("pwd"));
    scope_set(scope, "ls", new_builtin("ls"));
    scope_set(scope, "cd", new_builtin("cd"));
    scope_set(scope, "cat", new_builtin("cat"));
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
   scope_set(scope, "read_file", new_stdfn("read_file"));
   scope_set(scope, "write_file", new_stdfn("write_file"));
}

void init_builtin_types() {
    global_set("int", new_type_meta(&TYPE_INT));
    global_set("bool", new_type_meta(&TYPE_BOOL));
    global_set("byte", new_type_meta(&TYPE_BYTE));
    global_set("float", new_type_meta(&TYPE_FLOAT));
    global_set("double", new_type_meta(&TYPE_DOUBLE));
}