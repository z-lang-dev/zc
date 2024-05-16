#include <stdlib.h>
#include <stdio.h>
#include "meta.h"

Meta *new_meta(Node *expr) {
    Meta *meta = calloc(1, sizeof(Meta));
    meta->node = expr;
    meta->kind = expr->kind;
    return meta;
}

Meta *new_type_meta(Type *type) {
    Meta *m = calloc(1, sizeof(Meta));
    m->type = type;
    m->kind = ND_TYPE;
    return m;
}

int total_meta_size() {
    return GlobalScope->cur_offset;
}

Scope *make_scope(ScopeKind kind, Scope *parent) {
    Scope *scope = calloc(1, sizeof(Scope));
    scope->kind = kind;
    scope->parent = parent;
    switch (kind) {
        case SC_BLOCK:
            scope->as.block = calloc(1, sizeof(BlockScope));
            scope->as.block->table = new_hash_table();
            break;
        case SC_METHOD:
            scope->as.method = calloc(1, sizeof(MethodScope));
            break;
        case SC_Runtime:
            scope->as.runtime = calloc(1, sizeof(RuntimeScope));
            scope->as.runtime->table = new_hash_table();
            scope->as.runtime->values = new_hash_table();
            break;
    }
    return scope;
}

Scope *init_global_scope(ScopeKind kind) {
    GlobalScope = make_scope(kind, NULL);
    return GlobalScope;
}

Scope *new_scope(Scope *parent) {
    return make_scope(SC_BLOCK, parent);
}

Meta *scope_lookup(Scope *scope, const char *name) {
    Meta *meta = hash_get(scope->as.block->table, name);
    if (meta) return meta;
    if (scope->parent) return scope_lookup(scope->parent, name);
    return NULL;
}

bool scope_set(Scope *scope, const char *name, Meta *meta) {
    // 进行统计
    if (meta->kind == ND_IDENT) {
        int size = SIZE_INT; // 现在只有int类型，它的尺寸是4字节
        meta->offset = scope->cur_offset;
        scope->cur_offset += size;
        meta->seq = scope->cur_seq++;
    }
    hash_set(scope->as.block->table, name, meta);
    return true;
}


Meta *global_lookup(const char *name) {
    return scope_lookup(GlobalScope, name);
}

Scope *global_scope() {
    if (GlobalScope == NULL) {
        // GlobalScope = new_scope(NULL);
        printf("Error: GlobalScope is NULL\n");
        exit(1);
    }
    return GlobalScope;
}

bool global_set(const char *name, Meta *meta) {
    return scope_set(global_scope(), name, meta);
}