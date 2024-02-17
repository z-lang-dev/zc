#include "types.h"

const Type TYPE_BOOL = {TY_BOOL, "bool", 1};
const Type TYPE_BYTE = {TY_BYTE, "byte", 1};
const Type TYPE_INT = {TY_INT, "int", 4};
const Type TYPE_FLOAT = {TY_FLOAT, "float", 4};
const Type TYPE_DOUBLE = {TY_DOUBLE, "double", 8};

Type *new_type(char *name) {
    Type *type = malloc(sizeof(Type));
    type->kind = TY_USER;
    type->name = name;
    return type;
}