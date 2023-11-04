#pragma once

#include "zast.h"

void codegen_linux(CallExpr *expr);
void codegen_win(CallExpr *expr);