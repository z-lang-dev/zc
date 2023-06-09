#include "zc.h"
#include <stdarg.h>

static char *arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static FILE *fp;

Node *new_node_num(long val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = TYPE_INT;
  return node;
}

Node *new_ctcall_node(Node* def, Node* ctcall) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;

  Node head;
  Node *cur = &head;
  cur = cur->next = def;
  cur = cur->next = ctcall;
  node->body = head.next;

  Meta *meta= calloc(1, sizeof(Meta));
  meta->kind= META_FN;
  meta->type= fn_type(TYPE_INT);
  node->meta= meta;
 
  return node;
}

static void gen_expr(Node *node);

static void emit(char *fmt, ...) {
  va_list ap;
  fprintf(fp, "  ");
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
  fprintf(fp, "\n");
}

static void label(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
  fprintf(fp, ":\n");
}

static void comment(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(fp, "\t\t# ----- ");
  vfprintf(fp, fmt, ap);
  fprintf(fp, "\n");
  va_end(ap);
}

// 用来累计临时标签的值，区分同一段函数的不同标签
static int count(void) {
  static int i = 1;
  return i++;
}

// 将rax寄存器的值压入栈中
static void push(void) {
  emit("push rax");
}

// 将栈顶的值弹出到指定的寄存器中
static void pop(char *reg) {
  emit("pop %s", reg);
}

// 将地址中的值加载到rax寄存器中，需要在前一句代码中emit出地址，例如gen_addr或gen_deref
static void load(Type *type) {
  comment("Load.");
  // 数组类型的变量，没法直接load出整个数组的数据，而是只能获取到对应的第一个元素的指针。
  // 由于load()前面必然会获取地址，因此这里什么都不需要做
  if (type->kind == TY_ARRAY) {
    return;
  }
  // 将变量的值加载到rax寄存器中
  if (type->size == CHAR_SIZE) {
    emit("movsx rax, byte ptr [rax]");
  } else {
    emit("mov rax, [rax]");
  }
}

// 将rax寄存器的值存入到栈顶的地址中
static void store(void) {
  comment("Store.");
  pop("rdi");
  emit("mov [rdi], rax");
}

// 获取值量对应的局部地址，放在rax中
static void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_IDENT: {
    int offset = node->meta->offset;
    if (node->meta->is_global) {
      emit("lea rax, [rip-%d]", offset);
    } else {
      emit("lea rax, [rbp-%d]", offset);
    }
    return;
  }
  case ND_DEREF: {
    gen_expr(node->rhs);
    return;
  }
  case ND_STR: {
    // 获取全局标签地址的方法是[rip+标签名]
    emit("lea rax, [rip+%s]", node->meta->name);
    return;
  }
  default:
    error_tok(node->token, "【ZC错误】：不支持的地址类型：%d\n", node->kind);
  }
}

static void gen_expr(Node *node) {
  switch (node->kind) {
    case ND_IF: {
      int c = count();
      gen_expr(node->cond);
      emit("cmp rax, 0");
      emit("je .L.else.%d", c);
      gen_expr(node->then);
      emit("jmp .L.end.%d", c);
      emit(".L.else.%d:", c);
      if (node->els) {
        gen_expr(node->els);
      }
      emit(".L.end.%d:", c);
      return;
    }
    case ND_FOR: {
      int c = count();
      emit(".L.begin.%d:", c);
      gen_expr(node->cond);
      emit("cmp rax, 0");
      emit("je .L.end.%d", c);
      gen_expr(node->body);
      emit("jmp .L.begin.%d", c);
      emit(".L.end.%d:", c);
      return;
    }
    case ND_FN:
      // 这里什么都不做，而是要等当前所在的scope结束之后，再单独生成函数定义相对应的代码
      return;
    case ND_STR:
      // 这里什么都不做，而是单独用gen_const()函数生成字符串常量的代码
      gen_addr(node);
      return;
    case ND_CALL: {
      int nargs = 0;
      for (Node *arg = node->args; arg; arg = arg->next) {
        comment("Arg <%ld>", arg->val);
        gen_expr(arg);
        push();
        nargs++;
      }

      for (int i = nargs - 1; i >= 0; i--) {
        pop(arg_regs[i]);
      }

      comment("Calling %s()", node->meta->name);
      emit("mov rax, 0");
      emit("call %s", node->meta->name);
      return;
    }
    case ND_CTCALL: {
      Node *def = node->meta->def;
      Node *prog = new_ctcall_node(def, node);
      Value* val = interpret(prog);
      long n = val->as.num;
      Node *node = new_node_num(n);
      gen_expr(node);
      return;
    }
    case ND_BLOCK: {
      for (Node *n=node->body; n; n=n->next) {
        gen_expr(n);
      }
      return;
    }
    case ND_NUM:
      emit("mov rax, %ld", node->val);
      return;
    case ND_CHAR:
      emit("mov rax, '%c'", node->cha);
      return;
    case ND_NEG:
      gen_expr(node->rhs);
      emit("neg rax");
      return;
    case ND_IDENT:
      gen_addr(node);
      load(node->type);
      return;
    case ND_ASN:
      comment("Assignment.");
      gen_addr(node->lhs);
      push();
      gen_expr(node->rhs);
      store();
      return;
    case ND_ADDR:
      comment("Get addr for pointer.");
      gen_addr(node->rhs);
      return;
    case ND_DEREF:
      comment("Deref a pointer.");
      gen_expr(node->rhs);
      load(node->type);
      return;
    case ND_ARRAY: {
      comment("Array literal.");
      // 如果数组长度为1，那么处理方式和普通值量一样，只需要对节点的第一个元素求值即可
      if (node->len == 1) {
        gen_expr(node->elems);
        return;
      }

      // 如果有多个元素，则每个元素都需要求值，然后调用store()存入栈上分配的空间里。
      // 注意，每个元素的地址要递增
      size_t k = 0;
      for (Node *n=node->elems; n; n=n->next) {
        gen_expr(n);
        // 递增rdi中的地址，并存到栈顶。这是因为store()函数是从栈顶获取地址，并放到rdi中进行计算的
        // TODO: 优化store()和load()函数，减少地址压栈操作
        if (k < node->len - 1) {
          store();
          comment("Increment address in rdi.");
          emit("add rdi, %ld", n->type->size);
        } else {
          comment("Last addr for array.");
        }
        emit("push rdi");
        k++;
      }
      return;
    }
    case ND_INDEX: {
      comment("Array index.");
      // array
      Node *array_ident = node->lhs;
      gen_expr(array_ident);
      push();
      // index
      gen_expr(node->rhs);
      push();

      Type *elem_type = array_ident->type->target;

      // 获取地址偏移
      pop("rdi");
      pop("rax");
      emit("imul rdi, %d", elem_type->size);
      emit("add rax, rdi");

      load(node->lhs->type->target);
      return;
    }
    case ND_USE:
      return;
    default:
      break;
  }

  // 计算左侧结果并压栈
  gen_expr(node->lhs);
  push();
  // 计算右侧结果并压栈
  gen_expr(node->rhs);
  push();
  // 把盏顶的两个值弹出到rax和rdi
  pop("rdi");
  pop("rax");
  // TODO: 上面的计算如果左右顺序反过来，就可以节省一次push和pop，未来可以考虑优化

  // 执行计算
  switch (node->kind) {
    case ND_PLUS: {
      // ptr + num
      if (node->lhs->type->kind == TY_PTR) {
        // 如果加法左侧是指针类型，那么所加的值应当乘以8，即ptr+1相当于地址移动8个字节
        emit("imul rdi, %d", OFFSET_SIZE /*node->lhs->type->target->size*/);
      }

      emit("add rax, rdi");
      return;
    }
    case ND_MINUS: {
      // ptr - num
      if (is_ptr(node->lhs->type) && is_num(node->rhs->type)) {
        // 如果减法左侧是指针类型，那么所减的值应当乘以8，即ptr-1相当于地址移动-8个字节
        emit("imul rdi, %d", OFFSET_SIZE /*node->lhs->type->target->size*/);
      }
      emit("sub rax, rdi");
      // ptr - ptr
      if (is_ptr(node->lhs->type) && is_ptr(node->rhs->type)) {
        emit("cqo");
        emit("mov rdi, %d", OFFSET_SIZE /*node->lhs->type->target->size*/);
        emit("idiv rdi");
        return;
      }
      return;
    }
    case ND_MUL:
      emit("imul rax, rdi");
      return;
    case ND_DIV:
      emit("cqo");
      emit("idiv rdi");
      return;
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE: {
      emit("cmp rax, rdi");
      if (node->kind== ND_EQ) {
        emit("sete al");
      } else if (node->kind== ND_NE) {
        emit("setne al");
      } else if (node->kind== ND_LT) {
        emit("setl al");
      } else if (node->kind== ND_LE) {
        emit("setle al");
      }
      emit("movzx rax, al");
      return;
    }
    default:
      error_tok(node->token, "【CodeGen错误】：不支持的运算符：%c\n", node->kind);
  }

}

// 把n对齐到align的倍数，例如 align_to(13, 8) => 16
static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

static void set_local_offsets(Meta *fmeta) {
  int offset = 0;
  for (Meta *meta= fmeta->region->locals; meta; meta=meta->next) {
    if (meta->kind == META_LET) {
      // 注意，这里数组的size实际是(元素尺寸*len)
      size_t size = meta->type->size;
      if (meta->type->kind == TY_STR) {
        size = OFFSET_SIZE;
      }
      offset += size;
      meta->offset = offset;
    }
  }
  fmeta->stack_size = align_to(offset, 16);
}

static void gen_fn(Meta *meta) {
  emit("\t\t# ===== [Define Function: %s]", meta->name);
  set_local_offsets(meta);
  emit("\n  .global %s", meta->name);
  emit("%s:", meta->name);

  // Prologue
  comment("Prologue");
  emit("push rbp");
  emit("mov rbp, rsp");
  emit("sub rsp, %zu", meta->stack_size);

  // 处理参数
  comment("Handle params");
  int i = 0;
  for (Meta *p = meta->params; p; p = p->next) {
    emit("mov [rbp-%d], %s", p->offset, arg_regs[i++]);
  }

  comment("Function body");
  // 生成函数体
  for (Node *n = meta->body; n; n = n->next) {
    gen_expr(n);
  }

  // Epilogue
  comment("Epilogue");
  emit(".L.return.%s:", meta->name);
  emit("mov rsp, rbp");
  emit("pop rbp");
  emit("ret");
}


static void gen_global_data(Meta* meta) {
  if (meta->kind == META_CONST) {
    emit(".globl %s", meta->name);
    label("%s", meta->name);

    if (meta->type->kind == TY_ARRAY || meta->type->kind == TY_STR) {
      for (size_t i = 0; i < meta->len; i++) {
        emit(".byte %d", meta->str[i]);
      }
    } else { 
      emit(".zero %zu", meta->type->size);
    }
  }
}

void codegen_main(Node *prog) {
  // 打开目标汇编文件，并写入汇编代码
  fp = fopen("app.s", "w");

  set_local_offsets(prog->meta);

  emit(".intel_syntax noprefix");

  // 生成自定义函数的代码
  bool has_global_data = false;
  Meta* mainFn = NULL;
  for (Meta *meta= prog->meta->region->locals; meta; meta=meta->next) {
    if (meta->kind== META_FN) {
      if (strcmp(meta->name, "main") == 0) {
        mainFn = meta;
      } else {
        // 单独声明没有定义的话，就不处理了。
        if (meta->is_decl) {
          continue;
        }
        gen_fn(meta);
      }
    } else if (meta->kind == META_CONST) {
      if (!has_global_data) {
        emit(".data");
        has_global_data = true;
      }
      gen_global_data(meta);
    }
  }

  emit(".text");
  emit(".global main");
  label("main");


  // Prologue
  emit("push rbp");
  emit("mov rbp, rsp");
  emit("sub rsp, %zu", prog->meta->stack_size);

  for (Node *n = prog->body; n; n = n->next) {
    gen_expr(n);
  }

  // 如果有main定义，在这里生成
  if (mainFn) {
    for (Node *n = mainFn->body; n; n = n->next) {
      gen_expr(n);
    }
  }

  // Epilogue
  emit("mov rsp, rbp");
  emit("pop rbp");
  emit("ret");

  fclose(fp);
}

void codegen_lib(Box *b) {
  char *buf = calloc(1, 1024);
  sprintf(buf, "%s.s", b->name);
  fp = fopen(buf, "w");

  set_local_offsets(b->prog->meta);

  emit(".intel_syntax noprefix");

  // 生成自定义函数的代码
  bool has_global_data = false;
  for (Meta *meta= b->prog->meta->region->locals; meta; meta=meta->next) {
    if (meta->kind== META_FN) {
      if (strcmp(meta->name, "main") == 0) {
        printf("DEBUG: Got main definition in lib, ignored.\n");
      } else {
        // 单独声明没有定义的话，就不处理了。
        if (meta->is_decl) {
          continue;
        }
        gen_fn(meta);
      }
    } else if (meta->kind == META_CONST) {
      if (!has_global_data) {
        emit(".data");
        has_global_data = true;
      }
      gen_global_data(meta);
    }
  }

  fclose(fp);
}


void codegen_box(Box *b) {

  // 生成use引用到的模块
  for (Box *bo = all_boxes(); bo; bo = bo->next) {
    // 忽略掉主模块
    if (strcmp(bo->name, b->name) == 0) {
      continue;
    }
    codegen_lib(bo);
  }

  // 生成主模块的代码
  codegen_main(b->prog);
}

