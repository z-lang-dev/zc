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

static Token *single_or_double_token(Lexer *lexer, char follow, TokenKind op1, TokenKind op2) {
    if (peek(lexer) == follow) {
        next_char(lexer);
        return new_token(lexer, op2);
    } else {
        return new_token(lexer, op1);
    }
}

static Token *double_token(Lexer *lexer, char c, TokenKind op) {
    if (peek(lexer) == c) {
        next_char(lexer);
        return new_token(lexer, op);
    } else {
        log_trace("double token error: %c %c\n", c, peek(lexer));
        return new_token(lexer, TK_EOF);
    }
}

// 解析整数或浮点数
static Token *number(Lexer *lexer) {
    bool has_dot = false;
    while (is_digit(*lexer->cur) || peek(lexer) == '.') {
        if (peek(lexer) == '.') {
            if (has_dot) {
                log_trace("Unexpected twice dot: %c\n", *lexer->cur);
                return new_token(lexer, TK_EOF);
            } else {
                has_dot = true;
            }
        }
        next_char(lexer);
    }
    if (has_dot) {
        if (peek(lexer) == 'f') {
            Token * ret = new_token(lexer, TK_FLOAT_NUM);
            next_char(lexer);
            return ret;
        } else if (peek(lexer) == 'd') {
            Token *ret = new_token(lexer, TK_DOUBLE_NUM);
            next_char(lexer);
            return ret;
        } else {
            // 浮点数默认64位。TODO: lib场景下，应当必须声明f或d结尾。
            return new_token(lexer, TK_DOUBLE_NUM);
        }
    } else {
        return new_token(lexer, TK_INT_NUM);
    }
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
    {"bool", TK_BOOL},
    {"break", TK_BREAK},
    {"continue", TK_CONTINUE},
    {"else", TK_ELSE},
    {"false", TK_FALSE},
    {"float", TK_FLOAT},
    {"double", TK_DOUBLE},
    {"fn", TK_FN},
    {"for", TK_FOR},
    {"if", TK_IF},
    {"int", TK_INT},
    {"let", TK_LET},
    {"mut", TK_MUT},
    {"true", TK_TRUE},
    {"type", TK_TYPE},
    {"use", TK_USE},
};

// 查找关键字
// TODO: 现在是顺序对比，以后可以考虑用hash表，或者用trie树
static Token *lookup_keyword(Lexer *lexer) {
    for (int i = 0; i < sizeof(keywords) / sizeof(Keyword); i++) {
        if (lexer->cur - lexer->start != strlen(keywords[i].name)) {
            continue;
        }
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
    case '{':
        return new_token(lexer, TK_LBRACE);
    case '}':
        return new_token(lexer, TK_RBRACE);
    case '[':
        return new_token(lexer, TK_LSQUARE);
    case ']':
        return new_token(lexer, TK_RSQUARE);
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
    case ':':
        return new_token(lexer, TK_COLON);
    case '=':
        return single_or_double_token(lexer, '=', TK_ASN, TK_EQ);
    case '!':
        return single_or_double_token(lexer, '=', TK_NOT, TK_NE);
    case '>':
        return single_or_double_token(lexer, '=', TK_GT, TK_GE);
    case '<':
        return single_or_double_token(lexer, '=', TK_LT, TK_LE);
    case '&':
        return double_token(lexer, c, TK_AND);
    case '|':
        return double_token(lexer, c, TK_OR);
    default:
        log_trace("Unexpected character: %c\n", c);
        return new_token(lexer, TK_EOF);
    }
}

char* token_to_str(TokenKind kind) {
    switch (kind) {
    case TK_ADD: return "TK_ADD";
    case TK_SUB: return "TK_SUB";
    case TK_MUL: return "TK_MUL";
    case TK_DIV: return "TK_DIV";
    case TK_INT_NUM: return "TK_INT_NUM";
    case TK_FLOAT_NUM: return "TK_FLOAT_NUM";
    case TK_DOUBLE_NUM: return "TK_DOUBLE_NUM";
    case TK_LPAREN: return "TK_LPAREN";
    case TK_RPAREN: return "TK_RPAREN";
    case TK_EOF: return "TK_EOF";
    case TK_NAME: return "TK_NAME";
    case TK_STR: return "TK_STR";
    case TK_COMMA: return "TK_COMMA";
    case TK_NLINE: return "TK_NLINE";
    case TK_SEMI: return "TK_SEMI";
    case TK_DOT: return "TK_DOT";
    case TK_USE: return "TK_USE";
    case TK_ASN: return "TK_ASN";
    case TK_LET: return "TK_LET";
    case TK_MUT: return "TK_MUT";
    case TK_IF: return "TK_IF";
    case TK_ELSE: return "TK_ELSE";
    case TK_LBRACE: return "TK_LBRACE";
    case TK_RBRACE: return "TK_RBRACE";
    case TK_LSQUARE: return "TK_LSQUARE";
    case TK_RSQUARE: return "TK_RSQUARE";
    case TK_GT: return "TK_GT";
    case TK_LT: return "TK_LT";
    case TK_GE: return "TK_GE";
    case TK_LE: return "TK_LE";
    case TK_EQ: return "TK_EQ";
    case TK_NE: return "TK_NE";
    case TK_AND: return "TK_AND";
    case TK_OR: return "TK_OR";
    case TK_TRUE: return "TK_TRUE";
    case TK_FALSE: return "TK_FALSE";
    case TK_NOT: return "TK_NOT";
    case TK_INT: return "TK_INT";
    case TK_FLOAT : return "TK_FLOAT";
    case TK_DOUBLE: return "TK_DOUBLE";
    case TK_FOR: return "TK_FOR";
    case TK_FN: return "TK_FN";
    case TK_BOOL: return "TK_BOOL";
    case TK_TYPE: return "TK_TYPE";
    default: return "TK_UNKNOWN";
    }
}

