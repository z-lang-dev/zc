#pragma once
#include "meta.h"

Meta *new_builtin(char *name);
void make_builtins(Scope *scope);
void use_stdz(Scope *scope);
Meta *new_stdfn(char *name);