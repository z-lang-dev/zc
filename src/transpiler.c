#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include "transpiler.h"
#include "parser.h"
#include "util.h"
#include "builtin.h"

#define MAX_USES 100
typedef struct TransMeta TransMeta;

typedef enum {
    LAN_C,
    LAN_PY,
    LAN_JS,
} LAN;
struct TransMeta {
    int use_count;
    char *uses[MAX_USES];

    LAN lan;
};

static TransMeta META;

char* sfmt(const char* fmt, char *msg) {
    size_t needed = snprintf(NULL, 0, fmt, msg, strerror(errno), errno) + 1;
    char  *buffer = malloc(needed);
    sprintf(buffer, fmt, msg, strerror(errno), errno);
    return buffer;
}

static void do_meta(Node *prog) {
    char *name_in_use = "";
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_CALL) {
            char *name = expr->as.call.name->as.str;
            if (strcmp(name, "print") == 0) {
                META.uses[META.use_count++] = "<stdio.h>";
            } else if (strcmp(name, name_in_use) != 0) {
                META.uses[META.use_count++] = "\"stdz.h\"";
            }
        } else if (expr->kind == ND_USE) {
            META.uses[META.use_count++] = sfmt("\"%s.h\"", expr->as.use.box);
            name_in_use = expr->as.use.name;
        }
    }
}

static void gen_expr(FILE *fp, Node *expr) {
    switch (expr->kind) {
    case ND_BLOCK:
        char *tab = "    ";
        if (META.lan != LAN_PY) fprintf(fp, "{\n");
        for (int i = 0; i < expr->as.exprs.count; ++i) {
            Node *e = expr->as.exprs.list[i];
            if (e->kind == ND_USE) continue;
            fprintf(fp, "%s", tab); // 暂时只支持一层缩进
            // if (META.lan == LAN_C && i == expr->as.exprs.count - 1) {
                // fprintf(fp, "return ");
            // }
            gen_expr(fp, e);
            if (META.lan == LAN_C) fprintf(fp, ";\n");
            else fprintf(fp, "\n");
        }
        if (META.lan != LAN_PY) fprintf(fp, "}");
        return;
    case ND_MUT: {
        char *name = expr->as.asn.name->as.str;
        if (META.lan == LAN_C) {
            fprintf(fp, "    ");
            fprintf(fp, "int %s = ", name);
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
            fprintf(fp, "    ");
            fprintf(fp, "int %s = ", name);
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
    case ND_LNAME:
    case ND_NAME:
        fprintf(fp, "%s", expr->as.str);
        return;
    case ND_INT:
        fprintf(fp, "%d", expr->as.num);
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
        if (META.lan == LAN_C && strcmp(expr->as.call.name->as.str, "print") == 0) {
            Node *val = expr->as.call.args[0];
            if (val->kind == ND_INT) {
                fprintf(fp, "    printf(\"%%d\\n\", %d)", val->as.num);
            } else {
                fprintf(fp, "    printf(\"%s\\n\")", val->as.str);
            }
            return;
        } else {
            char *tab = META.lan == LAN_C ? "    " : "";
            fprintf(fp, "%s%s(", tab, expr->as.call.name->as.str);
            for (int i = 0; i < expr->as.call.argc; ++i) {
                Node *arg = expr->as.call.args[i];
                if (arg->kind == ND_INT) {
                    fprintf(fp, "%d", arg->as.num);
                } else {
                    fprintf(fp, "\"%s\"", arg->as.str);
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

// 将AST编译成C代码
static void codegen_c(Node *prog) {
    do_meta(prog);
    META.lan = LAN_C;
    
    // 打开输出文件
    FILE *fp = fopen("app.c", "w");
    for (int i = 0; i < META.use_count; ++i) {
        fprintf(fp, "#include %s\n", META.uses[i]);
    }

    // main函数
    fprintf(fp, "int main(void) {\n");

    // 生成多条语句
    if (prog->as.exprs.count > 1) {
        for (int i = 0; i < prog->as.exprs.count - 1; ++i) {
            Node *expr = prog->as.exprs.list[i];
            if (expr->kind == ND_USE) continue;
            gen_expr(fp, expr);
            fprintf(fp, ";\n"); // C语句需要`;`结尾
        }
    }

    // 最后一条语句需要处理`return`
    Node *last = last_expr(prog);
    if (last->kind == ND_INT || last->kind == ND_BOOL || last->kind == ND_BINOP || last->kind == ND_NEG || last->kind == ND_NAME) {
        fprintf(fp, "    return ");
        gen_expr(fp, last);
        fprintf(fp, ";\n");
    } else {
        gen_expr(fp, last);
        fprintf(fp, ";\n");
        fprintf(fp, "    return 0;\n");
    }

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
    Parser *parser = new_parser(code);
    make_builtins(global_scope());
    use_stdz(global_scope());
    Node *prog = parse(parser);
    trace_node(prog);
    // 输出C代码
    codegen_c(prog);
}

// 将AST编译成Python代码
static void codegen_py(Node *prog) {
    META.lan = LAN_PY;
    // 打开输出文件
    FILE *fp = fopen("app.py", "w");
    bool has_import = false;
    char *name_in_use = "";
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) {
            char *name = expr->as.use.name;
            if (strcmp(name, name_in_use) != 0) {
                fprintf(fp, "from %s import %s\n", expr->as.use.box, expr->as.use.name);
                has_import = true;
            }
            name_in_use = name;
        } else if (expr->kind == ND_CALL) {
            char *name = expr->as.call.name->as.str;
            if (strcmp(name, "print") != 0 && strcmp(name, name_in_use) != 0) {
                fprintf(fp, "from stdz import *\n", META.uses[i]);
                has_import = true;
            }
        }
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

static void use_charts() {
    global_set("pie", new_stdfn("pie"));
}

void trans_py(char *file) {
    log_trace("Transpiling %s to Python\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Parser *parser = new_parser(code);
    make_builtins(global_scope());
    use_stdz(global_scope());
    // TODO: temp until support for use lib
    use_charts();
    Node *prog = parse(parser);
    // 输出Python代码
    codegen_py(prog);
}

// 将AST编译成JS代码
static void codegen_js(Node *prog) {
    META.lan = LAN_JS;
    Node *expr = prog->as.exprs.list[0];
    // 打开输出文件
    FILE *fp = fopen("app.js", "w");
    // 第一道收集信息，顺便打出import语句
    bool has_import = false;
    char *name_in_use = "";
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_USE) {
            char *name = expr->as.use.name;
            if (strcmp(name, name_in_use) != 0) {
                fprintf(fp, "import {%s} from \"./%s\"\n", expr->as.use.name, expr->as.use.box);
                has_import = true;
            }
            name_in_use = name;
        } else if (expr->kind == ND_CALL) {
            char *name = expr->as.call.name->as.str;
            // 注意，print直接替换为console.log即可
            if (strcmp(name, "print") == 0) {
                expr->as.call.name->as.str = "console.log";
            } else if (strcmp(name, name_in_use) != 0) {
                fprintf(fp, "import {%s} from \"./stdz.js\"\n", name);
                has_import = true;
            }
        }
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

static void use_js_stdz() {
    global_set("alert", new_stdfn("alert"));
}

void trans_js(char *file) {
    log_trace("Transpiling %s to JS\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Parser *parser = new_parser(code);
    make_builtins(global_scope());
    use_stdz(global_scope());
    use_js_stdz();
    Node *prog = parse(parser);
    // 输出JS代码
    codegen_js(prog);
}
