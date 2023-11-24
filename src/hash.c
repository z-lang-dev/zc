#include <stdlib.h>
#include "hash.h"


HashTable *new_hash_table() {
    HashTable *hash = calloc(1, sizeof(HashTable));
    hash->cap = DEFAULT_HASH_CAP;
    hash->size = 0;
    hash->entries = calloc(hash->cap, sizeof(Entry));
    return hash;
}

/**
 * @brief 计算哈希值。
 *
 * @param key 需要计算哈希值的字符串。
 * @return int型的哈希值。
 */
static int hash_code(char *key) {
    int h = 0;
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
static int hash_idx(HashTable *hash, char *key) {
    int h = hash_code(key);
    return h % hash->cap;
}


bool hash_has(HashTable *hash, char *key) {
    int idx = hash_idx(hash, key);
    return hash->entries[idx] != NULL && strcmp(hash->entries[idx]->key, key) == 0;
}

void hash_set(HashTable *hash, char *key, int value) {
    // 如果size/cap超过LOAD_FACTOR，就扩容一倍
    if ((double) hash->size / (double) hash->cap > LOAD_FACTOR) {
        hash->cap *= 2;
        hash->entries = realloc(hash->entries, hash->cap * sizeof(Entry));
    }

    int idx = hash_idx(hash, key);
    if (hash->entries[idx] != NULL) {
        Entry *ent = hash->entries[idx];
        char *exist_key = ent->key;
        if (strcmp(exist_key, key) == 0) { 
            // 如果key相同，说明找到目标了，直接更新值
            ent->value = value;
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
    hash->entries[idx] = calloc(1, sizeof(Entry));
    Entry *ent = hash->entries[idx];
    ent->key = key;
    ent->value = value;
}

int hash_get(HashTable *hash, char *key) {
    int idx = hash_idx(hash, key);
    Entry *ent = hash->entries[idx];
    return ent->value;
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
            ent->value = value;
            return;
        }
    }

    // 没找到，就新建一项
    Entry *ent = calloc(1, sizeof(Entry));
    ent->key = key;
    ent->value = value;

    // 如果已经满了，就扩容
    if (arr->size + 1 >= arr->cap) {
        arr->cap *= 2;
        arr->entries = realloc(arr->entries, arr->cap * sizeof(Entry));
    }

    // 写入数组
    arr->entries[arr->size++] = ent;
}

int array_get(ValueArray *arr, char *key) {
    for (int i = 0; i < arr->size; i++) {
        Entry *ent = arr->entries[i];
        if (strcmp(ent->key, key) == 0) {
            return ent->value;
        }
    }
    return 0;
}