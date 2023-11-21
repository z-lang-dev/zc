#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "util.h"
#include "lexer.h"

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
    case TK_ADD:
        return "TK_ADD";
    case TK_SUB:
        return "TK_SUB";
    case TK_MUL:
        return "TK_MUL";
    case TK_DIV:
        return "TK_DIV";
    case TK_INT:
        return "TK_INT";
    case TK_LPAREN:
        return "TK_LPAREN";
    case TK_RPAREN:
        return "TK_RPAREN";
    case TK_EOF:
        return "TK_EOF";
    case TK_NAME:
        return "TK_NAME";
    case TK_STR:
        return "TK_STR";
    case TK_COMMA:
        return "TK_COMMA";
    default:
        return "TK_ILL";
    }
}

static void expect(Parser *parser, TokenKind kind) {
    if (parser->cur->kind != kind) {
        printf("Expected %s, but got %s\n", token_to_str(kind), token_to_str(parser->cur->kind));
        exit(1);
    }
    advance(parser);
}


// 检查是否为表达式结束符（End Of Expression）
static void expect_eoe(Parser *parser) {
    TokenKind kind = parser->cur->kind;
    if (kind == TK_NLINE || kind == TK_SEMI) {
        advance(parser);
    } else if (kind == TK_EOF) {
        return;
    } else {
        printf("Expected %s, but got %s\n", token_to_str(kind), token_to_str(kind));
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

static Node *fname(Parser *parser) {
  Node *node = new_node(ND_FNAME);
  node->as.str = strip(parser->cur->pos, parser->cur->len);
  advance(parser);
  return node;
}

static Node *call(Parser *parser) {
  Node *fn = fname(parser);
  expect(parser, TK_LPAREN);
  ArgBuf *buf = args(parser);
  expect(parser, TK_RPAREN); 
  Node *node = malloc(sizeof(Node) + buf->count * sizeof(Node *));
  node->kind = ND_CALL;
  node->as.call.fname = fn;
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

static char *extract_token_text(Parser *parser) {
    char *code = parser->code;
    int len = parser->cur->len;
    char *result = calloc(len + 1, sizeof(char));
    int i = 0;
    while (i < len) {
        result[i] = parser->cur->pos[i];
        i++;
    }
    result[i] = '\0';
    return result;
}

static Node *integer(Parser *parser) {
    Node *expr = calloc(1, sizeof(Node));
    expr->kind = ND_INT;
    char *num_text = extract_token_text(parser);
    log_trace("Parsing int text: %s\n", num_text);
    expr->as.num = atoll(num_text);
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

static Node *unary(Parser *parser) {
  switch (parser->cur->kind) {
    case TK_LPAREN:
        return group(parser);
    case TK_SUB:
        return neg(parser);
      case TK_STR:
        return string(parser);
    case TK_INT:
        return integer(parser);
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
    default:
        printf("Unknown operator for prec: %d\n", kind);
        return PREC_NONE;
    }
}

static bool is_binop(TokenKind kind) {
    return kind == TK_ADD || kind == TK_SUB || kind == TK_MUL || kind == TK_DIV;
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
        bop->as.bop.op = get_op(cur->kind);
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
    // 表达式可以是一个整数、一系列运算，或者一个调用
    if (parser->cur->kind != TK_NAME) {
        // 如果是整数
        Node *left = unary(parser);
        // Precedence prec = base_prec;
        // if (left->kind == ND_NEG && PREC_NEG > base_prec) {
        //     prec = PREC_NEG;
        // }
        return binop(parser, left, base_prec);
    } else {
        // 否则就是一个函数调用
        return call(parser);
    }
}

static Node *expression(Parser *parser) {
    return expr_prec(parser, PREC_NONE);
}

static bool is_end(Parser *parser) {
    return parser->cur->kind == TK_EOF;
}

// 解析表达式
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
    return prog;
}

Parser *new_parser(char *code) {
    Parser *parser = calloc(1, sizeof(Parser));
    parser->lexer = new_lexer(code);
    parser->code = code;
    parser->cur = next_token(parser->lexer);
    parser->next = next_token(parser->lexer);
    return parser;
}