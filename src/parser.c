#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "util.h"
#include "lexer.h"
#include "meta.h"
#include "front.h"

static Node *expression(Parser *parser);
static Node *expr_prec(Parser *parser, Precedence base_prec);

static void advance(Parser *parser) {
    parser->cur = parser->next;
    if (parser->next->kind == TK_EOF) {
        return;
    }
    parser->next = next_token(parser->lexer);
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

static void enter_scope(Parser *parser) {
    Scope *scope = new_scope(parser->scope);
    parser->scope = scope;
}

static void exit_scope(Parser *parser) {
    parser->scope = parser->scope->parent;
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

static void register_use(HashTable *uses, Node *path) {
    if (path->kind != ND_IDENT) return;
    if (path->as.path.len < 2) return;
    char *mod = path->as.path.names[0].name;
    char *name = path->as.path.names[1].name;
    char *key = calloc(1, strlen(mod) + strlen(name) + 2, sizeof(char));
    sprintf(key, "%s.%s", mod, name);
    hash_set(uses, key, path);
}

// 定义新的名称，不需要查找元信息，不需要考虑多级名称路径的情况
static Node *new_name(Parser *parser) {
    Node *node = new_node(ND_IDENT);
    char *n = get_text(parser);
    advance(parser);
    node->as.path.names[0].name = n;
    node->as.path.len = 1;
    return node;
}

static Meta *member_lookup(Parser *parser, char *obj_name, char *mem_name) {
    Meta *obj = scope_lookup(parser->scope, obj_name);
    if (obj == NULL) {
        printf("Error: Unknown object name: %s\n", obj_name);
        exit(1);
    }
    Type *obj_type = obj->type;
    Meta *mem_meta = hash_get(obj_type->as.user.members, mem_name);
    if (mem_meta == NULL) {
        printf("Error: Unknown member name: %s.%s\n", obj_name, mem_name);
        exit(1);
    }
    return mem_meta;
}

// 注意，暂时只支持两层路径。
static Meta *ident_lookup(Parser *parser, Node *node) {
    int len = node->as.path.len;
    if (len < 1) return NULL;
    if (len == 1) return scope_lookup(parser->scope, node->as.path.names[0].name);

    // 如果是多层路径，那么先尝试该名称是否为模块名
    char *first = node->as.path.names[0].name;
    char *sec = node->as.path.names[1].name;
    Mod *mod = find_mod(parser->front, first);
    if (mod != NULL) { // 如果是模块名，就尝试在模块的视野中查找
        return scope_lookup(mod->scope, node->as.path.names[1].name);
    } else { // 如果不是模块名，那么就应当是对象属性的访问
        return member_lookup(parser, first, sec);
    }
}

static Node *new_ident(Parser *parser) {
    Node *node = new_node(ND_IDENT);
    int count = 0;
    char *n = get_text(parser);
    node->as.path.names[count++].name =  n;
    advance(parser);
    if (match(parser, TK_DOT)) {
        while (match(parser, TK_DOT)) {
            advance(parser); // skip '.'
            node->as.path.names[count++].name = get_text(parser);
            advance(parser); // skip name
            if (count > MAX_PATH_LEN) {
                printf("Too many names in path: \n");
                print_node(node);
                exit(1);
            }
        }
    }
    node->as.path.len = count;

    return node;
}

// 解析一个标识名称；可以是单个名字，如`a`，也可以是多级路径，如`a.b.c`
static Node *ident(Parser *parser) {
    Node *node = new_ident(parser);

    // 从第一个名称开始，一个个查询其元信息
    Meta *m = ident_lookup(parser, node);
    if (m == NULL) {
        printf("Unknown name: ");
        echo_node(node);
        printf("\n");
        exit(1);
    }
    node->meta = m;
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
    Node *name = node->as.call.name;
    if (name->kind == ND_IDENT) {
        Meta *m = ident_lookup(parser, name);
        if (m == NULL) {
            printf("Error: Unknown call function name: %s\n", get_name(name));
            exit(-1);
        }
        node->meta = m;
        return node;
    } else {
        printf("Error: Unknown call name kind: %d for name ", name->kind);
        echo_node(name);
        printf("\n");
        exit(-1);
    }
}


static Node* string(Parser *parser) {
  Node *node = new_node(ND_STR);
  node->as.str = strip(parser->cur->pos+1, parser->cur->len-2);
  node->meta->type = &TYPE_STR;
  advance(parser);
  return node;
}

static Node *integer(Parser *parser) {
    Node *expr = new_node(ND_INT);
    char *num_text = get_text(parser);
    expr->as.num.lit = num_text;
    log_trace("Parsing int text: %s\n", num_text);
    expr->as.num.val = atoll(num_text);
    expr->meta->type = &TYPE_INT;
    // 打印出AST
    trace_node(expr);
    advance(parser);
    return expr;
}

static Node *float_num(Parser *parser) {
    Node *expr = new_node(ND_FLOAT);
    char *num_text = get_text(parser);
    expr->as.float_num.lit = num_text;
    log_trace("Parsing float text: %s\n", num_text);
    expr->as.float_num.val = atof(num_text);
    expr->meta->type = &TYPE_FLOAT;
    // 打印出AST
    trace_node(expr);
    advance(parser);
    return expr;
}

static Node *double_num(Parser *parser) {
    Node *expr = new_node(ND_DOUBLE);
    char *num_text = get_text(parser);
    expr->as.double_num.lit = num_text;
    log_trace("Parsing double text: %s\n", num_text);
    expr->as.double_num.val = atof(num_text);
    expr->meta->type = &TYPE_DOUBLE;
    // 打印出AST
    trace_node(expr);
    advance(parser);
    return expr;
}

static Node *bul(Parser *parser, bool val) {
    Node *expr = new_node(ND_BOOL);
    expr->as.bul = val;
    expr->meta->type = &TYPE_BOOL;
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
    expr->meta->type = expr->as.una.body->meta->type;
    // 打印出AST
    trace_node(expr);
    return expr;
}

static Node *not(Parser *parser) {
    advance(parser);
    Node *expr = new_node(ND_NOT);
    expr->as.una.op = OP_NOT;
    expr->as.una.body = expr_prec(parser, PREC_NOT);
    expr->meta->type = &TYPE_BOOL;
    // 打印出AST
    trace_node(expr);
    return expr;
}

// 加载模块的内容
static Mod *load_mod(Parser *parser, Node *use) {
    // 构造文件名
    char* mod_name = use->as.use.mod;
    char *path = calloc(strlen(mod_name) + 3, sizeof(char));
    strcpy(path, mod_name);
    strcat(path, ".z");

    // 调用Front前端的do_file加载模块
    log_trace("Loading mod: %s\n", path);
    Mod *mod = do_file(parser->front, path);
    return mod;
}

static Node *use(Parser *parser) {
    advance(parser); // skip 'use'
    Node *expr = new_node(ND_USE);
    expr->as.use.mod = get_text(parser);
    Meta *m = new_meta(expr);
    expr->meta = m;
    scope_set(parser->scope, expr->as.use.mod, m);
    advance(parser); // skip TK_NAME
    if (match(parser, TK_DOT)) {
        advance(parser); // skip '.'
        expr->as.use.name = get_text(parser);
        advance(parser); // skip TK_NAME
    }
    // 就加载模块的内容
    if (expr->as.use.name == NULL) {
        load_mod(parser, expr);
    }
    return expr;
}

static Meta *do_meta(Parser *parser, Node *expr) {
    Meta *m = new_meta(expr);
    char *n = get_name(expr);
    scope_set(parser->scope, n, m);
    expr->meta = m;
    m->node = expr;
    return m;
}

static Type *type_lookup(Parser *parser, Node *type_name) {
    Meta *m = ident_lookup(parser, type_name);
    if (m == NULL) return NULL;
    return m->type;
}

static bool is_type_name(Parser *parser) {
    return parser->cur->kind == TK_INT ||
        parser->cur->kind == TK_BOOL ||
        parser->cur->kind == TK_FLOAT ||
        parser->cur->kind == TK_DOUBLE ||
        parser->cur->kind == TK_NAME;
}

static Node *symbol_def(Parser *parser, NodeKind kind) {
    // 跳过'let'
    advance(parser);
    Node *expr = new_node(kind);

    // 解析存量名称
    Node *store_name = new_node(ND_IDENT);
    store_name->as.path.names[0].name = get_text(parser);
    store_name->as.path.len = 1;
    expr->as.asn.name = store_name;
    advance(parser);

    Type *type = NULL;
    // 解析存量类型
    if (is_type_name(parser)) {
        Node *type_name = ident(parser);
        type = type_lookup(parser, type_name);
    }

    // 解析'='
    expect(parser, TK_ASN);

    // 解析数值表达式
    Node *value_node = expression(parser);
    expr->as.asn.value = value_node;
    if (type == NULL && value_node->meta) { // 如果没有指定类型，就尝试用右侧值的类型来推导
        type = value_node->meta->type;
    }

    // 收集元信息
    Meta *m = do_meta(parser, store_name);
    if (type) {
        m->type = type;
    } else {
        m->type = &TYPE_INT; // 当前阶段，默认类型是int
    }
    return expr;
}

static Node *let(Parser *parser) {
    return symbol_def(parser, ND_LET);
}

static Node *mut(Parser *parser) {
    return symbol_def(parser, ND_MUT);
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

static Node *block_impl(Parser *parser, bool peeked) {
    if (!peeked) expect(parser, TK_LBRACE);
    enter_scope(parser);
    Node *block = new_block();
    while (!match(parser, TK_RBRACE)) {
        append_expr(block, expression(parser));
        expect_eob(parser);
    }
    expect(parser, TK_RBRACE);
    exit_scope(parser);
    return block;
}


static Node *block(Parser *parser) {
    return block_impl(parser, false);
}

static Node *kv(Parser *parser, Node *left) {
    Node *entry = new_node(ND_KV);
    expect(parser, TK_COLON);
    Node *val = expression(parser);
    entry->as.kv.key = left;
    entry->as.kv.val = val;
    if (entry->meta == NULL) entry->meta = new_meta(entry);
    return entry;
}

static void expect_sep_dict(Parser *parser) {
    if (parser->cur->kind == TK_COMMA) {
        advance(parser);
    } else if (parser->cur->kind == TK_RBRACE) {
        return;
    } else {
        printf("Expected ',' or ']' between array items, but got %s\n", token_to_str(parser->cur->kind));
        exit(1);
    }
}

// 检查当前节点的类型，并尝试标注类型信息
static Type *infer_type(Parser *parser, Node *node) {
    // 如果已经有类型标注了，就直接返回
    // TODO: 这里要不要再检查一次？
    if (node->meta->type) return node->meta->type;
    // 先看看是不是基本类型节点
    Type *primary = check_primary_type(node);
    if (primary) {
        node->meta->type == primary;
        return primary;
    }
    // 如果是名称节点，就尝试查找类型
    if (node->kind == ND_IDENT) {
        Type *t = type_lookup(parser, node);
        if (t) {
            node->meta->type = t;
            return t;
        }
    }
    printf("Error: Unknown type for node: \n");
    print_node(node);
    exit(1);
}

static Node *dict_impl(Parser *parser, bool peeked) {
    Node *d = new_node(ND_DICT);
    if (!peeked) expect(parser, TK_LBRACE);
    if (parser->cur->kind == TK_RBRACE) { // 空字典
        return d;
    }
    d->as.dict.entries = new_hash_table();

    Type *key_type = &TYPE_STR;
    Type *val_type = NULL;
    while (!match(parser, TK_RBRACE)) {
        Node *key = new_name(parser);
        char *key_name = get_name(key);
        Node *entry = kv(parser, key);
        if (entry->kind != ND_KV) {
            printf("dict entry should be key:value form, got instead:");
            echo_node(entry);
            exit(1);
        }
        if (val_type == NULL) {
            Node *val = entry->as.kv.val;
            val_type = infer_type(parser, val);
        }
        hash_set(d->as.dict.entries, key_name, entry);
        expect_sep_dict(parser);
    }
    expect(parser, TK_RBRACE);
    // 处理类型信息
    Type *type = new_dict_type(key_type, val_type);
    if (d->meta == NULL) d->meta = new_meta(d);
    d->meta->type = type;
    return d;
}

// 注：为了区别字典和代码块，只能提前读取两个词符，通过`:`来区分
// 这样的解析就很不优雅，无法和其他地方单独解析`block`和`dict`的函数统一起来。
// 得想办法改进
static Node *block_or_dict(Parser *parser) {
    expect(parser, TK_LBRACE);
    //表达式以 `{name: ...}`开头，是一个字典 
    if (parser->cur->kind == TK_NAME && parser->next->kind == TK_COLON) { 
        return dict_impl(parser, true);
    } else {
        return block_impl(parser, true);
    }
}

static Node *dict(Parser *parser) {
    return dict_impl(parser, false);
}

static void expect_sep_array(Parser *parser) {
    if (parser->cur->kind == TK_COMMA) {
        advance(parser);
    } else if (parser->cur->kind == TK_RSQUARE) {
        return;
    } else {
        printf("Expected ',' or ']' between array items, but got %s\n", token_to_str(parser->cur->kind));
        exit(1);
    }
}

// 解析数组
static Node *array(Parser *parser) {
    // 跳过'['
    expect(parser, TK_LSQUARE);
    // 新建一个数组节点，其节点类型是ND_ARRAY
    Node *array = new_array();
    Type *item_type = NULL;
    // 解析各个元素
    while (!match(parser, TK_RSQUARE)) {
        Node *item = expression(parser);
        if (item_type == NULL) item_type = infer_type(parser, item);
        append_array_item(array, item);
        // 处理`,`间隔符
        expect_sep_array(parser);
    }
    // 处理类型信息
    Meta *meta = array->meta;
    if (item_type == NULL) {
        printf("Error: Cannot infer array item type\n");
        echo_node(array);
        exit(1);
    }
    meta->type = new_array_type(item_type, array->as.array.size);
    // 跳过`]`
    expect(parser, TK_RSQUARE);
    return array;
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

static void append_param(Params *p, Node *param) {
    if (p->count == p->cap) {
        p->cap *= 2;
        p->list = realloc(p->list, p->cap * sizeof(Node *));
    }
    p->list[p->count++] = param;
}

static Params *params(Parser *parser) {
    Params *p = calloc(1, sizeof(Params));
    p->count = 0;
    p->cap = 4;
    p->list = calloc(1, sizeof(Node *));
    while (parser->cur->kind != TK_RPAREN) {
        Node *pname = new_node(ND_IDENT);
        pname->as.path.names[0].name = get_text(parser);
        pname->as.path.len = 1;
        pname->meta = new_meta(pname);
        advance(parser);
        Type *ptype = NULL;
        if (!match(parser, TK_DOT)) {
            Node *type_name = ident(parser);
            ptype = type_lookup(parser, type_name);
        }
        pname->meta->type = ptype != NULL ? ptype : &TYPE_INT;
        append_param(p, pname);
        if (parser->cur->kind == TK_COMMA) {
            advance(parser); // 跳过','
        }
    }
    return p;
}

static bool is_method(Node *fname) {
    return fname->as.path.len > 1;
}

static char *get_class_name(Node *fname) {
    return fname->as.path.names[0].name;
}

static void enter_method_scope(Parser *parser, Type *class_type) {
    Scope *scope = new_scope(parser->scope);
    scope_set(scope, "self", new_type_meta(class_type));

    parser->scope = scope;
}

// 解析函数的定义
static Node *fn(Parser *parser) {
    advance(parser); // 跳过'fn'
    Node *fname = new_ident(parser); // 函数名，可能是简单函数，也可以是`类名.方法名`
    bool is_meth = is_method(fname); 
    Node *expr = new_node(ND_FN);
    expr->as.fn.name = get_full_name(fname);
    expr->as.fn.fname = fname;
    // 处理函数类型
    Type *fn_type = calloc(1, sizeof(Type));
    fn_type->kind = TY_FN;
    if (is_meth) {
        fn_type->as.fn.is_method = true;
        char *class_name = get_class_name(fname);
        fn_type->as.fn.class = type_lookup(parser, class_name);
        enter_method_scope(parser, fn_type);
    } else {
        enter_scope(parser);
    }
    expect(parser, TK_LPAREN);
    Params *p = params(parser);
    expr->as.fn.params = p;
    expect(parser, TK_RPAREN);
    // 在视野中设置参数名称
    for (int i = 0; i < p->count; i++) {
        Node *param = p->list[i];
        scope_set(parser->scope, get_full_name(param), new_meta(param));
    }
    fn_type->as.fn.param_count = p->count;
    fn_type->as.fn.params = calloc(p->count, sizeof(Type *));
    for (int i = 0; i < p->count; i++) {
        fn_type->as.fn.params[i] = p->list[i]->meta->type;
    }
    // 返回类型
    if (!match(parser, TK_LBRACE)) {
        Node *type_name = ident(parser);
        Type *ret_type = type_lookup(parser, type_name);
        fn_type->as.fn.ret = ret_type;
    } else {
        fn_type->as.fn.ret = &TYPE_INT; // 暂时默认返回类型是int，未来需要改为void
    }
    // 函数体
    expr->as.fn.body = block(parser);
    // 注意：函数体需要处理返回值
    Meta *body_meta = new_meta(expr->as.fn.body);
    body_meta->need_return = true;
    expr->as.fn.body->meta = body_meta;
    exit_scope(parser);
    Meta *m = do_meta(parser, expr);
    m->is_def = true;
    return expr;
}

static void expect_sep_type_fields(Parser *parser) {
    if (parser->cur->kind == TK_SEMI || parser->cur->kind == TK_NLINE) {
        advance(parser);
    } else if (parser->cur->kind == TK_RBRACE) {
        return;
    } else {
        printf("Expected ';' or '}' between type fields, but got %s\n", token_to_str(parser->cur->kind));
        exit(1);
    }
}

static void skip_new_line(Parser *parser) {
    while (parser->cur->kind == TK_NLINE) {
        advance(parser);
    }
}

static List *fields(Parser *parser) {
    List *fs = calloc(1, sizeof(List));
    fs->size = 0;
    fs->cap = 4;
    fs->items = calloc(1, sizeof(Node *));
    while (!match(parser, TK_RBRACE)) {
        skip_new_line(parser); // 跳过空行
        // 字段名称
        Node *field = new_name(parser); 
        Meta *field_meta = new_meta(field);
        field->meta = field_meta;

        // 字段类型
        Node *field_type = new_name(parser);
        Type *ftype = type_lookup(parser, field_type);
        if (!ftype) {
            printf("Error: field %s of type %s not found!", get_name(field), get_name(field_type));
            exit(1);
        } 
        field_meta->type = ftype;
        list_append(fs, field);
        expect_sep_type_fields(parser);
    }
    return fs;
}

static Node *type(Parser *parser) {
    advance(parser); // 跳过`type`关键字
    Node *type_decl = new_node(ND_TYPE);

    // 类型名称
    Node *type_name = new_name(parser);
    type_decl->as.type.name = type_name;
    char *tname = get_name(type_name);

    // 类型的各个字段
    expect(parser, TK_LBRACE);
    List *fs = fields(parser);
    type_decl->as.type.fields = fs;

    // 创建编译器中的UserType
    Type *tp = new_user_type(tname);
    tp->as.user.members = new_hash_table();
    for (int i = 0; i < fs->size; i++) {
        Node *field = fs->items[i];
        char *fname = get_name(field);
        // Type *ftype = field->meta->type;
        hash_set(tp->as.user.members, fname, field->meta);
    }
    Meta *tmeta = new_meta(type_decl);
    type_decl->meta = tmeta;
    tmeta->type = tp;
    expect(parser, TK_RBRACE);

    // 把类型的元信息存入到视野中
    scope_set(parser->scope, tname, tmeta);
    return type_decl;
}

static Node *single(Parser *parser) {
    switch (parser->cur->kind) {
        case TK_LBRACE:
            return block_or_dict(parser);
        case TK_LSQUARE:
            return array(parser);
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
        case TK_FN:
            return fn(parser);
        case TK_LPAREN:
            return group(parser);
        case TK_SUB:
            return neg(parser);
        case TK_NOT:
            return not(parser);
        case TK_STR:
            return string(parser);
        case TK_INT_NUM:
            return integer(parser);
        case TK_FLOAT_NUM:
            return float_num(parser);
        case TK_DOUBLE_NUM:
            return double_num(parser);
        case TK_TRUE:
            return bul(parser, true);
        case TK_FALSE:
            return bul(parser, false);
        case TK_NAME:
            return ident(parser);
        case TK_TYPE:
            return type(parser);
        default:
            printf("Unknown token: %s\n", token_to_str(parser->cur->kind));
            exit(1);
    }
}

static Type *get_type(Parser *parser, Node *node) {
    // 如果节点的meta直接可以获得类型，则返回该类型
    if (node->meta->type) {
        return node->meta->type;
    } else if (node->kind == ND_IDENT) { // 否则，尝试查找类型
        Type *type = type_lookup(parser, node);
        if (type) {
            node->meta->type = type;
            return type;
        }
    }
    return NULL;
}

// 数组下标访问
static Node *index(Parser *parser, Node *left) {
    advance(parser); // 跳过'['
    Node *idx = expression(parser);
    Node *node = new_node(ND_INDEX);
    node->as.index.parent = left;
    node->as.index.idx = idx;
    expect(parser, TK_RSQUARE); // 解析']'
    // 获取左侧的类型
    Type *left_type = get_type(parser, left);
    if (!left_type) return node;
    if (left_type->kind == TY_ARRAY) { // 如果是数组，则返回其元素类型
        node->meta->type = left->meta->type->as.array.item;
    } else if (left_type->kind == TY_DICT) { // 如果是字典，则返回其值类型
        node->meta->type = left->meta->type->as.dict.val;
    } else {
        printf("Error: Cannot index on a type that is neither a dict or an array: ");
        echo_node(left);
        exit(0);
    }
    return node;
}


// 对象字面量，如`Student{name: "zhangsan", age: 18}`
static Node *object(Parser *parser, Node *left) {
    if (left->kind != ND_IDENT) {
        printf("Error: Object name should be an ident, got instead:");
        echo_node(left);
        exit(0);
    }
    char *n = get_name(left);
    Meta *tmeta = scope_lookup(parser->scope, n);
    if (tmeta == NULL) {
        printf("Error: Unknown object type: %s\n", n);
        exit(0);
    }
    Node *obj = new_node(ND_OBJ);
    Node *dic = dict(parser);
    obj->as.obj.members = dic->as.dict.entries;
    obj->meta = tmeta;
    return obj;
}

static bool allow_postfix(NodeKind kind) {
    return kind == ND_IDENT || kind == ND_CALL || kind == ND_INDEX || kind == ND_OBJ;
}

static Node *unary(Parser *parser) {
    Node *expr = single(parser);
    if (!allow_postfix(expr->kind)) {
        return expr;
    }
    // 尝试解析后缀操作
    Node *res = expr;
    bool has_more_postfix = true;
    while (has_more_postfix) {
        switch (parser->cur->kind) {
        case TK_LPAREN:
            res = call(parser, res);
            break;
        case TK_LSQUARE:
            res = index(parser, res);
            break;
        case TK_LBRACE:
            res = object(parser, res);
            break;
        case TK_COLON:
            res = kv(parser, res);
            break;
        default:
            has_more_postfix = false;
        }
    }
    return res;
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
    case TK_DOT:
        return OP_DOT;
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
    case TK_DOT:
        return PREC_DOT;
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

bool is_relation_op(Op op) {
    return op == OP_GT || op == OP_LT || op == OP_GE || op == OP_LE || op == OP_EQ || op == OP_NE;
}

static Node *binop(Parser *parser, Node *left, Precedence base_prec) {
    Token *cur = parser->cur;
    // 如果下一个词符是运算符，那么应当是一个二元表达式
    if (is_binop(cur->kind)) {
        Precedence cur_prec = get_prec(cur->kind); // 当前操作符的优先级
        if (cur_prec < base_prec) {
            return left;
        }
        Node *bop = new_node(ND_BINOP);
        Op op = get_op(cur->kind);
        bop->as.bop.op = op;
        if (op == OP_ASN && left->kind == ND_IDENT) {
            left->kind = ND_LNAME;
        }
        bop->as.bop.left = left;
        // 二元表达式的结果类型应当可以由左子节点的类型和操作符推导出来
        Type *left_type = NULL;
        if (left->meta) left_type = left->meta->type;
        if (left_type) {
            bop->meta = new_meta(bop);
            if (is_relation_op(op)) {
                bop->meta->type = &TYPE_BOOL;
            } else {
                bop->meta->type = left_type;
            }
        }
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
    return binop(parser, left, base_prec);
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
Parser *new_parser(char *code, Scope *scope) {
    Parser *parser = calloc(1, sizeof(Parser));
    parser->lexer = new_lexer(code);
    parser->code = code;
    parser->cur = next_token(parser->lexer);
    parser->next = next_token(parser->lexer);
    parser->root_scope = scope == NULL ? new_scope(global_scope()) : scope;
    parser->scope = parser->root_scope;
    parser->uses = new_hash_table();
    return parser;
}