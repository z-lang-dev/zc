#include <stdio.h>
#include "zast.h"

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
        fprintf(fp, "{kind: ND_INT, as.num: %I64d}", node->as.num);
    #else
        fprintf(fp, "{kind: ND_INT, as.num: %lld}", node->as.num);
    #endif
        break;
    case ND_STR:
        fprintf(fp, "{kind: ND_STR, as.str: \"%s\"}", node->as.str);
        break;
    case ND_FNAME:
        fprintf(fp, "{kind: ND_FNAME, as.str: %s}", node->as.str);
        break;
    }
}

void print_node(Node *node) {
    fprint_node(stdout, node);
}

void trace_node(Node *node) {
#ifdef LOG_TRACE
    printf("----- NODE ---- \n");
    print_node(node);
    printf("\n");
    printf("----- END ---- \n");
#endif
}