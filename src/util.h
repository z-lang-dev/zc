#pragma once

#include <stdbool.h>

#ifdef _WIN32
#include <stdint.h>
#include <stdio.h>
#endif


bool is_digit(char c);
bool is_alpha(char c);
bool is_alnum(char c);

// 获取字符c在字符串中的位置
int index_of(char *str, char c);

// 提取一个子串
char *substr(char *str, int start, int end);

bool starts_with(char *str, char *prefix);

// 根据LOG_TRACE开关决定是否打印信息
void log_trace(const char *fmt, ...);

// 读取源码
char *read_src(char *file);

// 读取一行输入
#ifdef _WIN32
typedef intptr_t ssize_t;
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

// 比较两个文件的内容
int compare_file(char *file1, char *file2);

// 删除字符串最后的文件扩展名
char *remove_ext(char *path);

char* sfmt(const char* fmt, char *msg);