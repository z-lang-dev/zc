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

typedef enum {
    PREC_NONE,
    PREC_ADDSUB,
    PREC_MULDIV,
    PREC_NEG,
} Precedence;


Parser *new_parser(char *code);
Node *parse(Parser *parser);
