#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "repl.h"
#include "util.h"
#include "parser.h"
#include "interp.h"

// REPL内置的命令
char *commands[] = {
    "print",
    "pwd",
    "ls",
    "cd",
    "cat",
    NULL
};

// 把命令转换成Z函数
static char *command(char *line) {
    for (int i = 0; ; i++) {
        char *cmd = commands[i];
        if (cmd == NULL) return line;
        if (starts_with(line, cmd)) {
            size_t len = strlen(cmd);
            char *res = calloc((int)strlen(line) + 16, sizeof(char));
            if (line[len] == '\0') { // 空命令，例如`ls`或者`pwd`
                if (strcmp(cmd, "ls") == 0) {
                    return "ls(\".\")";
                }
                sprintf(res, "%s()", cmd);
                return res;
            } else if (line[len] == ' ') { // 带参数的命令
                // 取得参数
                char *arg = substr(line, len + 1, strlen(line));
                if (!is_digit(arg[0])) { 
                    // 命令参数可以不带引号，但转换成Z函数时需要加上引号
                    sprintf(res, "%s(\"%s\")", cmd, arg);
                } else {
                    sprintf(res, "%s(%s)", cmd, arg);
                }
                printf("Got command: %s\n", res);
                return res;
            } else {
                return line;
            }
        }
    }
}

// 交互式环境REPL
void repl(void) {
    printf("Z REPL v0.1\n");

    for (;;) {
        printf("-------------- \n");
        printf(">>> ");
        char *line = NULL;
        size_t len = 0;
        int nread = getline(&line, &len, stdin);
        if (nread == -1) {
            printf("\n");
            break;
        }
        // 去掉行尾的换行符
        line[nread-1] = '\0';
        // 解析命令
        char *cmd = command(line);
        // 解析源码
        interp(cmd);
    }
}

