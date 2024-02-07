#pragma once

#include "zast.h"
#include "value.h"
#include "front.h"

// 解释代码
void interp(char *code);

void interp_once(Front *front, char *code);

// 执行AST
Value *execute(Node *code);
