#include <stdlib.h>
#include "meta.h"

static HashTable *META_TABLE;
static int cur_seq = 0;
static int cur_offset = 0;

void init_meta() {
    META_TABLE = new_hash_table();
}

Meta *new_meta(Node *expr, MetaKind kind) {
    Meta *meta = calloc(1, sizeof(Meta));
    meta->kind = kind;
    meta->node = expr;
    return meta;
}

void set_meta(Meta *meta) {
    // 进行统计
    int size = SIZE_INT; // 现在只有int类型，它的尺寸是4字节
    meta->offset = cur_offset;
    cur_offset += size;
    meta->seq = cur_seq++;

    // 存入哈希表
    hash_set(META_TABLE, meta->name, meta);
}

Meta *get_meta(char *name) {
    return hash_get(META_TABLE, name);
}

int total_meta_size() {
    return cur_offset;
}

HashTable *get_meta_table() {
    return META_TABLE;
}