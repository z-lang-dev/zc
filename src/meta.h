#pragma once
#include <stdbool.h>
#include "zast.h"
#include "hash.h"

static const int SIZE_INT = 4;
typedef struct Meta Meta;
typedef struct Scope Scope;

typedef enum {
    MT_LET,
    MT_MUT,
    MT_FN,
    MT_BLOCK,
    MT_USE,
    MT_ILL,
} MetaKind;

// TODO: 把Meta改造成tagged-union
struct Meta {
    MetaKind kind;
    Node *node;
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

/**
 * @brief Sets the meta information for the given object.
 *
 * This function sets the meta information for the object pointed to by the `meta` parameter.
 *
 * @param meta A pointer to the Meta object.
 */
// void set_meta(Meta *meta);

/**
 * Retrieves the Meta object associated with the given name.
 *
 * @param name The name of the Meta object to retrieve.
 * @return A pointer to the Meta object, or nullptr if not found.
 */
// Meta *get_meta(char *name);

// int total_meta_size();
// HashTable *get_meta_table();

Scope *new_scope(Scope *parent);

Meta *scope_lookup(Scope *scope, const char *name);
bool scope_set(Scope *scope, const char *name, Meta *meta);

Meta *global_lookup(const char *name);
bool global_set(const char *name, Meta *meta);

Scope *global_scope();
