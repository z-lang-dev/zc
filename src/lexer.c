#include <stdlib.h>
#include <stdbool.h>
#include "lexer.h"

// 初始化一个词法分析器
Lexer *new_lexer(const char *code) {
    Lexer *lexer = calloc(1, sizeof(Lexer));
    lexer->start = code;
    lexer->cur = code;
    return lexer;
}

static bool is_eof(Lexer *lexer) {
    return *lexer->cur == '\0';
}

static bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

static bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c) {
    return is_digit(c) || is_alpha(c);
}

static char peek(Lexer *lexer) {
    return *lexer->cur;
}

static Token *new_token(Lexer *lexer, TokenKind kind) {
    Token *token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->pos = lexer->start;
    token->len = lexer->cur - lexer->start;
    return token;
}

static char next_char(Lexer *lexer) {
    lexer->cur++;
    return lexer->cur[-1];
}

static Token *number(Lexer *lexer) {
    while (is_digit(*lexer->cur)) {
        next_char(lexer);
    }
    return new_token(lexer, TK_INT);
}

static Token *str(Lexer *lexer) {
    while (*lexer->cur != '"') {
        next_char(lexer);
    }
    next_char(lexer);
    return new_token(lexer, TK_STR);
}

static Token *name(Lexer *lexer) {
    while (is_alnum(peek(lexer))) {
        next_char(lexer);
    }
    return new_token(lexer, TK_NAME);
}

// 解析下一个Token
Token *next_token(Lexer *lexer) {
    // 更新start指针，指向上个Token的末尾
    lexer->start = lexer->cur;

    // 如果遇到文件或源码末尾，就返回TK_EOF
    if (is_eof(lexer)) {
        return new_token(lexer, TK_EOF);
    }
    // 读取一个字符
    char c = next_char(lexer);

    // 如果是数字
    if (is_digit(c)) {
        return number(lexer);
    }

    // '"'代表字符串
    if (c == '"') {
        return str(lexer);
    }

    // 如果是字母或下划线
    if (is_alpha(c)) {
        return name(lexer);
    }

    // 处理其他的特殊字符
    switch (c) {
    case '(':
        return new_token(lexer, TK_LPAREN);
    case ')':
        return new_token(lexer, TK_RPAREN);
    }
}