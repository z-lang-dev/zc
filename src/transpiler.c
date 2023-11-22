#include <stdio.h>
#include <stdbool.h>
#include "transpiler.h"
#include "parser.h"
#include "util.h"

#define MAX_USES 100
typedef struct TransMeta TransMeta;
struct TransMeta {
    int use_count;
    char *uses[MAX_USES];

    bool is_c;
};

static TransMeta META;

static void do_meta(Node *prog) {
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_CALL) {
            char *fname = expr->as.call.fname->as.str;
            if (strcmp(fname, "print") == 0) {
                META.uses[META.use_count++] = "<stdio.h>";
            } else {
                META.uses[META.use_count++] = "\"stdz.h\"";
            }
        }
    }
}

static void gen_expr(FILE *fp, Node *expr) {
    switch (expr->kind) {
    case ND_INT:
        fprintf(fp, "%d", expr->as.num);
        return;
    case ND_NEG:
        fprintf(fp, "-(");
        gen_expr(fp, expr->as.una.body);
        fprintf(fp, ")");
        return;
    case ND_CALL:
        if (META.is_c && strcmp(expr->as.call.fname->as.str, "print") == 0) {
            Node *val = expr->as.call.args[0];
            if (val->kind == ND_INT) {
                fprintf(fp, "    printf(\"%%d\\n\", %d)", val->as.num);
            } else {
                fprintf(fp, "    printf(\"%s\\n\")", val->as.str);
            }
            return;
        } else {
            char *tab = META.is_c ? "    " : "";
            fprintf(fp, "%s%s(", tab, expr->as.call.fname->as.str);
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
    // 右臂，现在的语境下，右臂只可能是整数，因此可以当做立即数直接参与计算
    if (expr->as.bop.right->kind == ND_INT) {
        int right = expr->as.bop.right->as.num;
        // 具体的二元运算
        switch (expr->as.bop.op) {
        case OP_ADD:
            fprintf(fp, " + %d", right);
            break;
        case OP_SUB:
            fprintf(fp, " - %d", right);
            break;
        case OP_MUL:
            fprintf(fp, " * %d", right);
            break;
        case OP_DIV:
            fprintf(fp, " / %d", right);
            break;
        default:
            printf("Error: unknown operator for binop expr: %d\n", expr->as.bop.op);
        }
    } else {
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
        default:
            printf("Error: unknown operator for binop expr: %d\n", expr->as.bop.op);
        }
        gen_expr(fp, expr->as.bop.right);
    }
}

static Node *last_expr(Node *prog) {
    return prog->as.exprs.list[prog->as.exprs.count - 1];
}

// 将AST编译成C代码
static void codegen_c(Node *prog) {
    do_meta(prog);
    META.is_c = true;
    
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
            gen_expr(fp, expr);
            fprintf(fp, ";\n"); // C语句需要`;`结尾
        }
    }

    // 最后一条语句需要处理`return`
    Node *last = last_expr(prog);
    if (last->kind == ND_CALL) {
        gen_expr(fp, last);
        fprintf(fp, ";\n");
        fprintf(fp, "    return 0;\n");
    } else {
        fprintf(fp, "    return ");
        gen_expr(fp, last);
        fprintf(fp, ";\n");
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
    Node *prog = parse(parser);
    trace_node(prog);
    // 输出C代码
    codegen_c(prog);
}

// 将AST编译成Python代码
static void codegen_py(Node *prog) {
    // 打开输出文件
    FILE *fp = fopen("app.py", "w");
    bool has_import = false;
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_CALL) {
            char *fname = expr->as.call.fname->as.str;
            if (strcmp(fname, "print") != 0) {
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
        gen_expr(fp, expr);
        fprintf(fp, "\n");
    }   
    // 保存并关闭文件
    fclose(fp);
}

void trans_py(char *file) {
    log_trace("Transpiling %s to Python\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Parser *parser = new_parser(code);
    Node *prog = parse(parser);
    // 输出Python代码
    codegen_py(prog);
}

// 将AST编译成JS代码
static void codegen_js(Node *prog) {
    Node *expr = prog->as.exprs.list[0];
    // 打开输出文件
    FILE *fp = fopen("app.js", "w");
    // 第一道收集信息，顺便打出import语句
    bool has_import = false;
    for (int i = 0; i < prog->as.exprs.count; ++i) {
        Node *expr = prog->as.exprs.list[i];
        if (expr->kind == ND_CALL) {
            char *fname = expr->as.call.fname->as.str;
            // 注意，print直接替换为console.log即可
            if (strcmp(fname, "print") == 0) {
                expr->as.call.fname->as.str = "console.log";
            } else {
                fprintf(fp, "import {%s} from \"./stdz.js\"\n", fname);
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
        gen_expr(fp, expr);
        fprintf(fp, "\n");
    }
    
    // 保存并关闭文件
    fclose(fp);
}

void trans_js(char *file) {
    log_trace("Transpiling %s to JS\n", file);
    // 读取源码文件内容
    char *code = read_src(file);
    // 解析出AST
    Parser *parser = new_parser(code);
    Node *prog = parse(parser);
    // 输出JS代码
    codegen_js(prog);
}
