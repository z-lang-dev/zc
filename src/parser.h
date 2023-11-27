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
    PREC_ANDOR, // &&, ||
    PREC_COMPARE, // >, <, >=, <=, ==, !=
    PREC_ADDSUB, // +, -
    PREC_MULDIV, // *, /
    PREC_NEG, // -
    PREC_NOT, // !
} Precedence;


Parser *new_parser(char *code);
Node *parse(Parser *parser);
