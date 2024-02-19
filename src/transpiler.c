#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include "zast.h"
#include "transpiler.h"
#include "parser.h"
#include "util.h"
#include "builtin.h"
#include "front.h"

#define MAX_USES 100
typedef struct TransMeta TransMeta;

typedef enum {
    LAN_C,
    LAN_PY,
    LAN_JS,
} LAN;

struct TransMeta {
    int indent;
    int use_count;
    char *uses[MAX_USES];
    LAN lan;
};

static TransMeta META;

// 检查是否需要引入标准库
static void do_meta(Node *prog) {
    // init META
    META.indent = 0;
    META.use_count = 0;
    META.lan = LAN_C;
    char *name_in_use = "";
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_CALL) {
            if (expr->as.call.name->kind == ND_NAME) {
                char *name = get_name(expr->as.call.name);
                if (strcmp(name, "print") == 0) {
                    META.uses[META.use_count++] = "<stdio.h>";
                } else if (strcmp(name, name_in_use) != 0) {
                    META.uses[META.use_count++] = "\"stdz.h\"";
                }
            }
        } else if (expr->kind == ND_USE) {
            META.uses[META.use_count++] = sfmt("\"%s.h\"", expr->as.use.mod);
            if (expr->as.use.name) {
                name_in_use = expr->as.use.name;
            }
        }
    }
}

void gen_expr(FILE *fp, Node *expr);

// 生成函数定义
static void gen_fn(FILE *fp, Node *expr) {
    char *name = expr->as.fn.name;
    Params *params = expr->as.fn.params;
    switch (META.lan) {
    case LAN_C: {
        // TODO: 由于还没有支持返回类型，这里暂时统一都写成int
        fprintf(fp, "int %s(", name);
        if (params == NULL) {
            fprintf(fp, "void");
        } else {
            for (int i = 0; i < params->count; ++i) {
                Node *param = params->list[i];
                fprintf(fp, "int %s", param->as.str);
                if (i < params->count - 1) {
                    fprintf(fp, ", ");
                }
            }
        }
        fprintf(fp, ") ");
        gen_expr(fp, expr->as.fn.body);
        fprintf(fp, "\n");
        break;
    }
    case LAN_PY: {
        fprintf(fp, "def %s(", name);
        if (params != NULL) {
            for (int i = 0; i < params->count; ++i) {
                Node *param = params->list[i];
                fprintf(fp, "%s", param->as.str);
                if (i < params->count - 1) {
                    fprintf(fp, ", ");
                }
            }
        }
        fprintf(fp, "):\n");
        gen_expr(fp, expr->as.fn.body);
        break;
    }
    case LAN_JS: {
        fprintf(fp, "export function %s(", name);
        if (params != NULL) {
            for (int i = 0; i < params->count; ++i) {
                Node *param = params->list[i];
                fprintf(fp, "%s", param->as.str);
                if (i < params->count - 1) {
                    fprintf(fp, ", ");
                }
            }
        }
        fprintf(fp, ") ");
        gen_expr(fp, expr->as.fn.body);
        fprintf(fp, "\n");
        break;
    }
    }
}

static void add_indent() {
    META.indent++;
}

static void sub_indent() {
    META.indent--;
}

static void print_indent(FILE *fp) {
    for (int i = 0; i < META.indent; ++i) {
        fprintf(fp, "    ");
    }
}

static bool is_void_call(Node *expr) {
    // 暂时按名字处理，未来可以通过“返回类型”来判断。
    return expr->kind == ND_CALL && (
        expr->as.call.name->kind == ND_NAME && (
        strcmp(expr->as.call.name->as.str, "print") == 0 || 
        strcmp(expr->as.call.name->as.str, "read_file") == 0 ||
        strcmp(expr->as.call.name->as.str, "write_file") == 0
        )
    );
}

// 生成C语言的printf
static void cprintf(FILE *fp, Node *val) {
    Type *type = NULL;
    if (val->meta) {
        type = val->meta->type;
    }
    if (type && type->kind == TY_ARRAY) {
        fprintf(fp, "printf(\"[");
        for (int i = 0; i < type->as.array.size; ++i) {
            gen_expr(fp, val->as.array.items[i]);
            if (i < type->as.array.size - 1) {
                fprintf(fp, ", ");
            }
        }
        fprintf(fp, "]\\n\")");
        return;
    }
    switch (val->kind) {
    case ND_INT:
        fprintf(fp, "printf(\"%%d\\n\", ");
        break;
    case ND_BOOL:
        fprintf(fp, "printf(\"%%s\\n\", ");
        break;
    case ND_FLOAT:
        fprintf(fp, "printf(\"%%f\\n\", ");
        break;
    case ND_DOUBLE:
        fprintf(fp, "printf(\"%%lf\\n\", ");
        break;
    case ND_STR:
        fprintf(fp, "printf(\"%%s\\n\", ");
        break;
    case ND_INDEX:
    case ND_BINOP: {
        if (type == NULL) break;
        switch (type->kind) {
        case TY_INT:
            fprintf(fp, "printf(\"%%d\\n\", ");
            break;
        case TY_BOOL:
            fprintf(fp, "printf(\"%%s\\n\", ");
            break;
        case TY_FLOAT:
            fprintf(fp, "printf(\"%%f\\n\", ");
            break;
        case TY_DOUBLE:
            fprintf(fp, "printf(\"%%lf\\n\", ");
            break;
        }
        break;
    }
    default:
        fprintf(fp, "printf(\"%%d\\n\", ");
        break;
    }
    gen_expr(fp, val);
    if (type && type->kind == TY_BOOL) fprintf(fp, " ? \"true\" : \"false\"");
    fprintf(fp, ")");
}

// 生成一个语句
static void gen_expr(FILE *fp, Node *expr) {
    switch (expr->kind) {
    case ND_BLOCK: {
        bool need_return = (expr->meta && ((Meta*)expr->meta)->need_return) ? true : false;
        add_indent();
        if (META.lan != LAN_PY) fprintf(fp, "{\n");
        int cnt = expr->as.exprs.count;
        if (cnt > 0) {
            for (int i = 0; i < expr->as.exprs.count - 1; ++i) {
                Node *e = expr->as.exprs.list[i];
                if (e->kind == ND_USE) continue;
                print_indent(fp);
                gen_expr(fp, e);
                if (META.lan == LAN_C) fprintf(fp, ";\n");
                else fprintf(fp, "\n");
            }
            // 处理最后一句
            Node *last = expr->as.exprs.list[expr->as.exprs.count - 1];
            if (need_return) {
                if (META.lan == LAN_C) {
                    if (is_void_call(last)) {
                        print_indent(fp);
                        gen_expr(fp, last);
                        fprintf(fp, ";\n");
                        print_indent(fp);
                        fprintf(fp, "return 0;\n");
                    } else {
                        print_indent(fp);
                        fprintf(fp, "return ");
                        gen_expr(fp, last);
                        fprintf(fp, ";\n");
                    }
                } else {
                    print_indent(fp);
                    fprintf(fp, "return ");
                    gen_expr(fp, last);
                    fprintf(fp, "\n");
                }
            } else {
                print_indent(fp);
                gen_expr(fp, last);
                if (META.lan == LAN_C) fprintf(fp, ";\n");
                else fprintf(fp, "\n");
            }
        }
        sub_indent();
        if (META.lan != LAN_PY) {
          print_indent(fp);
          fprintf(fp, "}");
        }
        return;
    }
    case ND_ARRAY: {
        if (META.lan == LAN_C) fprintf(fp, "{");
        else fprintf(fp, "[");

        for (int i = 0; i < expr->as.array.size; ++i) {
            gen_expr(fp, expr->as.array.items[i]);
            if (i < expr->as.array.size - 1) {
                fprintf(fp, ", ");
            }
        }
        if (META.lan == LAN_C) fprintf(fp, "}");
        else fprintf(fp, "]");
        return;
    }
    case ND_INDEX: {
        gen_expr(fp, expr->as.index.array);
        fprintf(fp, "[");
        gen_expr(fp, expr->as.index.idx);
        fprintf(fp, "]");
        return;
    }
    case ND_MUT: {
        char *name = expr->as.asn.name->as.str;
        if (META.lan == LAN_C) {
            Type *type = expr->as.asn.name->meta->type;
            if (type == NULL) type = &TYPE_INT;
            fprintf(fp, "%s %s = ", type->name, name);
        } else if (META.lan == LAN_PY) {
            fprintf(fp, "%s = ", name);
        } else if (META.lan == LAN_JS) {
            fprintf(fp, "var %s = ", name);
        }
        gen_expr(fp, expr->as.asn.value);
        return;
    }
    case ND_LET: {
        char *name = expr->as.asn.name->as.str;
        if (META.lan == LAN_C) {
            Type *type = expr->as.asn.name->meta->type;
            if (type == NULL) type = &TYPE_INT;
            if (type->kind == TY_ARRAY) {
                fprintf(fp, "%s %s[%d] = ", type->as.array.item->name, name, type->as.array.size);
            } else {
                fprintf(fp, "%s %s = ", type->name, name);
            }
        } else if (META.lan == LAN_PY) {
            fprintf(fp, "%s = ", name);
        } else if (META.lan == LAN_JS) {
            fprintf(fp, "let %s = ", name);
        }
        gen_expr(fp, expr->as.asn.value);
        return;
    }
    case ND_IF:
        switch (META.lan) {
        case LAN_C:
        case LAN_JS:
            fprintf(fp, "if (");
            gen_expr(fp, expr->as.if_else.cond);
            fprintf(fp, ") ");
            gen_expr(fp, expr->as.if_else.then);
            fprintf(fp, " else ");
            gen_expr(fp, expr->as.if_else.els);
            break;
        case LAN_PY:
            fprintf(fp, "if ");
            gen_expr(fp, expr->as.if_else.cond);
            fprintf(fp, ":\n");
            gen_expr(fp, expr->as.if_else.then);
            fprintf(fp, "else:\n");
            gen_expr(fp, expr->as.if_else.els);
            break;
        }
        return;
    case ND_FOR: {
        switch (META.lan) {
        case LAN_C:
        case LAN_JS:
            fprintf(fp, "while (");
            gen_expr(fp, expr->as.loop.cond);
            fprintf(fp, ") ");
            gen_expr(fp, expr->as.loop.body);
            break;
        case LAN_PY:
            fprintf(fp, "while ");
            gen_expr(fp, expr->as.loop.cond);
            fprintf(fp, ":\n");
            gen_expr(fp, expr->as.loop.body);
            break;
        }
        return;
    }
    case ND_FN: {
        gen_fn(fp, expr);
        return;
    }
    case ND_LNAME:
    case ND_NAME:
        fprintf(fp, "%s", expr->as.str);
        return;
    case ND_INT:
        fprintf(fp, "%s", expr->as.num.lit);
        return;
    case ND_FLOAT:
        fprintf(fp, "%s", expr->as.float_num.lit);
        return;
    case ND_DOUBLE:
        fprintf(fp, "%s", expr->as.double_num.lit);
        return;
    case ND_BOOL:
        switch (META.lan) {
        case LAN_C:
            // 因为C里true/false还得单独引入stdbool.h，所以这里直接用1/0代替。
            // 未来有了更完善的依赖库引入功能之后，再改回true/false
            fprintf(fp, "%s", expr->as.bul ? "1" : "0"); 
            break;
        case LAN_PY:
            fprintf(fp, "%s", expr->as.bul ? "True" : "False");
            break;
        default:
            fprintf(fp, "%s", expr->as.bul ? "true" : "false");
            break;
        }
        return;
    case ND_STR:
        fprintf(fp, "\"%s\"", expr->as.str);
        return;
    case ND_NEG:
        fprintf(fp, "-(");
        gen_expr(fp, expr->as.una.body);
        fprintf(fp, ")");
        return;
    case ND_NOT:
        if (META.lan == LAN_PY) fprintf(fp, "not (");
        else fprintf(fp, "!(");
        gen_expr(fp, expr->as.una.body);
        fprintf(fp, ")");
        return;
    case ND_USE:
        return;
    case ND_CALL:
        if (META.lan == LAN_C && strcmp(get_name(expr->as.call.name), "print") == 0) {
            // 注意：这里的print仍然只打印第一个参数。多参数的打印，要等Z支持可变长度参数之后再说。
            cprintf(fp, expr->as.call.args[0]);
            return;
        } else {
            fprintf(fp, "%s(", get_name(expr->as.call.name));
            for (int i = 0; i < expr->as.call.argc; ++i) {
                Node *arg = expr->as.call.args[i];
                switch (arg->kind) {
                case ND_INT:
                    fprintf(fp, "%s", arg->as.num.lit);
                    break;
                case ND_BOOL:
                    fprintf(fp, "%s", arg->as.bul ? "true" : "false");
                    break;
                case ND_FLOAT:
                    fprintf(fp, "%s", arg->as.float_num.lit);
                    break;
                case ND_DOUBLE:
                    fprintf(fp, "%s", arg->as.double_num.lit);
                    break;
                case ND_STR:
                    fprintf(fp, "\"%s\"", arg->as.str);
                    break;
                case ND_NAME:
                    fprintf(fp, "%s", arg->as.str);
                    break;
                case ND_BINOP:
                    gen_expr(fp, arg);
                    break;
                case ND_INDEX:
                    gen_expr(fp, arg);
                    break;
                case ND_ARRAY:
                    gen_expr(fp, arg);
                    break;
                default:
                    fprintf(fp, "print: unknown kind of arg: %d\n", arg->kind);
                }
                if (i < expr->as.call.argc - 1) {
                    fprintf(fp, ", ");
                }
            }
            fprintf(fp, ")");
        }
        return;
    }

    if (expr->kind != ND_BINOP) {
        printf("Error: unknown node kind for gen_expr: %d\n", expr->kind);
        return;
    }
    // 处理二元表达式
    // 左膀，gen_expr_win完成之后，结果存在rax中
    gen_expr(fp, expr->as.bop.left);
    // 操作符
    switch (expr->as.bop.op) {
    case OP_ADD:
        fprintf(fp, " + ");
        break;
    case OP_SUB:
        fprintf(fp, " - ");
        break;
    case OP_MUL:
        fprintf(fp, " * ");
        break;
    case OP_DIV:
        fprintf(fp, " / ");
        break;
    case OP_GT:
        fprintf(fp, " > ");
        break;
    case OP_LT:
        fprintf(fp, " < ");
        break;
    case OP_GE:
        fprintf(fp, " >= ");
        break;
    case OP_LE:
        fprintf(fp, " <= ");
        break;
    case OP_EQ:
        fprintf(fp, " == ");
        break;
    case OP_NE:
        fprintf(fp, " != ");
        break;
    case OP_AND:
        if (META.lan == LAN_PY) fprintf(fp, " and ");
        else fprintf(fp, " && ");
        break;
    case OP_OR:
        if (META.lan == LAN_PY) fprintf(fp, " or ");
        else fprintf(fp, " || ");
        break;
    case OP_ASN:
        fprintf(fp, " = ");
        break;
    default:
        printf("Error: unknown operator for binop expr: %d\n", expr->as.bop.op);
    }
    // 右臂
    gen_expr(fp, expr->as.bop.right);
}

static Node *last_expr(Node *prog) {
    return prog->as.exprs.list[prog->as.exprs.count - 1];
}

// 把Z语言代码的语句重新组织：全局定义放在前面；其他语句统一填入main函数
static Node *extract_main(Node *prog) {
    Node *p = new_prog();
    if (prog->as.exprs.count == 0) {
        return prog;
    }
    // main fn
    Node *main = new_node(ND_FN);
    main->as.fn.name = "main";
    main->as.fn.body = new_block();
    Meta *meta = new_meta(main->as.fn.body);
    meta->need_return = true;
    main->as.fn.body->meta = meta;
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) continue;
        if (expr->kind == ND_FN) {
            append_expr(p, expr);
        } else {
            append_expr(main->as.fn.body, expr);
        }
    }
    append_expr(p, main);
    return p;
}

// 将AST编译成C代码
static void codegen_c_app(Node *prog) {
    do_meta(prog);
    META.lan = LAN_C;

    prog = extract_main(prog);
    // 打开输出文件
    FILE *fp = fopen("app.c", "w"); // TODO: 以后还统一叫app.c吗？
    for (int i = 0; i < META.use_count; ++i) {
        fprintf(fp, "#include %s\n", META.uses[i]);
    }
    if (META.use_count > 0) fprintf(fp, "\n");

    // 生成多条语句
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) continue;
        gen_expr(fp, expr);
        if (i < prog->as.exprs.count - 1 && expr->kind == ND_FN) {
            fprintf(fp, "\n");
        }
    }

    // 保存并关闭文件
    fclose(fp);
}

static void gen_fn_header(FILE *fp, Node *expr) {
    char *name = expr->as.fn.name;
    Params *params = expr->as.fn.params;
    fprintf(fp, "int %s(", name);
    if (params == NULL) {
        fprintf(fp, "void");
    } else {
        for (int i = 0; i < params->count; ++i) {
            Node *param = params->list[i];
            fprintf(fp, "int %s", param->as.str);
            if (i < params->count - 1) {
                fprintf(fp, ", ");
            }
        }
    }
    fprintf(fp, ");\n");
}

// 将AST编译成C代码
static void codegen_c_lib(Mod *mod, char *name) {
    Node *prog = mod->prog;
    do_meta(prog);
    META.lan = LAN_C;

    char *c_file = sfmt("%s.c", name);
    char *h_file = sfmt("%s.h", name);

    // 找到所有的定义，放到头文件中
    FILE *hp = fopen(h_file, "w");
    Scope *scope = mod->scope;
    HashIter *i = hash_iter(scope->metas);
    while (hash_next(scope->metas, i)) {
        Meta *meta = (Meta*)i->value;
        switch (meta->kind) {
        case MT_FN: // 暂时只有函数定义需要输出到头文件
            if (meta->is_def == false) continue;
            gen_fn_header(hp, meta->node);
            break;
        }
    }

    fclose(hp);

    // 输出C文件
    FILE *fp = fopen(c_file, "w");
    fprintf(fp, "#include \"%s\"\n", h_file);
    for (int i = 0; i < META.use_count; ++i) {
        fprintf(fp, "#include %s\n", META.uses[i]);
    }
    if (META.use_count > 0) fprintf(fp, "\n");

    // 生成多条语句
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) continue;
        gen_expr(fp, expr);
        if (i < prog->as.exprs.count - 1 && expr->kind == ND_FN) {
            fprintf(fp, "\n");
        }
    }

    // 保存并关闭文件
    fclose(fp);
}

static void codegen_c(Front *front) {
    // 遍历front的所有模块：
    HashIter *i = hash_iter(front->mods);
    while (hash_next(front->mods, i)) {
        Mod *mod = (Mod*)i->value;
        if (strcmp(mod->name, "app") == 0) {
            codegen_c_app(mod->prog);
        } else {
            codegen_c_lib(mod, mod->name);
        }
    }
}

void trans_c(char *file) {
    log_trace("Transpiling %s to C...\n", file);
    // 新建前端
    Front *front = new_front();
    // 解析文件并生成模块
    Mod *mod = do_file(front, file);
    mod->name = "app";
    trace_node(mod->prog);
    // 输出C代码
    codegen_c(front);
}

// 将AST编译成Python代码
static void codegen_py_mod(Mod *mod) {
    Node *prog = mod->prog;
    META.lan = LAN_PY;
    // 打开输出文件
    char *fname = sfmt("%s.py", mod->name);
    FILE *fp = fopen(fname, "w");
    bool has_import = false;
    char *name_in_use = "";
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) {
            char *name = expr->as.use.name;
            if (name && strcmp(name, name_in_use) != 0) {
                fprintf(fp, "from %s import %s\n", expr->as.use.mod, expr->as.use.name);
                has_import = true;
            }
            name_in_use = name;
        } else if (expr->kind == ND_CALL) {
            Node *name_node = expr->as.call.name;
            if (name_node->kind == ND_NAME) {
                char *name = expr->as.call.name->as.str;
                if (strcmp(name, "print") != 0 && strcmp(name, name_in_use) != 0) {
                    fprintf(fp, "from stdz import *\n", META.uses[i]);
                    has_import = true;
                }
            }
        }
    }
    HashIter *i = hash_iter(mod->uses);
    while (hash_next(mod->uses, i)) {
        Node *path = (Node*)i->value;
        if (path->kind != ND_PATH) continue;
        if (path->as.path.len < 2) continue;
        char *mod = path->as.path.names[0].name;
        char *name = path->as.path.names[1].name;
        fprintf(fp, "from %s import %s\n", mod, name);
        has_import = true;
    }
    if (has_import) {
        fprintf(fp, "\n");
    }
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) continue;
        gen_expr(fp, expr);
        fprintf(fp, "\n");
    }   
    // 保存并关闭文件
    fclose(fp);
}

static void codegen_py(Front *front) {
    // 遍历front的所有模块：
    HashIter *i = hash_iter(front->mods);
    while (hash_next(front->mods, i)) {
        Mod *mod = (Mod*)i->value;
        codegen_py_mod(mod);
    }
}

static void use_charts() {
    global_set("pie", new_stdfn("pie"));
}

void trans_py(char *file) {
    log_trace("Transpiling %s to Python\n", file);
    // 新建前端
    Front *front = new_front();
    use_charts();
    // 解析文件并生成模块
    Mod *mod = do_file(front, file);
    scope_set(mod->scope, "pie", new_stdfn("pie"));
    mod->name = "app";
    trace_node(mod->prog);
    codegen_py(front);
}

// 将AST编译成JS代码
static void codegen_js_mod(Mod *mod) {
    Node *prog = mod->prog;
    META.lan = LAN_JS;
    Node *expr = prog->as.exprs.list[0];
    // 打开输出文件
    char *fname = sfmt("%s.js", mod->name);
    FILE *fp = fopen(fname, "w");
    // 第一道收集信息，顺便打出import语句
    bool has_import = false;
    char *name_in_use = "";
    // TODO: 把这些逻辑统一移到parser.uses中
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) {
            char *name = expr->as.use.name;
            if (name != NULL && strcmp(name, name_in_use) != 0) {
                fprintf(fp, "import {%s} from \"./%s\"\n", expr->as.use.name, expr->as.use.mod);
                has_import = true;
            }
            name_in_use = name;
        } else if (expr->kind == ND_CALL) {
            Node *name_node = expr->as.call.name;
            if (name_node->kind == ND_NAME) {
                char *name = get_name(expr->as.call.name);
                // 注意，print直接替换为console.log即可
                if (strcmp(name, "print") == 0) {
                    expr->as.call.name->as.str = "console.log";
                } else if (expr->meta) {
                    Meta *m = (Meta*)expr->meta;
                    if (m->kind == MT_FN && m->is_def == false) {
                        fprintf(fp, "import {%s} from \"./stdz.js\"\n", name);
                        has_import = true;
                    }
                } else if (strcmp(name, name_in_use) != 0) {
                    fprintf(fp, "import {%s} from \"./stdz.js\"\n", name);
                    has_import = true;
                }
            }
        }
    }
    HashIter *i = hash_iter(mod->uses);
    while (hash_next(mod->uses, i)) {
        Node *path = (Node*)i->value;
        if (path->kind != ND_PATH) continue;
        if (path->as.path.len < 2) continue;
        char *mod = path->as.path.names[0].name;
        char *name = path->as.path.names[1].name;
        fprintf(fp, "import {%s} from \"./%s\"\n", name, mod);
        has_import = true;
    }
    if (has_import) {
        fprintf(fp, "\n");
    }
    // 第二道，遍历每个语句，生成代码
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) continue;
        gen_expr(fp, expr);
        fprintf(fp, "\n");
    }
    
    // 保存并关闭文件
    fclose(fp);
}

static void codegen_js(Front *front) {
    // 遍历front的所有模块：
    HashIter *i = hash_iter(front->mods);
    while (hash_next(front->mods, i)) {
        Mod *mod = (Mod*)i->value;
        codegen_js_mod(mod);
    }
}

static void use_js_stdz() {
    global_set("alert", new_stdfn("alert"));
}

void trans_js(char *file) {
    log_trace("Transpiling %s to JS\n", file);
    // 新建前端
    Front *front = new_front();
    use_js_stdz();
    // 解析文件并生成模块
    Mod *mod = do_file(front, file);
    mod->name = "app";
    trace_node(mod->prog);
    codegen_js(front);
}
