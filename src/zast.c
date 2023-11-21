#include <stdio.h>
#include "zast.h"
#include <stdlib.h>

static const char *op_to_str(Op op) {
    switch (op) {
    case OP_ADD:
        return "+";
    case OP_SUB:
        return "-";
    case OP_MUL:
        return "*";
    case OP_DIV:
        return "/";
    }
}

void fecho_node(FILE *fp, Node *node) {
    switch (node->kind) {
    case ND_PROG:
        for (int i = 0; i < node->as.exprs.count; i++) {
            if (i > 0) {
                fprintf(fp, "\n");
            }
            fecho_node(fp, node->as.exprs.list[i]);
        }
    case ND_NEG:
        fprintf(fp, "-");
        fecho_node(fp, node->as.una.body);
        break;
    case ND_CALL:
        fecho_node(fp, node->as.call.fname);
        fprintf(fp, "(");
        for (int i = 0; i < node->as.call.argc; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            fecho_node(fp, node->as.call.args[i]);
        }
        fprintf(fp, ")");
        break;
    case ND_INT:
    #ifdef _WIN32
        fprintf(fp, "%d", node->as.num);
    #else
        fprintf(fp, "%d", node->as.num);
    #endif
        break;
    case ND_STR:
        fprintf(fp, "\"%s\"", node->as.str);
        break;
    case ND_FNAME:
        fprintf(fp, "%s", node->as.str);
        break;
    case ND_BINOP:
        fprintf(fp, "{");
        fecho_node(fp, node->as.bop.left);
        fprintf(fp, " %s ", op_to_str(node->as.bop.op));
        fecho_node(fp, node->as.bop.right);
        fprintf(fp, "}");
        break;
    }
}

void fprint_node(FILE *fp, Node *node) {
    switch (node->kind) {
    case ND_PROG:
        fprintf(fp, "{kind:ND_PROG, exprs: [");
        for (int i = 0; i < node->as.exprs.count; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            fprint_node(fp, node->as.exprs.list[i]);
        }
        fprintf(fp, "]}");
        break;
    case ND_NEG:
        fprintf(fp, "{kind:ND_NEG, body: ");
        fprint_node(fp, node->as.una.body);
        fprintf(fp, " }");
        break;
    case ND_CALL:
        fprintf(fp, "{ kind:NT_CALL, fname: ");
        fprint_node(fp, node->as.call.fname);
        fprintf(fp, ", args: ");
        for (int i = 0; i < node->as.call.argc; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            fprint_node(fp, node->as.call.args[i]);
        }
        fprintf(fp, " }");
        break;
    case ND_INT:
    #ifdef _WIN32
        fprintf(fp, "{kind: ND_INT, as.num: %d}", node->as.num);
    #else
        fprintf(fp, "{kind: ND_INT, as.num: %d}", node->as.num);
    #endif
        break;
    case ND_STR:
        fprintf(fp, "{kind: ND_STR, as.str: \"%s\"}", node->as.str);
        break;
    case ND_FNAME:
        fprintf(fp, "{kind: ND_FNAME, as.str: %s}", node->as.str);
        break;
    case ND_BINOP:
        fprintf(fp, "{kind: ND_BINOP, op: %s", op_to_str(node->as.bop.op));
        fprintf(fp, ", left: ");
        fprint_node(fp, node->as.bop.left);
        fprintf(fp, ", right: ");
        fprint_node(fp, node->as.bop.right);
        fprintf(fp, " }");
        break;
    }
}

void print_node(Node *node) {
    fprint_node(stdout, node);
    fflush(stdout);
}

void echo_node(Node *node) {
#ifdef LOG_TRACE
    printf("> node: ");
    fecho_node(stdout, node);
    printf("\n");
#endif
}

void trace_node(Node *node) {
#ifdef LOG_TRACE
    printf("----- NODE ---- \n");
    print_node(node);
    printf("\n");
    printf("----- END ---- \n");
#endif
}

Node *new_prog() {
    Node *prog = calloc(1, sizeof(Node));
    prog->kind = ND_PROG;
    prog->as.exprs.count = 0;
    prog->as.exprs.cap = 1;
    prog->as.exprs.list = calloc(1, sizeof(Node *));
    return prog;
}

void append_expr(Node *prog, Node *node) {
    Exprs *exprs = &prog->as.exprs;
    if (exprs->count >= exprs->cap) { // grow if needed
        if (exprs->cap <= 0) exprs->cap = 1;
        else exprs->cap *= 2;
        exprs->list = realloc(exprs->list, exprs->cap * sizeof(Node *));
    }
    exprs->list[exprs->count++] = node;
}

/*
void append_expr(Exprs *exprs, Node *node) {
    if (exprs->count >= exprs->cap) { // grow if needed
        if (exprs->cap <= 0) exprs->cap = 1;
        else exprs->cap *= 2;
        exprs->exprs = realloc(exprs->exprs, exprs->cap * sizeof(Node *));
    }
    exprs->exprs[exprs->count++] = node;
}*/