#pragma once
#include <stdbool.h>
#include "zast.h"
#include "hash.h"
#include "type.h"

static const int SIZE_INT = 4;
typedef struct Meta Meta;
typedef struct Scope Scope;
typedef struct MethodScope MethodScope;
typedef struct BlockScope BlockScope;
typedef struct RuntimeScope RuntimeScope;

// TODO: 把Meta改造成tagged-union
struct Meta {
    NodeKind kind;
    Node *node;
    Type *type;
    char *name;
    int seq;
    int offset;
    bool need_return; // for block
    bool is_def; // self defined function
};

typedef enum {
    // TODO: SC_FOR
    SC_BLOCK,
    SC_METHOD,
    SC_Runtime,
} ScopeKind;

struct MethodScope {
    Type *type;
};

struct BlockScope {
    HashTable *metas;
};

struct RuntimeScope {
    HashTable *values;
};

struct Scope {
    ScopeKind kind;
    Scope *parent;  // 父节点
    union {
        MethodScope *method;
        BlockScope *block;
        RuntimeScope *runtime;
    } as;
    // TODO: 挪到对应的Scope类型中
    int cur_seq;
    int cur_offset;
};

Scope *GlobalScope; // 全局视野

Meta *new_meta(Node *expr);
Meta *new_type_meta(Type *type);

Scope *make_scope(ScopeKind kind, Scope *parent);
Scope *new_scope(Scope *parent);
Meta *scope_lookup(Scope *scope, const char *name);
bool scope_set(Scope *scope, const char *name, Meta *meta);

Meta *global_lookup(const char *name);
bool global_set(const char *name, Meta *meta);
Scope *global_scope();
