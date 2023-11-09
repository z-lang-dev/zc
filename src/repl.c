#include <stdio.h>

#include "repl.h"
#include "util.h"
#include "parser.h"
#include "interp.h"

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
        // 解析源码
        Parser *parser = new_parser(line);
        Node *expr = parse(parser);
        // 执行
        execute(expr);
    }
}

