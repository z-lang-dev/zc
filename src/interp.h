#pragma once

#include "zast.h"

// 解释代码
void interp(char *code);

// 执行AST
void execute(CallExpr *call);