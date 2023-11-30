#pragma once

typedef struct Token Token;
typedef struct Lexer Lexer;

// 词符的类型
typedef enum {
    // keywords
    TK_LET, // let
    TK_MUT, // mut
    TK_TRUE, // true
    TK_FALSE, // false
    TK_INT, // int
    TK_BOOL, // bool
    TK_IF, // if
    TK_FOR, // for
    TK_BREAK, // break
    TK_CONTINUE, // continue
    TK_ELSE, // else
    TK_NAME, // 名称
    TK_ASN, // =
    TK_USE, // 导入
    TK_INTEGER, // 整数
    TK_LPAREN, // (
    TK_RPAREN, // )
    TK_LBRACE, // {
    TK_RBRACE, // }
    TK_STR, // 字符串
    TK_ADD, // +
    TK_SUB, // -
    TK_MUL, // *
    TK_DIV, // /
    TK_GT, // >
    TK_LT, // <
    TK_GE, // >=
    TK_LE, // <=
    TK_EQ, // ==
    TK_NE, // !=
    TK_NOT, // !
    TK_AND, // &&
    TK_OR, // ||
    TK_COMMA, // ,
    TK_NLINE, // \n
    TK_SEMI, // ;
    TK_DOT, // .
    TK_EOF // 源码结束
} TokenKind;

// 词符
struct Token {
    TokenKind kind;
    const char *pos; // 指向词符在源码中的起始位置
    size_t len; // 词符的长度
};

// 词法分析器
struct Lexer {
    char* start; // 解析的起始位置。每解析完一个词符，start就会被更新。
    char* cur; // 解析的当前位置。解析完一个词符时，start到cur之间的字符串就是词符的内容。
};


// 新建一个词法分析器
Lexer *new_lexer(const char *code);

// 解析下一个词符
Token *next_token(Lexer *lexer);
