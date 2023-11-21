#include <stdio.h>
#include "stdz.h"
#include "util.h"

// 打印字符串
void print_str(const char *str) {
    printf("%s\n", str);
}

// 打印整数
void print_int(int num) {
    printf("%d\n", num);
}

// 读取文件
void read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Error: cannot open file %s\n", path);
        exit(1);
    }
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) != -1) {
        printf("%s", line);
    }
    fclose(fp);
}

// 写入文件
void write_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        printf("Error: cannot open file %s\n", path);
        exit(1);
    }
    fprintf(fp, "%s", content);
    fclose(fp);
}