#include <stdlib.h>
#include <stdbool.h>
#include "lexer.h"
#include "util.h"

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

static char peek(Lexer *lexer) {
    return *lexer->cur;
}

static char next_char(Lexer *lexer) {
    if (lexer->cur != '\0') {
        lexer->cur++;
        return lexer->cur[-1];
    } else {
        return '\0';
    }
}

static Token *new_token(Lexer *lexer, TokenKind kind) {
    Token *token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->pos = lexer->start;
    token->len = lexer->cur - lexer->start;
    return token;
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

typedef struct {
    const char *name;
    TokenKind kind;
} Keyword;

static Keyword keywords[] = {
    {"use", TK_USE},
};

// 查找关键字
// TODO: 现在是顺序对比，以后可以考虑用hash表，或者用trie树
static Token *lookup_keyword(Lexer *lexer) {
    for (int i = 0; i < sizeof(keywords) / sizeof(Keyword); i++) {
        if (strncmp(lexer->start, keywords[i].name, lexer->cur - lexer->start) == 0) {
            return new_token(lexer, keywords[i].kind);
        }
    }
    return NULL;
}

static Token *name(Lexer *lexer) {
    while (is_alnum(peek(lexer))) {
        next_char(lexer);
    }

    Token *keyword = lookup_keyword(lexer);
    return keyword != NULL ? keyword : new_token(lexer, TK_NAME);
}

static void skip_whitespace(Lexer *lexer) {
    char c = peek(lexer);
    while (c == ' ' || c == '\t' || c == '\r') {
        next_char(lexer);
        c = peek(lexer);
    }
}

// 解析下一个Token
Token *next_token(Lexer *lexer) {
    skip_whitespace(lexer);
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
    case '+':
        return new_token(lexer, TK_ADD);
    case '-':
        return new_token(lexer, TK_SUB);
    case '*':
        return new_token(lexer, TK_MUL);
    case '/':
        return new_token(lexer, TK_DIV);
    case ',':
        return new_token(lexer, TK_COMMA);
    case '\n':
        return new_token(lexer, TK_NLINE);
    case ';':
        return new_token(lexer, TK_SEMI);
    case '.':
        return new_token(lexer, TK_DOT);
    default:
        log_trace("Unexpected character: %c\n", c);
        return new_token(lexer, TK_EOF);
    }

}