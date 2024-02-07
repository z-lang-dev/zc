#pragma once

#include "zast.h"
#include "lexer.h"
#include "meta.h"

typedef struct Parser Parser;

struct Parser {
    Lexer *lexer;
    char *code;
    Token *cur;
    Token *next;
    Scope *scope; // 当前的视野
};

typedef enum {
    PREC_NONE,
    PREC_ASN, // =
    PREC_ANDOR, // &&, ||
    PREC_COMPARE, // >, <, >=, <=, ==, !=
    PREC_ADDSUB, // +, -
    PREC_MULDIV, // *, /
    PREC_NEG, // -
    PREC_NOT, // !
} Precedence;


Parser *new_parser(char *code);
Node *parse(Parser *parser);
