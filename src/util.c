#include "util.h"

#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#endif

// 获取字符c在字符串中的位置
int index_of(char *str, char c) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == c) {
            return i;
        }
        i++;
    }
    return -1;
}

// 提取一个子串
char *substr(char *str, int start, int end) {
    int len = end - start;
    char *result = calloc(len + 1, sizeof(char));
    int i = 0;
    while (i < len) {
        result[i] = str[start + i];
        i++;
    }
    result[i] = '\0';
    return result;
}


// 读取源码
char *read_src(char *file) {
    // 打开文件
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        printf("似乎无法打开文件：%s\n", file);
        exit(1);
    }
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // 读取文件内容
    char *code = calloc(size + 1, sizeof(char));
    fread(code, sizeof(char), size, fp);
    // 关闭文件
    fclose(fp);
    return code;
}

/* The original code is public domain -- Will Hartung 4/9/09 */
/* Modifications, public domain as well, by Antti Haapala, 11/10/17
   - Switched to getc on 5/23/19 */
// if typedef doesn't exist (msvc, blah)
#ifdef _WIN32

ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = malloc(128);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while(c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char *new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos ++] = c;
        if (c == '\n') {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}
#endif

