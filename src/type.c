#include <stdlib.h>
#include "type.h"
#include "string.h"

const Type TYPE_BOOL = {TY_BOOL, "bool", 1};
const Type TYPE_BYTE = {TY_BYTE, "byte", 1};
const Type TYPE_INT = {TY_INT, "int", 4};
const Type TYPE_FLOAT = {TY_FLOAT, "float", 4};
const Type TYPE_DOUBLE = {TY_DOUBLE, "double", 8};
const Type TYPE_STR = {TY_STR, "str", 0};

Type *new_type(char *name) {
    Type *type = malloc(sizeof(Type));
    type->kind = TY_USER;
    type->name = name;
    return type;
}

Type *new_array_type(Type *item, int size) {
    // TODO: 相同的数组类型应当缓存
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_ARRAY;
    type->name = "array";
    type->as.array.item = item;
    type->as.array.size = size;
    return type;
}

Type *check_type(Node *node) {
    switch (node->kind) {
    case ND_INT:
        return &TYPE_INT;
    case ND_FLOAT:
        return &TYPE_FLOAT;
    case ND_DOUBLE:
        return &TYPE_DOUBLE;
    case ND_BOOL:
        return &TYPE_BOOL;
    case ND_STR:
        return &TYPE_STR;
    default:
        return NULL;
    }
}
