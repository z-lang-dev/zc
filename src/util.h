#pragma once
#ifdef _WIN32
#include <stdint.h>
#include <stdio.h>
#endif

// 获取字符c在字符串中的位置
int index_of(char *str, char c);

// 提取一个子串
char *substr(char *str, int start, int end);

// 读取源码
char *read_src(char *file);

// 读取一行输入
#ifdef _WIN32
typedef intptr_t ssize_t;
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif