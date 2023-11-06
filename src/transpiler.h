#pragma once

#include "zast.h"

// 将AST编译成C代码
void trans_c(char *file);

// 将AST编译成Python代码
void trans_py(char *file);

// 将AST编译成JS代码
void trans_js(char *file);
