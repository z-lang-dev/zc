#include <stdio.h>
#include <stdlib.h>
#include "zast.h"
#include "type.h"
#include "meta.h"

void fecho_node(FILE *fp, Node *node) {
    switch (node->kind) {
    case ND_PROG:
        for (int i = 0; i < node->as.exprs.count; i++) {
            if (i > 0) {
                fprintf(fp, "\n");
            }
            fecho_node(fp, node->as.exprs.list[i]);
        }
        break;
    case ND_BLOCK:
        fprintf(fp, "{");
        for (int i = 0; i < node->as.exprs.count; i++) {
            if (i > 0) {
                fprintf(fp, "\n");
            }
            fecho_node(fp, node->as.exprs.list[i]);
        }
        fprintf(fp, "}");
        break;
    case ND_ARRAY: {
        fprintf(fp, "[");
        for (int i = 0; i < node->as.array.size; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            fecho_node(fp, node->as.array.items[i]);
        }
        fprintf(fp, "]");
        break;
    }
    case ND_INDEX: {
        fecho_node(fp, node->as.index.parent);
        fprintf(fp, "[");
        fecho_node(fp, node->as.index.idx);
        fprintf(fp, "]");
        break;
    }
    case ND_USE:
        fprintf(fp, "use %s", node->as.use.mod);
        if (node->as.use.name != NULL) {
            fprintf(fp, ".%s", node->as.use.name);
        }
        break;
    case ND_LET: {
        Type *type = node->as.asn.name->meta->type;
        fprintf(fp, "let %s %s = ", node->as.asn.name->as.str, type->name);
        fecho_node(fp, node->as.asn.value);
        break;
    }
    case ND_MUT: {
        Type *type = node->as.asn.name->meta->type;
        fprintf(fp, "mut %s %s = ", node->as.asn.name->as.str, type->name);
        fecho_node(fp, node->as.asn.value);
        break;
    }
    case ND_ASN:
        fecho_node(fp, node->as.asn.name);
        fprintf(fp, " = ");
        fecho_node(fp, node->as.asn.value);
        break;
    case ND_IF:
        fprintf(fp, "if ");
        fecho_node(fp, node->as.if_else.cond);
        fprintf(fp, " {");
        fecho_node(fp, node->as.if_else.then);
        fprintf(fp, "} else {");
        fecho_node(fp, node->as.if_else.els);
        fprintf(fp, "}");
        break;
    case ND_FOR:
        fprintf(fp, "for ");
        fecho_node(fp, node->as.loop.cond);
        fprintf(fp, " {");
        fecho_node(fp, node->as.loop.body);
        fprintf(fp, "}");
        break;
    case ND_FN:
        fprintf(fp, "fn %s(", node->as.fn.name);
        for (int i = 0; i < node->as.fn.params->count; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            Node *p = node->as.fn.params->list[i];
            fecho_node(fp, p);
            if (p->meta->type != NULL) {
                fprintf(fp, " %s", p->meta->type->name);
            }
        }
        fprintf(fp, ") {");
        fecho_node(fp, node->as.fn.body);
        fprintf(fp, "}");
        break;
    case ND_TYPE:
        fprintf(fp, "typ %s {", node->as.type.name->as.str);
        for (int i = 0; i < node->as.type.fields->size; i++) {
            if (i > 0) {
                fprintf(fp, "; ");
            }
            Node *f = node->as.type.fields->items[i];
            fecho_node(fp, f);
            if (f->meta->type != NULL) {
                fprintf(fp, " %s", f->meta->type->name);
            }
        }
        fprintf(fp, "}");
        break;
    case ND_NEG:
        fprintf(fp, "-");
        fecho_node(fp, node->as.una.body);
        break;
    case ND_CALL:
        fecho_node(fp, node->as.call.name);
        fprintf(fp, "(");
        for (int i = 0; i < node->as.call.argc; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            fecho_node(fp, node->as.call.args[i]);
        }
        fprintf(fp, ")");
        break;
    case ND_NAME:
        fprintf(fp, "%s", node->as.str);
        break;
    case ND_PATH:
        for (int i = 0; i < node->as.path.len; i++) {
            if (i > 0) {
                fprintf(fp, ".");
            }
            fprintf(fp, "%s", node->as.path.names[i].name);
        }
        break;
    case ND_LNAME:
        fprintf(fp, "%s", node->as.str);
        break;
    case ND_INT:
        fprintf(fp, "%s", node->as.num.lit);
        break;
    case ND_FLOAT:
        fprintf(fp, "%s", node->as.float_num.lit);
        break;
    case ND_DOUBLE:
        fprintf(fp, "%s", node->as.double_num.lit);
        break;
    case ND_BOOL:
        fprintf(fp, "%s", node->as.bul ? "true" : "false");
        break;
    case ND_NOT:
        fprintf(fp, "!");
        fecho_node(fp, node->as.una.body);
        break;
    case ND_STR:
        fprintf(fp, "\"%s\"", node->as.str);
        break;
    case ND_BINOP:
        fprintf(fp, "{");
        fecho_node(fp, node->as.bop.left);
        fprintf(fp, " %s ", op_to_str(node->as.bop.op));
        fecho_node(fp, node->as.bop.right);
        fprintf(fp, "}");
        break;
    default:
        fprintf(fp, "Unknown node kind: %d", node->kind);
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
    case ND_BLOCK:
        fprintf(fp, "{kind:ND_BLOCK, exprs: [");
        for (int i = 0; i < node->as.exprs.count; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            fprint_node(fp, node->as.exprs.list[i]);
        }
        fprintf(fp, "]}");
        break;
    case ND_ARRAY: {
        fprintf(fp, "{kind:ND_ARRAY, items: [");
        for (int i = 0; i < node->as.array.size; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            fprint_node(fp, node->as.array.items[i]);
        }
        fprintf(fp, "]}");
        break;
    }
    case ND_INDEX: {
        fprintf(fp, "{kind:ND_INDEX, array: ");
        fprint_node(fp, node->as.index.parent);
        fprintf(fp, ", idx: ");
        fprint_node(fp, node->as.index.idx);
        fprintf(fp, " }");
        break;
    }
    case ND_USE:
        fprintf(fp, "{kind:ND_USE, mod: %s", node->as.use.mod);
        if (node->as.use.name != NULL) {
            fprintf(fp, ", name: %s", node->as.use.name);
        }
        break;
    case ND_LET: {
        Type *type = node->as.asn.name->meta->type;
        fprintf(fp, "{kind:ND_LET, name: %s, type: %s, value: ", node->as.asn.name->as.str, type->name);
        fprint_node(fp, node->as.asn.value);
        fprintf(fp, " }");
        break;
    }
    case ND_MUT: {
        Type *type = node->as.asn.name->meta->type;
        fprintf(fp, "{kind:ND_MUT, name: %s, type: %s, value: ", node->as.asn.name->as.str, type->name);
        fprint_node(fp, node->as.asn.value);
        fprintf(fp, " }");
        break;
    }
    case ND_ASN:
        fprintf(fp, "{kind:ND_ASN, name: ");
        fprint_node(fp, node->as.asn.name);
        fprintf(fp, ", value: ");
        fprint_node(fp, node->as.asn.value);
        fprintf(fp, " }");
        break;
    case ND_IF:
        fprintf(fp, "{kind:ND_IF, cond: ");
        fprint_node(fp, node->as.if_else.cond);
        fprintf(fp, ", then: ");
        fprint_node(fp, node->as.if_else.then);
        fprintf(fp, ", els: ");
        fprint_node(fp, node->as.if_else.els);
        fprintf(fp, " }");
        break;
    case ND_FOR:
        fprintf(fp, "{kind:ND_FOR, cond: ");
        fprint_node(fp, node->as.loop.cond);
        fprintf(fp, ", body: ");
        fprint_node(fp, node->as.loop.body);
        fprintf(fp, " }");
        break;
    case ND_FN: {
        fprintf(fp, "{kind:ND_FN, name: %s, params: [", node->as.fn.name);
        for (int i = 0; i < node->as.fn.params->count; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            Node *p = node->as.fn.params->list[i];
            fprint_node(fp, p);
            Type *ptype = p->meta->type;
            if (ptype != NULL) {
                fprintf(fp, " %s", ptype->name);
            }
        }
        fprintf(fp, "], body: ");
        fprint_node(fp, node->as.fn.body);
        fprintf(fp, " }");
        break;
    }
    case ND_TYPE: {
        fprintf(fp, "{kind:ND_TYPE, name: %s, fields: [", node->as.type.name->as.str);
        for (int i = 0; i < node->as.type.fields->size; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            Node *f = node->as.type.fields->items[i];
            fprint_node(fp, f);
            Type *ftype = f->meta->type;
            if (ftype != NULL) {
                fprintf(fp, " %s", ftype->name);
            }
        }
        fprintf(fp, "]}");
        break;
    }
    case ND_NEG:
        fprintf(fp, "{kind:ND_NEG, body: ");
        fprint_node(fp, node->as.una.body);
        fprintf(fp, " }");
        break;
    case ND_CALL:
        fprintf(fp, "{ kind:NT_CALL, name: ");
        fprint_node(fp, node->as.call.name);
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
        fprintf(fp, "{kind: ND_INT, as.num: %s}", node->as.num.lit);
        break;
    case ND_BOOL:
        fprintf(fp, "%s", node->as.bul ? "true" : "false");
        break;
    case ND_FLOAT:
        fprintf(fp, "{kind: ND_FLOAT, as.float_num: %s}", node->as.float_num.lit);
        break;
    case ND_DOUBLE:
        fprintf(fp, "{kind: ND_DOUBLE, as.double_num: %s}", node->as.double_num.lit);
        break;
    case ND_NOT:
        fprintf(fp, "{kind: ND_NOT, body: ");
        fprint_node(fp, node->as.una.body);
        fprintf(fp, " }");
        break;
    case ND_STR:
        fprintf(fp, "{kind: ND_STR, as.str: \"%s\"}", node->as.str);
        break;
    case ND_LNAME:
        fprintf(fp, "{kind: ND_LNAME, as.str: %s}", node->as.str);
        break;
    case ND_NAME:
        fprintf(fp, "{kind: ND_NAME, as.str: %s}", node->as.str);
        break;
    case ND_PATH:
        fprintf(fp, "{kind: ND_PATH, as.str: ");
        for (int i = 0; i < node->as.path.len; i++) {
            if (i > 0) {
                fprintf(fp, ".");
            }
            fprintf(fp, "%s", node->as.path.names[i].name);
        }
        fprintf(fp, "}");
        break;
    case ND_BINOP:
        fprintf(fp, "{kind: ND_BINOP, op: %s", op_to_str(node->as.bop.op));
        fprintf(fp, ", left: ");
        fprint_node(fp, node->as.bop.left);
        fprintf(fp, ", right: ");
        fprint_node(fp, node->as.bop.right);
        fprintf(fp, " }");
        break;
    default:
        fprintf(fp, "Unknown node kind: %d", node->kind);
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

Node *new_block() {
    Node *prog = calloc(1, sizeof(Node));
    prog->kind = ND_BLOCK;
    prog->as.exprs.count = 0;
    prog->as.exprs.cap = 1;
    prog->as.exprs.list = calloc(1, sizeof(Node *));
    return prog;
}

Node *new_array() {
    Node *array = calloc(1, sizeof(Node));
    array->kind = ND_ARRAY;
    array->as.array.size = 0;
    array->as.array.cap = 1;
    array->as.array.items = calloc(1, sizeof(Node *));
    return array;
}

Node *new_prog() {
    Node *prog = calloc(1, sizeof(Node));
    prog->kind = ND_PROG;
    prog->as.exprs.count = 0;
    prog->as.exprs.cap = 1;
    prog->as.exprs.list = calloc(1, sizeof(Node *));
    return prog;
}

void list_append(List *list, Node *node) {
    if (list->size >= list->cap) { // grow if needed
        if (list->cap <= 0) list->cap = 1;
        else list->cap *= 2;
        list->items = realloc(list->items, list->cap * sizeof(Node *));
    }
    list->items[list->size++] = node;
}

void append_array_item(Node *parent, Node *node) {
    Array *array = &parent->as.array;
    if (array->size >= array->cap) { // grow if needed
        if (array->cap <= 0) array->cap = 1;
        else array->cap *= 2;
        array->items = realloc(array->items, array->cap * sizeof(Node *));
    }
    array->items[array->size++] = node;
}

void append_expr(Node *parent, Node *node) {
    Exprs *exprs = &parent->as.exprs;
    if (exprs->count >= exprs->cap) { // grow if needed
        if (exprs->cap <= 0) exprs->cap = 1;
        else exprs->cap *= 2;
        exprs->list = realloc(exprs->list, exprs->cap * sizeof(Node *));
    }
    exprs->list[exprs->count++] = node;
}


char *op_to_str(Op op) {
    switch (op) {
    case OP_ADD:
        return "+";
    case OP_SUB:
        return "-";
    case OP_MUL:
        return "*";
    case OP_DIV:
        return "/";
    case OP_GT:
        return ">";
    case OP_LT:
        return "<";
    case OP_GE:
        return ">=";
    case OP_LE:
        return "<=";
    case OP_EQ:
        return "==";
    case OP_NE:
        return "!=";
    case OP_AND:
        return "&&";
    case OP_OR:
        return "||";
    case OP_NOT:
        return "!";
    case OP_ILL:
        return "<ILL_OP>";
    case OP_ASN:
        return "=";
    default:
        return "<UNKNOWN_OP>";
    }
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

char *get_name(Node *name) {
    switch (name->kind) {
    case ND_NAME:
        return name->as.str;
    case ND_PATH:
        return name->as.path.names[name->as.path.len - 1].name;
    default:
        return "<UNKNOWN_NAME_TYPE>";
    }
}