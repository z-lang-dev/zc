#pragma once

#include <stdbool.h>

/**
 * @brief 默认的哈希表容量。
 */
#define DEFAULT_HASH_CAP 16
#define LOAD_FACTOR 0.75

/**
 * @brief 哈希表
 */
typedef struct HashTable HashTable;

/**
 * @brief 哈希表中的一项
 */
typedef struct Entry Entry;

/**
 * @brief 哈希表中的一项
 */
struct Entry {
    char *key;      /**< 用来索引的键 */
    int value;      /**< 需要存储的值 */
};

/**
 * @brief 哈希表
 */
struct HashTable {
    int size;           /**< 当前的大小 */
    int cap;            /**< 容量 */
    Entry **entries;/**< 实际的存储数组 */
};

/**
 * @brief 新建一个哈希表
 * 
 * @return 哈希表的指针
 */
HashTable *new_hash_table();


/**
 * 检查哈希表中是否存在指定的键。
 *
 * @param hash 要搜索的哈希表。
 * @param key 要搜索的键。
 * @return 如果键存在，则返回true；否则返回false。
 */
bool hash_has(HashTable *hash, char *key);

/**
 * @brief 修改key对应的存值
 * 
 * @param hash 哈希表
 * @param key 键匙
 * @param value 存值
 */
void hash_set(HashTable *hash, char *key, int value);

/**
 * @brief 根据键匙获得存值
 * 
 * @param hash 哈希表
 * @param key 键匙
 * @return 存值
 */
int hash_get(HashTable *hash, char *key);


#define DEFAULT_ARRAY_CAP 16

typedef struct ValueArray ValueArray;
struct ValueArray {
    int size;
    int cap;
    Entry **entries;
};

ValueArray *new_value_array();
bool array_has(ValueArray *arr, char *key);
void array_set(ValueArray *arr, char *key, int value);
int array_get(ValueArray *arr, char *key);