#pragma once
#include "zast.h"

typedef struct Type Type;
typedef struct TypeNum TypeNum;
typedef struct TypeUser TypeUser;
typedef struct TypeFn TypeFn;
typedef struct TypeArray TypeArray;
typedef struct TypeDict TypeDict;

// 类型系统
typedef enum {
    TY_VOID, // 空类型
    TY_INT, // int
    TY_BOOL,
    TY_BYTE,
    TY_FLOAT,
    TY_DOUBLE,
    TY_STR, // 字符串类型（静态）
    TY_USER, // 自定义类型
    TY_FN, // 函数类型
    TY_ARRAY, // 数组类型
    TY_DICT, // 字典类型
} TypeKind;

// 数字类型，包括int/bool/byte/float/double
struct TypeNum {
    int size; // 类型尺寸，即所占的字节数
};

// 自定以类型
struct TypeUser {
    int size; // 结构体所占的存储空间，字节数
    int field_count; // 成员字段个数

    HashTable *members; // 成员表：{名称->Meta*}
};

struct TypeFn {
    int param_count;
    Type **params;
    Type *ret;
};

struct TypeArray {
    int size;
    Type *item;
};

struct TypeDict {
    Type *key;
    Type *val;
};

// 类型
struct Type {
    TypeKind kind;
    char *name;
    union {
        TypeNum num;
        TypeUser user;
        TypeFn fn;
        TypeArray array;
        TypeDict dict;
    } as;
};

extern const Type TYPE_BOOL;
extern const Type TYPE_BYTE;
extern const Type TYPE_INT;
extern const Type TYPE_FLOAT;
extern const Type TYPE_DOUBLE;
extern const Type TYPE_STR;

// 新建一个类型，必然是TY_USER类型
Type *new_user_type(char *name);

// 注意，每个不同尺寸和元素类型的数组都是不同的类型
Type *new_array_type(Type *item, int size);

Type *new_dict_type(Type *key, Type *val);

Type *check_primary_type(Node *node);
