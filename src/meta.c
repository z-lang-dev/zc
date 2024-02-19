#include <stdlib.h>
#include <stdio.h>
#include "meta.h"

Meta *new_meta(Node *expr) {
    Meta *meta = calloc(1, sizeof(Meta));
    meta->node = expr;
    switch (expr->kind) {
        case ND_NAME:
            meta->kind = MT_NAME;
            meta->name = get_name(expr);
            break;
        case ND_PATH:
            meta->kind = MT_PATH;
            meta->name = get_name(expr);
            break;
        case ND_FN:
            meta->kind = MT_FN;
            meta->name = expr->as.fn.name;
            break;
        case ND_BLOCK:
            meta->kind = MT_BLOCK;
            break;
        case ND_USE:
            meta->kind = MT_USE;
        case ND_BINOP:
            meta->kind = MT_BINOP;
            break;
        default:
            meta->kind = MT_ILL;
            break;
    }
    return meta;
}

Meta *new_type_meta(Type *type) {
    Meta *m = calloc(1, sizeof(Meta));
    m->type = type;
    m->kind = MT_TYPE;
    return m;
}

int total_meta_size() {
    return GlobalScope->cur_offset;
}

Scope *new_scope(Scope *parent) {
    Scope *scope = calloc(1, sizeof(Scope));
    scope->parent = parent;
    scope->metas = new_hash_table();
    scope->values = new_hash_table();
    scope->cur_seq = 0;
    scope->cur_offset = 0;
    return scope;
}

Meta *scope_lookup(Scope *scope, const char *name) {
    Meta *meta = hash_get(scope->metas, name);
    if (meta) return meta;
    if (scope->parent) return scope_lookup(scope->parent, name);
    return NULL;
}

bool scope_set(Scope *scope, const char *name, Meta *meta) {
    // 进行统计
    if (meta->kind == MT_NAME || meta->kind == MT_PATH) { 
        int size = SIZE_INT; // 现在只有int类型，它的尺寸是4字节
        meta->offset = scope->cur_offset;
        scope->cur_offset += size;
        meta->seq = scope->cur_seq++;
    }
    hash_set(scope->metas, name, meta);
    return true;
}


Meta *global_lookup(const char *name) {
    return scope_lookup(GlobalScope, name);
}

Scope *global_scope() {
    if (GlobalScope == NULL) {
        GlobalScope = new_scope(NULL);
    }
    return GlobalScope;
}

bool global_set(const char *name, Meta *meta) {
    return scope_set(global_scope(), name, meta);
}