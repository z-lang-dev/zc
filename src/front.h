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
    Scope *scope; // 源码的视野，这里的源码可能是整个源码的一部分（例如REPL的一段），因此它可能有先天的共享视野
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

/**
 * @brief 模块：用来存储一个模块的前端解析结果
*/
struct Mod {
    ModKind kind; // 种类：库文件或应用文件
    char *name; // 模块名称
    Source *source; // 对应的源码
    Scope *scope; // 模块的视野
    Node *prog; // 模块的AST
    HashTable *uses; // 对其他模块的引用
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

Meta *mod_lookup(Front *front, Node *path);

// 添加文本形式的源码
Source *add_source(Front *front, const char *code);
// 添加文件形式的源码
Source *load_source(Front *front, const char *name);
