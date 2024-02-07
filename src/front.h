#pragma once

#include "zast.h"
#include "meta.h"
#include "parser.h"

typedef struct Front Front;
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

/**
 * @brief Z编译器的前端，负责源码到AST的解析。
 * 解释器、编译器和转译器都利用这个前端来解析源码。
*/
struct Front {
    SourceQueue *sources; // 要解析的源码。
};

Front *new_front();

Node *do_file(Front *front, const char *path);
Node *do_code(Front *front, const char *code);

// 添加文本形式的源码
Source *add_source(Front *front, const char *code);
// 添加文件形式的源码
Source *load_source(Front *front, const char *name);
