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

static Value *get_mod_val(Mod *mod, char *name) {
    return hash_get(mod->scope->values, name);
}

static void set_mod_val(Mod *mod, char *name, Value *val) {
    return hash_set(mod->scope->values, name, val);
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
    char *name = get_name(expr->as.call.name);
    if (strcmp(name, "print") == 0) {
        print(expr->as.call.args[0]);
        return true;
    } else if (strcmp(name, "pwd") == 0) {
        pwd();
        return true;
    } else if (strcmp(name, "ls") == 0) {
        ls(get_name(expr->as.call.args[0]));
        return true;
    } else if (strcmp(name, "cd") == 0) {
        cd(get_name(expr->as.call.args[0]));
        return true;
    } else if (strcmp(name, "cat") == 0) {
        cat(get_name(expr->as.call.args[0]));
        return true;
    }
    return false;
}

bool call_stdlib(Node *expr) {
    char *name = get_name(expr->as.call.name);
    if (strcmp(name, "read_file") == 0) {
        read_file(get_name(expr->as.call.args[0]));
        return true;
    } else if (strcmp(name, "write_file") == 0) {
        write_file(get_name(expr->as.call.args[0]), get_name(expr->as.call.args[1]));
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

Value *eval_asn(Node *expr) {
    if (expr->kind == ND_BINOP) {
        Node *left = expr->as.bop.left;
        Node *right = expr->as.bop.right;
        Value *res = eval(right);
        if (left->kind == ND_IDENT || left->kind == ND_LNAME) {
            char *name = get_name(left);
            set_val(name, res);
        } else if (left->kind == ND_INDEX) {
            // 由于这里得到的item本身就是个指针，我们可以把它当做左值来使用。
            Value *item = eval(left);
            if (item->kind != res->kind) {
                printf("Type mismatch: %d %s %d\n", item->kind, op_to_str(OP_ASN), res->kind);
                return new_nil();
            }
            // 直接修改item的值内容。
            item->as = res->as;
        }
        return res;
    }
    return new_nil();
}

// 获取一个名符对应的值
// 有三种不同类型的名符：
// 1. 简单的标识符，如a, b, c等
// 2. 模块成员，如http.server，math.PI等
// 3. 对象成员访问，如obj.name，obj.age等
static Value *get_ident_val(Node *expr) {

    // 简单名符
    if (expr->as.path.len <= 1) return get_val(get_name(expr));

    // 模块成员访问
    // TODO: 暂时只支持单层模块
    Name *head = &expr->as.path.names[0];
    switch (head->kind) {
    case NM_MOD:
        // TODO：到对应的模块中查找
        return get_val(get_name(expr));
    case NM_NAME: {
        Value *obj = get_val(head->name);
        // TODO: 暂时只支持单层成员查找，如p.x
        char *key = expr->as.path.names[1].name;
        Value *val = hash_get(obj->as.dict->entries, key);
        return val;
    }
    }
    return new_nil();
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
    case ND_IDENT:
        return get_ident_val(expr);
    case ND_NEG:
        return neg_val(eval(expr->as.una.body));
    case ND_NOT:
        return not(eval(expr->as.una.body));
    case ND_LET: 
    case ND_MUT: {
        Value *val = eval(expr->as.asn.value);
        char *name = get_name(expr->as.asn.name);
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
        // 计算出数组或字典的值，注意这一步大概率是根据名称从符号表中取出来的
        Value *parent = eval(expr->as.index.parent);
        // 计算出下标的值
        Value *idx = eval(expr->as.index.idx);
        // 获取左值（即数组或字典）的类型
        Type *left_type = expr->as.index.parent->meta->type;
        if (left_type->kind == TY_ARRAY) { // 数组类型，下标是整数
            if (idx->kind != VAL_INT) {
                printf("Array index must be int, but got %d\n", idx->kind);
                return new_nil();
            }
            int i = idx->as.num;
            // 判断数组越界
            if (i < 0 || i >= parent->as.array->size) {
                printf("Index out of range: ");
                echo_node(expr);
                return new_nil();
            }
            // 根据下标取得数组的元素
            Value *item = parent->as.array->items[i];
            return item;
        } else if (left_type->kind == TY_DICT) { // 字典类型，下标是字符串
            if (idx->kind != VAL_STR) {
                printf("Dict index must be string, but got %d\n", idx->kind);
                return new_nil();
            }
            // 从字典的entries中取出值
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
        Value *val = get_val(get_name(expr->as.call.name));
        if (val == NULL) {
            printf("Unknown function: %s\n", get_name(expr->as.call.name));
            return new_nil();
        }
        Exprs *params = val->as.fn->params;
        for (int i = 0; i < params->count; ++i) {
            Node *p = params->list[i];
            char *n= get_name(p);
            set_val(n, eval(expr->as.call.args[i]));
        }
        return eval(val->as.fn->body);
    }
    case ND_TYPE: {
        Value *name = new_str(get_name(expr->as.type.name));
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
            res = eval_asn(expr);
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
    if (ret == NULL) {
        printf("Error: returned NULL value\n");
    } else if(ret->kind != VAL_NIL) {
        print_val(ret);
        printf("\n");
    }
}
