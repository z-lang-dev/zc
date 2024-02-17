#pragma once

typedef struct Type Type;
typedef struct TypeNum TypeNum;
typedef struct TypeUser TypeUser;
typedef struct TypeFn TypeFn;

// 类型系统
typedef enum {
    TY_VOID, // 空类型
    TY_INT, // int
    TY_BOOL,
    TY_BYTE,
    TY_FLOAT,
    TY_DOUBLE,
    TY_USER, // 自定义类型
    TY_FN, // 函数类型
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

// 类型
struct Type {
    TypeKind kind;
    char *name;
    union {
        TypeNum num;
        TypeUser user;
        TypeFn fn;
    } as;
};

extern const Type TYPE_BOOL;
extern const Type TYPE_BYTE;
extern const Type TYPE_INT;
extern const Type TYPE_FLOAT;
extern const Type TYPE_DOUBLE;

// 新建一个类型，必然是TY_USER类型
Type *new_type(char *name);
