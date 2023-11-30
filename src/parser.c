#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "util.h"
#include "lexer.h"
#include "meta.h"

static Node *expression(Parser *parser);
static Node *expr_prec(Parser *parser, Precedence base_prec);

static void advance(Parser *parser) {
    parser->cur = parser->next;
    if (parser->next->kind == TK_EOF) {
        return;
    }
    parser->next = next_token(parser->lexer);
}

static Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

static char* token_to_str(TokenKind kind) {
    switch (kind) {
    case TK_ADD: return "TK_ADD";
    case TK_SUB: return "TK_SUB";
    case TK_MUL: return "TK_MUL";
    case TK_DIV: return "TK_DIV";
    case TK_INTEGER: return "TK_INT";
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
    default: return "TK_UNKNOWN";
    }
}

static bool match(Parser *parser, TokenKind kind) {
    return parser->cur->kind == kind;
}

static bool skip_empty_line(Parser *parser) {
    bool has_end = false;
    while (parser->cur->kind == TK_NLINE || parser->cur->kind == TK_SEMI) {
        advance(parser);
        has_end = true;
    }
    return has_end;
}

// 检查当前词符是否为kind，如果是，就跳过它，否则就报错
static void expect(Parser *parser, TokenKind kind) {
    if (parser->cur->kind != kind) {
        printf("Expected %s, but got %s\n", token_to_str(kind), token_to_str(parser->cur->kind));
        exit(1);
    }
    advance(parser);
}

// 检查是否为表达式结束符（End Of Expression）
static void expect_eoe(Parser *parser) {
    bool has_end = skip_empty_line(parser);
    if (parser->cur->kind == TK_EOF || has_end) {
        return;
    } else {
        printf("Expected End of Expression, but got %s\n", token_to_str(parser->cur->kind));
        exit(1);
    }
}

typedef struct {
  int count;
  int cap;
  Node *data[];
} ArgBuf;

#define MAX_ARGS 4

ArgBuf *arg_buf;

static ArgBuf *args(Parser *parser) {
    if (arg_buf == NULL) {
        arg_buf = malloc(sizeof(ArgBuf) + MAX_ARGS * sizeof(Node *));
    }
    ArgBuf *buf = arg_buf;
    buf->count = 0;
    buf->cap = MAX_ARGS;
    while (parser->cur->kind != TK_RPAREN) {
        buf->data[buf->count++] = expression(parser);
        if (parser->cur->kind == TK_COMMA) {
        advance(parser);
        }
    }
    return buf;
}

static char *strip(char *str, int len) {
    char *result = calloc(len + 1, sizeof(char));
    int i = 0;
    while (i < len) {
        result[i] = str[i];
        i++;
    }
    result[i] = '\0';
    return result;
}

static char *get_text(Parser *parser) {
    return strip(parser->cur->pos, parser->cur->len);
}

static Node *name(Parser *parser) {
    Node *node = new_node(ND_NAME);
    node->as.str = get_text(parser);
    advance(parser);
    return node;
}

static Node *call(Parser *parser, Node *left) {
  expect(parser, TK_LPAREN);
  ArgBuf *buf = args(parser);
  expect(parser, TK_RPAREN); 
  Node *node = malloc(sizeof(Node) + buf->count * sizeof(Node *));
  node->kind = ND_CALL;
  node->as.call.name = left;
  node->as.call.argc = buf->count;
  for (int i = 0; i < buf->count; i++) {
    node->as.call.args[i] = buf->data[i];
  }
  trace_node(node);
  return node;
}

static Node* string(Parser *parser) {
  Node *node = new_node(ND_STR);
  node->as.str = strip(parser->cur->pos+1, parser->cur->len-2);
  advance(parser);
  return node;
}

static Node *integer(Parser *parser) {
    Node *expr = calloc(1, sizeof(Node));
    expr->kind = ND_INT;
    char *num_text = get_text(parser);
    log_trace("Parsing int text: %s\n", num_text);
    expr->as.num = atoll(num_text);
    // 打印出AST
    trace_node(expr);
    advance(parser);
    return expr;
}

static Node *bul(Parser *parser, bool val) {
    Node *expr = calloc(1, sizeof(Node));
    expr->kind = ND_BOOL;
    expr->as.bul = val;
    // 打印出AST
    trace_node(expr);
    advance(parser);
    return expr;
}

static Node *group(Parser *parser) {
    advance(parser); // 跳过'('
    Node *expr = expression(parser);
    advance(parser); // 跳过')'
    return expr;
}

static Node *neg(Parser *parser) {
    advance(parser);
    Node *expr = new_node(ND_NEG);
    expr->as.una.op = OP_SUB;
    expr->as.una.body = expr_prec(parser, PREC_NEG);
    // 打印出AST
    trace_node(expr);
    return expr;
}

static Node *not(Parser *parser) {
    advance(parser);
    Node *expr = new_node(ND_NOT);
    expr->as.una.op = OP_NOT;
    expr->as.una.body = expr_prec(parser, PREC_NOT);
    // 打印出AST
    trace_node(expr);
    return expr;
}

static Node *use(Parser *parser) {
    advance(parser); // skip 'use'
    Node *expr = new_node(ND_USE);
    expr->as.use.box = get_text(parser);
    advance(parser); // skip TK_NAME
    if (match(parser, TK_DOT)) {
        advance(parser); // skip '.'
        expr->as.use.name = get_text(parser);
        advance(parser); // skip TK_NAME
    }
    return expr;
}

static void do_meta(Parser *parser, Node *expr) {
    // 现在只需要收集定量`let`的元信息
    Meta *m = new_meta(expr, MT_LET);
    m->name = expr->as.asn.name->as.str;
    set_meta(m);
}

static Node *let(Parser *parser) {
    // 跳过'let'
    advance(parser);
    Node *expr = new_node(ND_LET);

    // 解析存量名称
    Node *name = new_node(ND_NAME);
    name->as.str = get_text(parser);
    expr->as.asn.name = name;
    advance(parser);

    // 解析'='
    expect(parser, TK_ASN);

    // 解析数值表达式
    expr->as.asn.value = expression(parser);

    // 收集元信息
    do_meta(parser, expr);
    return expr;
}

static Node *mut(Parser *parser) {
    // 跳过'mut'
    advance(parser);
    Node *expr = new_node(ND_MUT);

    // 解析存量名称
    Node *name = new_node(ND_NAME);
    name->as.str = get_text(parser);
    expr->as.asn.name = name;
    advance(parser);

    // 解析'='
    expect(parser, TK_ASN);

    // 解析数值表达式
    expr->as.asn.value = expression(parser);

    // 收集元信息
    do_meta(parser, expr);
    return expr;
}

static void *expect_eob(Parser *parser) {
    bool has_end = skip_empty_line(parser);
    if (has_end || parser->cur->kind == TK_RBRACE) {
        return;
    } else {
        printf("Expected end of block, but got %s\n", token_to_str(parser->cur->kind));
        exit(1);
    }
}

static Node *block(Parser *parser) {
    expect(parser, TK_LBRACE);
    Node *block = new_block();
    while (!match(parser, TK_RBRACE)) {
        append_expr(block, expression(parser));
        expect_eob(parser);
    }
    expect(parser, TK_RBRACE);
    return block;
}

static Node *if_else(Parser *parser) {
    advance(parser); // 跳过'if'
    Node *expr = new_node(ND_IF);
    expr->as.if_else.cond = expression(parser);
    expr->as.if_else.then = block(parser);
    if (match(parser, TK_ELSE)) {
        advance(parser); // 跳过'else'
        expr->as.if_else.els = block(parser);
    }
    return expr;
}

static Node *for_loop(Parser *parser) {
    advance(parser); // 跳过'for'
    Node *expr = new_node(ND_FOR);
    expr->as.loop.cond = expression(parser);
    expr->as.loop.body = block(parser);
    return expr;
}

static Node *unary(Parser *parser) {
  switch (parser->cur->kind) {
    case TK_LBRACE:
        return block(parser);
    case TK_LET:
        return let(parser);
    case TK_MUT:
        return mut(parser);
    case TK_USE:
        return use(parser);
    case TK_IF:
        return if_else(parser);
    case TK_FOR:
        return for_loop(parser);
    case TK_LPAREN:
        return group(parser);
    case TK_SUB:
        return neg(parser);
    case TK_NOT:
        return not(parser);
    case TK_STR:
        return string(parser);
    case TK_INTEGER:
        return integer(parser);
    case TK_TRUE:
        return bul(parser, true);
    case TK_FALSE:
        return bul(parser, false);
    case TK_NAME:
        return name(parser);
    default:
        printf("Unknown token: %s\n", token_to_str(parser->cur->kind));
        exit(1);
  }
}

static Op get_op(TokenKind kind) {
    switch (kind) {
    case TK_ADD:
        return OP_ADD;
    case TK_SUB:
        return OP_SUB;
    case TK_MUL:
        return OP_MUL;
    case TK_DIV:
        return OP_DIV;
    case TK_GT:
        return OP_GT;
    case TK_LT:
        return OP_LT;
    case TK_GE:
        return OP_GE;
    case TK_LE:
        return OP_LE;
    case TK_EQ:
        return OP_EQ;
    case TK_NE:
        return OP_NE;
    case TK_AND:
        return OP_AND;
    case TK_OR:
        return OP_OR;
    case TK_NOT:
        return OP_NOT;
    case TK_ASN:
        return OP_ASN;
    default:
        printf("Unknown operator: %d\n", kind);
        return OP_ILL;
    }
}

static Precedence get_prec(TokenKind kind) {
    switch (kind) {
    case TK_ADD:
    case TK_SUB:
        return PREC_ADDSUB;
    case TK_MUL:
    case TK_DIV:
        return PREC_MULDIV;
    case TK_EOF:
        return PREC_NONE;
    case TK_GT:
    case TK_LT:
    case TK_GE:
    case TK_LE:
    case TK_EQ:
    case TK_NE:
        return PREC_COMPARE;
    case TK_AND:
    case TK_OR:
        return PREC_ANDOR;
    case TK_ASN:
        return PREC_ASN;
    default:
        printf("Unknown operator for prec: %d\n", kind);
        return PREC_NONE;
    }
}

static bool is_binop(TokenKind kind) {
    return 
        kind == TK_ADD || kind == TK_SUB || kind == TK_MUL || kind == TK_DIV || 
        kind == TK_GT || kind == TK_LT || kind == TK_GE || kind == TK_LE || 
        kind == TK_EQ || kind == TK_NE || 
        kind == TK_AND || kind == TK_OR ||
        kind == TK_ASN;
}

static Node *binop(Parser *parser, Node *left, Precedence base_prec) {
    Token *cur = parser->cur;
    // 如果下一个词符是运算符，那么应当是一个二元表达式
    if (is_binop(cur->kind)) {
        Precedence cur_prec = get_prec(cur->kind); // 当前操作符的优先级
        if (cur_prec < base_prec) {
            return left;
        }
        Node *bop = calloc(1, sizeof(Node));
        bop->kind = ND_BINOP;
        Op op = get_op(cur->kind);
        bop->as.bop.op = op;
        if (op == OP_ASN && left->kind == ND_NAME) {
            left->kind = ND_LNAME;
        }
        bop->as.bop.left = left;
        advance(parser);
        Node *right = unary(parser);
        if (!is_binop(parser->cur->kind)) {
            bop->as.bop.right = right;
            return bop;
        }
        Precedence next_prec = get_prec(parser->cur->kind); // 下一个操作符的优先级。注意，调用`unary`之后，cur已经指向了下一个词符
        // peek
        if (next_prec > cur_prec) { // 下一个操作符优先级更高，右结合
            // 如果下一个运算符的优先级更高，那么就递归调用binop
            Node *right_bop = binop(parser, right, get_prec(parser->cur->kind));
            bop->as.bop.right = right_bop;
            Node *res = binop(parser, bop, base_prec);
            echo_node(res);
            return res;
        } else if (next_prec >= base_prec) { // base_prec <= next_prec < cur_prec，左结合
            bop->as.bop.right = right;
            // 打印出AST
            Node *res = binop(parser, bop, get_prec(parser->cur->kind));
            // echo_node(bop);
            return res;
        } else {  // next_rec < base_prec，退回到上一层。
            // 不再递归
            bop->as.bop.right = right;
            return bop;
        }
    } else {
        // 否则就直接返回左子节点。
        echo_node(left);
        return left;
    }
}


// 解析一个表达式
static Node *expr_prec(Parser *parser, Precedence base_prec) {
    // 表达式由一个一元操作开头，后接多层二元操作。

    // 先尝试一元操作
    Node *left = unary(parser);

    // 接着尝试二元操作
    switch (parser->cur->kind) {
    case TK_LPAREN:
        return call(parser, left);
    default:
        return binop(parser, left, base_prec);
    }
}

static Node *expression(Parser *parser) {
    skip_empty_line(parser);
    return expr_prec(parser, PREC_NONE);
}

static bool is_end(Parser *parser) {
    return parser->cur->kind == TK_EOF;
}

/**
 * 
 * 解析一段Z源码
 *
 * @param parser The parser object.
 * @return 一个`ND_PROG`类型的节点，代表一段Z源码。
 */
Node *parse(Parser *parser) {
    char *code = parser->code;
    log_trace("Parsing %s...\n", code);
    // 解析源码
    size_t len = strlen(code);
    if (len == 0) {
        return NULL;
    }

    Node *prog = new_prog();
    while (!is_end(parser)) {
        append_expr(prog, expression(parser));
        expect_eoe(parser);
    }

    log_trace("Parse done.\n");
    trace_node(prog);
    return prog;
}

/**
 * Creates a new Parser object.
 *
 * @param code The code to be parsed.
 * @return A pointer to the newly created Parser object.
 */
Parser *new_parser(char *code) {
    Parser *parser = calloc(1, sizeof(Parser));
    parser->lexer = new_lexer(code);
    parser->code = code;
    parser->cur = next_token(parser->lexer);
    parser->next = next_token(parser->lexer);
    init_meta();
    return parser;
}