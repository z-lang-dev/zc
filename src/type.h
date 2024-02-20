#pragma once
#include "zast.h"

typedef struct Type Type;
typedef struct TypeNum TypeNum;
typedef struct TypeUser TypeUser;
typedef struct TypeFn TypeFn;
typedef struct TypeArray TypeArray;

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
} TypeKind;

// 数字类型，包括int/bool/byte/float/double
struct TypeNum {
    int size; // 类型尺寸，即所占的字节数
};

// 自定以类型
struct TypeUser {
    int size; // 结构体所占的存储空间，字节数
    int field_count; // 成员字段个数
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

// 类型
struct Type {
    TypeKind kind;
    char *name;
    union {
        TypeNum num;
        TypeUser user;
        TypeFn fn;
        TypeArray array;
    } as;
};

extern const Type TYPE_BOOL;
extern const Type TYPE_BYTE;
extern const Type TYPE_INT;
extern const Type TYPE_FLOAT;
extern const Type TYPE_DOUBLE;
extern const Type TYPE_STR;

// 新建一个类型，必然是TY_USER类型
Type *new_type(char *name);

// 注意，每个不同尺寸和元素类型的数组都是不同的类型
Type *new_array_type(Type *item, int size);

Type *check_type(Node *node);