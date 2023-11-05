#include <stdio.h>
#include <string.h>

#include "interp.h"
#include "compiler.h"
#include "transpiler.h"
#include "util.h"

static void help(void) {
  printf("【用法】：./z <源码>|build <hello.z>|run <hello.z>\n");
}

int main(int argc, char** argv) {
    if (argc == 1) {
        repl();
        return 0;
    }
    log_trace("Hello from Z!\n");

    // 第一个参数是命令名称：interp|repl|build|run
    char *cmd = argv[1];

    // 如果命令是repl，直接进入repl()交互环境
    if (strcmp(cmd, "repl") == 0) {
        repl();
        return 0;
    }

    // 剩下的命令都需要提供内容（代码或文件名称）
    if (argc < 3) {
        help();
        return 1;
    }

    // 根据命令执行不同的操作
    if (strcmp(cmd, "interp") == 0) {
        interp(argv[2]);
    } else if (strcmp(cmd, "build") == 0) {
        build(argv[2]);
    } else if (strcmp(cmd, "run") == 0) {
        run(argv[2]);
    } else if (strcmp(cmd, "c") == 0) {
        trans_c(argv[2]);
    } else if (strcmp(cmd, "py") == 0) {
        trans_py(argv[2]);
    } else if (strcmp(cmd, "js") == 0) {
        trans_js(argv[2]);
    } else {
        help();
    }

    return 0;
}
