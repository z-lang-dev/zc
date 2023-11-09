#pragma once

#include "zast.h"
#include "lexer.h"

typedef struct Parser Parser;

struct Parser {
    Lexer *lexer;
    char *code;
    Token *cur;
    Token *next;
};

Parser *new_parser(char *code);
Node *parse(Parser *parser);
