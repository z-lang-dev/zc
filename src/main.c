#include <stdio.h>

static void help(void) {
  printf("【用法】：./z <源码>|build <hello.z>|run <hello.z>\n");
}

static void interp(char *code) {
    printf("TODO: interp %s\n", code);
}

static void repl(void) {
    printf("TODO: repl\n");
}

static void build(char *file) {
    printf("TODO: building %s\n", file);
}

static void run(char *file) {
    printf("TODO: run %s\n", file);
}

int main(int argc, char** argv) {
    if (argc == 1) {
        repl();
        return 0;
    }
    printf("Hello from Z!\n");

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
    } else {
        help();
    }

    return 0;
}
