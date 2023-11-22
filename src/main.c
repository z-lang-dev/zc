#include <stdio.h>
#include <string.h>

#include "interp.h"
#include "compiler.h"
#include "transpiler.h"
#include "util.h"

static void help(void) {
  printf("【用法】：`z <源码>` 或 `z repl` 或\n `z interp <源码>` 或\n `z build <文件.z>` 或\n `z c|py|js <hello.z>\n");
}

static void help_run(void) {
  printf("【用法】：./z run c|py|js <hello.z>\n");
}

void run(char *target, char *file) {
    if (strcmp(target, "py") == 0) {
        log_trace("Building %s...\n", file);
        trans_py(file);
        log_trace("\nRunning %s...\n", file);
        system("python app.py");
        return;
    }
    if (strcmp(target, "js") == 0) {
        log_trace("Building %s...\n", file);
        trans_js(file);
        log_trace("\nRunning %s...\n", file);
        system("node app.js");
        return;
    }
    if (strcmp(target, "c") == 0) {
        log_trace("Building %s...\n", file);
        trans_c(file);
#ifdef _WIN32
        system("cl app.c");
        log_trace("\nRunning %s...\n", file);
        system("app.exe");
#else
        system("clang app.c -o app.exe");
        log_trace("\nRunning %s...\n", file);
        system("./app.exe");
#endif
        return;
    }
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
    } else if (strcmp(cmd, "help") == 0) {
        help();
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
        if (argc < 4) {
            help_run();
            return 1;
        }
        run(argv[2], argv[3]);
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
