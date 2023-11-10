#include <stdio.h>
#include "zast.h"

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
    case ND_CALL:
        fecho_node(fp, node->as.call.fname);
        fprintf(fp, "(");
        fprint_node(fp, node->as.call.arg);
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
    case ND_CALL:
        fprintf(fp, "{ kind:NT_CALL, fname: ");
        fprint_node(fp, node->as.call.fname);
        fprintf(fp, ", arg: ");
        fprint_node(fp, node->as.call.arg);
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