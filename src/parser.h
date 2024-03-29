#pragma once

#include "zast.h"
#include "lexer.h"
#include "meta.h"
#include "front.h"

typedef struct Parser Parser;

struct Parser {
    Lexer *lexer;
    char *code;
    Token *cur;
    Token *next;
    Scope *scope; // 当前的视野
    Scope *root_scope; // parser对应的顶层视野，即模块视野
    Front *front;
    HashTable *uses;
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
    PREC_DOT, // .
} Precedence;


Parser *new_parser(char *code, Scope *scope);
Node *parse(Parser *parser);
