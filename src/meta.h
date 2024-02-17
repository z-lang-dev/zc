#pragma once
#include <stdbool.h>
#include "zast.h"
#include "hash.h"
#include "types.h"

static const int SIZE_INT = 4;
typedef struct Meta Meta;
typedef struct Scope Scope;

typedef enum {
    MT_NAME,
    MT_PATH,
    MT_FN,
    MT_BLOCK,
    MT_USE,
    MT_TYPE,
    MT_ILL,
} MetaKind;

// TODO: 把Meta改造成tagged-union
struct Meta {
    MetaKind kind;
    Node *node;
    Type *type;
    char *name;
    int seq;
    int offset;
    bool need_return; // for block
    bool is_def; // self defined function
};

struct Scope {
    HashTable *metas; // 元信息表
    HashTable *values; // 运行时存值表
    Scope *parent;  // 父节点
    int cur_seq;
    int cur_offset;
};

Scope *GlobalScope; // 全局视野

Meta *new_meta(Node *expr);
Meta *new_type_meta(Type *type);

Scope *new_scope(Scope *parent);
Meta *scope_lookup(Scope *scope, const char *name);
bool scope_set(Scope *scope, const char *name, Meta *meta);

Meta *global_lookup(const char *name);
bool global_set(const char *name, Meta *meta);
Scope *global_scope();
