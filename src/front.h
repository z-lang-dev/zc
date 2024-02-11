#pragma once

#include "zast.h"
#include "meta.h"

typedef struct Front Front;
typedef struct Mod Mod;
typedef struct Source Source;
typedef struct SourceQueue SourceQueue;

struct Source {
    const char *name; // 源码的名称，一般即文件名。
    const char *code; // 源码的文本。
};

struct SourceQueue {
    int count;
    int cap;
    Source **list;
};

typedef enum {
    MOD_LIB,
    MOD_APP,
} ModKind;

struct Mod {
    ModKind kind;
    char *name;
    Source *source;
    Scope *scope;
    Node *prog;
};

/**
 * @brief Z编译器的前端，负责源码到AST的解析。
 * 解释器、编译器和转译器都利用这个前端来解析源码。
*/
struct Front {
    SourceQueue *sources; // 要解析的源码。
    HashTable *mods; // 已经解析的模块：{名称->Node*}
};

Front *new_front();

Mod *do_file(Front *front, const char *path);
Mod *do_code(Front *front, const char *code);

Meta *name_lookup(Front *front, const char *name);
Meta *path_lookup(Front *front, const char *path);

// 添加文本形式的源码
Source *add_source(Front *front, const char *code);
// 添加文件形式的源码
Source *load_source(Front *front, const char *name);
