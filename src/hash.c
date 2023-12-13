#include <stdlib.h>
#include "hash.h"


HashTable *new_hash_table() {
    HashTable *hash = calloc(1, sizeof(HashTable));
    hash->cap = DEFAULT_HASH_CAP;
    hash->size = 0;
    hash->entries = calloc(hash->cap, sizeof(Entry*));
    return hash;
}

/**
 * @brief 计算哈希值。
 *
 * @param key 需要计算哈希值的字符串。
 * @return int型的哈希值。
 */
static size_t hash_code(char *key) {
    size_t h = 0;
    while (*key != '\0') {
        h = h * 31 + *key;
        key++;
    }
    return h;
}

/**
 * @brief 计算key在哈希表中的索引
 *
 * @param hash 哈希表
 * @param key 键值
 * @return int型的索引
 */
static size_t hash_idx(HashTable *hash, char *key) {
    size_t h = hash_code(key);
    return h % hash->cap;
}

bool hash_has(HashTable *hash, char *key) {
    size_t idx = hash_idx(hash, key);
    return hash->entries[idx] != NULL && strcmp(hash->entries[idx]->key, key) == 0;
}

void hash_set_int(HashTable *hash, char *key, int value) {
    // 如果size/cap超过LOAD_FACTOR，就扩容一倍
    if ((double) hash->size / (double) hash->cap > LOAD_FACTOR) {
        hash->cap *= 2;
        hash->entries = realloc(hash->entries, hash->cap * sizeof(IntEntry));
    }

    size_t idx = hash_idx(hash, key);
    if (hash->entries[idx] != NULL) {
        Entry *ent = hash->entries[idx];
        char *exist_key = ent->key;
        if (strcmp(exist_key, key) == 0) { 
            // 如果key相同，说明找到目标了，直接更新值
            ((IntEntry*)ent)->value = value;
            return;
        } else {
            // 冲突了，寻找下一个空位
            idx = (idx + 1) % hash->cap;
            while (hash->entries[idx] != NULL) {
                idx = (idx + 1) % hash->cap;
            }
            // 注意，这里没有处理找了一圈儿没找到的情况，因为扩容的问题另外处理
            // 所以假设循环结束时，总会找到一个空位
        }
    }
    // 找到了空位，新建一项并写入
    hash->entries[idx] = calloc(1, sizeof(IntEntry));
    Entry *ent = hash->entries[idx];
    ent->key = key;
    ((IntEntry*)ent)->value = value;
}

void hash_set(HashTable *hash, char *key, void *value) {
    // 如果size/cap超过LOAD_FACTOR，就扩容一倍
    if ((double) hash->size / (double) hash->cap > LOAD_FACTOR) {
        hash->cap *= 2;
        hash->entries = realloc(hash->entries, hash->cap * sizeof(ObjEntry));
    }

    size_t idx = hash_idx(hash, key);
    if (hash->entries[idx] != NULL) {
        Entry *ent = hash->entries[idx];
        char *exist_key = ent->key;
        if (strcmp(exist_key, key) == 0) { 
            // 如果key相同，说明找到目标了，直接更新值
            ((ObjEntry*)ent)->value = value;
            return;
        } else {
            // 冲突了，寻找下一个空位
            idx = (idx + 1) % hash->cap;
            while (hash->entries[idx] != NULL) {
                idx = (idx + 1) % hash->cap;
            }
            // 注意，这里没有处理找了一圈儿没找到的情况，因为扩容的问题另外处理
            // 所以假设循环结束时，总会找到一个空位
        }
    }
    // 找到了空位，新建一项并写入
    hash->entries[idx] = calloc(1, sizeof(ObjEntry));
    hash->entries[idx]->key = key;
    ((ObjEntry*)hash->entries[idx])->value = value;
}

int hash_get_int(HashTable *hash, char *key) {
    size_t idx = hash_idx(hash, key);
    Entry *ent = hash->entries[idx];
    if (ent == NULL) return 0;
    if (strcmp(ent->key, key) != 0) return 0;
    return ((IntEntry*)ent)->value;
}

void *hash_get(HashTable *hash, char *key) {
    size_t idx = hash_idx(hash, key);
    size_t i = idx;
    while (hash->entries[i] != NULL) {
        Entry *ent = hash->entries[i];
        if (strcmp(ent->key, key) == 0) {
            return ((ObjEntry*)ent)->value;
        }
        i = (i + 1) % hash->cap;
        if (i == idx) return NULL; // 找了一整圈都没找到。
    }
    return NULL;
}

ValueArray *new_value_array() {
    ValueArray *arr = calloc(1, sizeof(ValueArray));
    arr->cap = DEFAULT_ARRAY_CAP;
    arr->size = 0;
    arr->entries = calloc(arr->cap, sizeof(Entry));
    return arr;
}

bool array_has(ValueArray *arr, char *key) {
    for (int i = 0; i < arr->size; i++) {
        Entry *ent = arr->entries[i];
        if (strcmp(ent->key, key) == 0) {
            return true;
        }
    }
    return false;
}

void array_set(ValueArray *arr, char *key, int value) {
    for (int i = 0; i < arr->size; i++) {
        Entry *ent = arr->entries[i];
        if (strcmp(ent->key, key) == 0) {
            ((IntEntry*)ent)->value = value;
            return;
        }
    }

    // 没找到，就新建一项
    Entry *ent = calloc(1, sizeof(IntEntry));
    ent->key = key;
    ((IntEntry*)ent)->value = value;

    // 如果已经满了，就扩容
    if (arr->size + 1 >= arr->cap) {
        arr->cap *= 2;
        arr->entries = realloc(arr->entries, arr->cap * sizeof(IntEntry));
    }

    // 写入数组
    arr->entries[arr->size++] = ent;
}

int array_get(ValueArray *arr, char *key) {
    for (int i = 0; i < arr->size; i++) {
        Entry *ent = arr->entries[i];
        if (strcmp(ent->key, key) == 0) {
            return ((IntEntry*)ent)->value;
        }
    }
    return 0;
}

HashIter *hash_iter(HashTable *table) {
    HashIter *iter = calloc(1, sizeof(HashIter));
    iter->idx = 0;
    iter->key = NULL;
    iter->value = NULL;
    return iter;
}

bool hash_next(HashTable *table, HashIter *iter) {
    for (int i = iter->idx; i < table->cap; i++) {
        Entry *ent = table->entries[i];
        if (ent == NULL) continue;
        if (ent->key == NULL) continue;
        iter->idx = i+1;
        iter->key = ent->key;
        iter->value = ((ObjEntry*)ent)->value;
        return true;
    }
    return false;
}