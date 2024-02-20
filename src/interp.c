#include <stdio.h>
#include "interp.h"
#include "parser.h"
#include "util.h"
#include "stdz.h"
#include "hash.h"
#include "builtin.h"
#include "meta.h"

static void set_val(char *name, Value *val) {
    hash_set(global_scope()->values, name, val);
}

static Value *get_val(char *name) {
    return hash_get(global_scope()->values, name);
}

Value *eval(Node *expr);

// 内置函数

// print
static void print(Node *arg) {
    print_val(eval(arg));
    printf("\n");
}

// pwd
static void pwd() {
    char buf[1024];
    getcwd(buf, sizeof(buf));
    printf("%s\n", buf);
}

// ls
static void ls(char *path) {
    char cmd[1024];
    sprintf(cmd, "ls %s", path);
    system(cmd);
}

// cd
static void cd(char *path) {
    if (chdir(path) != 0) {
        perror("chdir");
    }
}

// cat
static void cat(char *path) {
    read_file(path);
}

static bool check_num(Value *left, Value *right) {
    return left->kind == VAL_INT && right->kind == VAL_INT ||
        left->kind == VAL_FLOAT && right->kind == VAL_FLOAT ||
        left->kind == VAL_DOUBLE && right->kind == VAL_DOUBLE;
    ;
}

static bool check_bool(Value *left, Value *right) {
    return left->kind == VAL_BOOL && right->kind == VAL_BOOL;
}

static Value *eval_compare(Value *left, Value *right, Op op) {
    if (!check_num(left, right)) {
        printf("Type mismatch: %d %s %d\n", left->kind, op_to_str(op), right->kind);
        return new_nil();
    }
    if (left->kind == VAL_INT) {
        switch (op) {
        case OP_GT:
            return new_bool(left->as.num > right->as.num);
        case OP_LT:
            return new_bool(left->as.num < right->as.num);
        case OP_GE:
            return new_bool(left->as.num >= right->as.num);
        case OP_LE:
            return new_bool(left->as.num <= right->as.num);
        default:
            printf("Unknown operator for compare: %d %s %d\n", left->as.num, op_to_str(op), right->as.num);
            return new_nil();
        }
    } else if (left->kind == VAL_FLOAT) {
        switch (op) {
        case OP_GT:
            return new_bool(left->as.float_num > right->as.float_num);
        case OP_LT:
            return new_bool(left->as.float_num < right->as.float_num);
        case OP_GE:
            return new_bool(left->as.float_num >= right->as.float_num);
        case OP_LE:
            return new_bool(left->as.float_num <= right->as.float_num);
        default:
            printf("Unknown operator for compare: %f %s %f\n", left->as.float_num, op_to_str(op), right->as.float_num);
            return new_nil();
        }
    } else if (left->kind == VAL_DOUBLE) {
        switch (op) {
        case OP_GT:
            return new_bool(left->as.double_num > right->as.double_num);
        case OP_LT:
            return new_bool(left->as.double_num < right->as.double_num);
        case OP_GE:
            return new_bool(left->as.double_num >= right->as.double_num);
        case OP_LE:
            return new_bool(left->as.double_num <= right->as.double_num);
        default:
            printf("Unknown operator for compare: %lf %s %lf\n", left->as.double_num, op_to_str(op), right->as.double_num);
            return new_nil();
        }
    } else {
        printf("Unknown operator for compare: %d %s %d\n", left->as.num, op_to_str(op), right->as.num);
        return new_nil();
    }
}

static bool float_eq(float a, float b) {
    return a - b < 0.000001 && a - b > -0.000001;
}

static bool double_eq(double a, double b) {
    return a - b < 0.0000001 && a - b > -0.0000001;
}

static Value *eval_eq(Value *left, Value *right, Op op) {
    if (left->kind != right->kind) {
        printf("Type mismatch: %d %s %d\n", left->kind, op_to_str(op), right->kind);
        return new_nil();
    }
    switch (left->kind) {
    case VAL_INT: {
        switch(op) {
        case OP_EQ:
            return new_bool(left->as.num == right->as.num);
        case OP_NE:
            return new_bool(left->as.num != right->as.num);
        }
        break;
    }
    case VAL_BOOL:
        switch (op) {
        case OP_EQ:
            return new_bool(left->as.bul == right->as.bul);
        case OP_NE:
            return new_bool(left->as.bul != right->as.bul);
        }
        break;
    case VAL_FLOAT:
        switch(op) {
        case OP_EQ:
            return new_bool(float_eq(left->as.float_num, right->as.float_num));
        case OP_NE:
            return new_bool(!float_eq(left->as.float_num, right->as.float_num));
        }
        break;
    case VAL_DOUBLE:
        switch(op) {
        case OP_EQ:
            return new_bool(double_eq(left->as.double_num, right->as.double_num));
        case OP_NE:
            return new_bool(!double_eq(left->as.double_num, right->as.double_num));
        }
        break;
    default:
        printf("Unknown operator for eq: %d %s %d\n", left->as.num, op_to_str(op), right->as.num);
        return new_nil();
    }
}

static Value *eval_logic(Value *left, Value *right, Op op) {
    if (!check_bool(left, right)) {
        printf("Type mismatch: %d %s %d\n", left->kind, op_to_str(op), right->kind);
        return new_nil();
    }
    switch (op) {
    case OP_AND:
        return new_bool(left->as.bul && right->as.bul);
    case OP_OR:
        return new_bool(left->as.bul || right->as.bul);
    default:
        printf("Unknown operator: %d\n", op);
        return new_nil();
    }
}

bool call_builtin(Node *expr) {
    char *name = expr->as.call.name->as.str;
    if (strcmp(name, "print") == 0) {
        print(expr->as.call.args[0]);
        return true;
    } else if (strcmp(name, "pwd") == 0) {
        pwd();
        return true;
    } else if (strcmp(name, "ls") == 0) {
        ls(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(name, "cd") == 0) {
        cd(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(name, "cat") == 0) {
        cat(expr->as.call.args[0]->as.str);
        return true;
    }
    return false;
}

bool call_stdlib(Node *expr) {
    char *name = expr->as.call.name->as.str;
    if (strcmp(name, "read_file") == 0) {
        read_file(expr->as.call.args[0]->as.str);
        return true;
    } else if (strcmp(name, "write_file") == 0) {
        write_file(expr->as.call.args[0]->as.str, expr->as.call.args[1]->as.str);
        return true;
    }
    return false;
}

static Value *eval_hashtable(HashTable *ht) {
    HashTable *d = new_hash_table();
    HashIter* i = hash_iter(ht);
    while (hash_next(ht, i)) {
        char *key = i->key;
        Node *val_node = (Node*)i->value;
        Value *val = eval(val_node);
        hash_set(d, key, val);
    }
    Value *val = new_dict_val(d);
    return val;
}

// 对表达式求值
Value *eval(Node *expr) {
    switch (expr->kind) {
    case ND_STR:
        return new_str(expr->as.str);
    case ND_INT:
        return new_int(expr->as.num.val);
    case ND_FLOAT:
        return new_float(expr->as.float_num.val);
    case ND_DOUBLE:
        return new_double(expr->as.double_num.val);
    case ND_BOOL:
        return new_bool(expr->as.bul);
    case ND_NAME:
        return get_val(expr->as.str);
    case ND_NEG:
        return neg_val(eval(expr->as.una.body));
    case ND_NOT:
        return not(eval(expr->as.una.body));
    case ND_LET: {
        Value *val = eval(expr->as.asn.value);
        char *name = expr->as.asn.name->as.str;
        set_val(name, val);
        return val;
    }
    case ND_MUT: {
        Value *val = eval(expr->as.asn.value);
        char *name = expr->as.asn.name->as.str;
        set_val(name, val);
        return val;
    }
    case ND_BLOCK: {
        Value *last = NULL;
        for (int i = 0; i < expr->as.exprs.count; i++) {
            last = eval(expr->as.exprs.list[i]);
        }
        return last;
    }
    case ND_ARRAY: {
        Value *arr = new_array_val(expr->as.array.size);
        for (int i = 0; i < expr->as.array.size; i++) {
            arr->as.array->items[i] = eval(expr->as.array.items[i]);
        }
        return arr;
    }
    case ND_INDEX: {
        Value *parent = eval(expr->as.index.parent);
        Value *idx = eval(expr->as.index.idx);
        Type *left_type = expr->as.index.parent->meta->type;
        if (left_type->kind == TY_ARRAY) {
            if (idx->kind != VAL_INT) {
                printf("Array index must be int, but got %d\n", idx->kind);
                return new_nil();
            }
            int i = idx->as.num;
            if (i < 0 || i >= parent->as.array->size) {
                printf("Index out of range: ");
                echo_node(expr);
                return new_nil();
            }
            Value *item = parent->as.array->items[i];
            return item;
        } else if (left_type->kind == TY_DICT) {
            if (idx->kind != VAL_STR) {
                printf("Dict index must be string, but got %d\n", idx->kind);
                return new_nil();
            }
            char *key = idx->as.str;
            Value *val = hash_get(parent->as.dict->entries, key);
            return val;
        } else {
            return new_nil();
        }
    }
    case ND_OBJ: {
        return eval_hashtable(expr->as.obj.members);
    }
    case ND_DICT: {
        return eval_hashtable(expr->as.dict.entries);
    }
    case ND_IF: {
        Value *cond = eval(expr->as.if_else.cond);
        if (cond->kind != VAL_BOOL) {
            printf("Type mismatch: %d\n", cond->kind);
            return new_nil();
        }
        if (cond->as.bul) {
            return eval(expr->as.if_else.then);
        } else {
            return eval(expr->as.if_else.els);
        }
    }
    case ND_FOR: {
        Value *cond = eval(expr->as.loop.cond);
        if (cond->kind != VAL_BOOL) {
            printf("Type mismatch: %d\n", cond->kind);
            return new_nil();
        }
        while (cond->as.bul) {
            eval(expr->as.loop.body);
            cond = eval(expr->as.loop.cond);
        }
        // 没有实现数组和切片之前，for循环的返回值暂时当做nil;
        return new_nil();
    }
    case ND_FN: {
        Meta *m = expr->meta;
        if (m == NULL) {
            printf("Meta for %s is NULL\n", expr->as.fn.name);
            return new_nil();
        }
        Value *val = new_fn(&expr->as.fn);
        set_val(expr->as.fn.name, val);
        return val;
    }
    case ND_CALL: {
        if (call_builtin(expr)) return new_nil();
        if (call_stdlib(expr)) return new_nil();
        /*
        Meta *m = expr->meta;
        if (m == NULL) {
            printf("Unknown function: %s\n", expr->as.call.name->as.str);
            return new_nil();
        }
        Node *fn = m->node;
        */
        Value *val = get_val(expr->as.call.name->as.str);
        if (val == NULL) {
            printf("Unknown function: %s\n", expr->as.call.name->as.str);
            return new_nil();
        }
        Exprs *params = val->as.fn->params;
        for (int i = 0; i < params->count; ++i) {
            Node *p = params->list[i];
            char *n= p->as.str;
            set_val(n, eval(expr->as.call.args[i]));
        }
        return eval(val->as.fn->body);
    }
    case ND_TYPE: {
        Value *name = new_str(expr->as.type.name->as.str);
        return name;
    }
    case ND_BINOP: {
        BinOp *bop = &expr->as.bop;
        Value *res = NULL;
        switch (bop->op) {
        case OP_ADD:
            res = add_val(eval(bop->left), eval(bop->right));
            break;
        case OP_SUB:
            res = add_val(eval(bop->left),  neg_val(eval(bop->right)));
            break;
        case OP_MUL:
            res = mul_val(eval(bop->left), eval(bop->right));
            break;
        case OP_DIV:
            res = div_val(eval(bop->left), eval(bop->right));
            break;
        case OP_GT:
            res = eval_compare(eval(bop->left), eval(bop->right), OP_GT);
            break;
        case OP_LT:
            res = eval_compare(eval(bop->left), eval(bop->right), OP_LT);
            break;
        case OP_GE:
            res = eval_compare(eval(bop->left), eval(bop->right), OP_GE);
            break;
        case OP_LE:
            res = eval_compare(eval(bop->left), eval(bop->right), OP_LE);
            break;
        case OP_EQ:
            res = eval_eq(eval(bop->left), eval(bop->right), OP_EQ);
            break;
        case OP_NE:
            res = eval_eq(eval(bop->left), eval(bop->right), OP_NE);
            break;
        case OP_AND:
            res = eval_logic(eval(bop->left), eval(bop->right), OP_AND);
            break;
        case OP_OR:
            res = eval_logic(eval(bop->left), eval(bop->right), OP_OR);
            break;
        case OP_ASN:
            char *name = bop->left->as.str;
            res = eval(bop->right);
            set_val(name, res);
            break;
        default:
            printf("Unknown operator: %d\n", op_to_str(bop->op));
        }
        return res;
    }
    default:
        printf("Wrong NodeKind to eval: %d\n", expr->kind);
        return new_nil();
    }
}

// 执行AST
Value *execute(Node *expr) {
    switch (expr->kind) {
    case ND_PROG:
        Value *last = NULL;
        for (int i = 0; i < expr->as.exprs.count; i++) {
            last = execute(expr->as.exprs.list[i]);
        }
        // 返回最后一个表达式的值
        return last;
    default:
        return eval(expr);
    }
    return new_nil();
}

// 解释并执行代码
void interp(char *code) {
    Front *front = new_front();
    interp_once(front, code);
}

void interp_once(Front *front, char *code) {
    log_trace("Interpreting %s...\n", code);
    Mod *mod= do_code(front, code);
    Node *prog = mod->prog;
    log_trace("Executing ...\n------------------\n");
    Value *ret = execute(prog);
    if (ret->kind != VAL_NIL) {
        print_val(ret);
        printf("\n");
    }
}
