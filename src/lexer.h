#pragma once

typedef struct Token Token;
typedef struct Lexer Lexer;

// 词符的类型
typedef enum {
    TK_NAME, // 名称
    TK_LET, // let
    TK_ASN, // =
    TK_USE, // 导入
    TK_INT, // 整数
    TK_LPAREN, // (
    TK_RPAREN, // )
    TK_STR, // 字符串
    TK_ADD, // +
    TK_SUB, // -
    TK_MUL, // *
    TK_DIV, // /
    TK_COMMA, // ,
    TK_NLINE, // \n
    TK_SEMI, // ;
    TK_DOT, // .
    TK_EOF, // 源码结束
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
