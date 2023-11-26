#pragma once

#include "zast.h"
#include "value.h"

// 解释代码
void interp(char *code);

// 执行AST
Value *execute(Node *code);