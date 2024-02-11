#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "util.h"

bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_alnum(char c) {
    return is_digit(c) || is_alpha(c);
}


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

bool starts_with(char *str, char *prefix) {
    int i = 0;
    while (*prefix) {
        if (*prefix++ != *str++) {
            return false;
        }
    }
    return true;
}

char *remove_ext(char *path) {
    char *ret_str, *last_ext, *last_path;
    // Error checks and allocate string.
    if (path == NULL) return NULL;
    if ((ret_str = malloc(strlen(path) + 1)) == NULL) return NULL;
    // Make a copy and find the relevant characters.
    strcpy(ret_str, path);
    last_ext = strrchr(ret_str, '.');
    last_path = ('/' == 0) ? NULL : strrchr (ret_str, '/');
    // If it has an extension separator.
    if (last_ext != NULL) {
        // and it's to the right of the path separator.
        if (last_path != NULL) {
            if (last_path < last_ext) {
                // then remove it.
                *last_ext = '\0';
            }
        } else {
            // Has extension separator with no path separator.
            *last_ext = '\0';
        }
    }
    // Return the modified string.
    return ret_str;
}

// 根据LOG_TRACE开关决定是否打印编译器详细信息
void log_trace(const char *fmt, ...) {
#ifdef LOG_TRACE
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#endif
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
    size_t n = fread(code, sizeof(char), size, fp);
    code[n] = '\0';
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


// 比较两个文件的内容
int compare_file(char *file1, char *file2) {
    FILE *fp1 = fopen(file1, "r");
    FILE *fp2 = fopen(file2, "r");
    if (fp1 == NULL || fp2 == NULL) {
        printf("似乎无法打开文件：%s 或 %s\n", file1, file2);
        exit(1);
    }
    int result = 0;
    for (;;) {
        int c1 = fgetc(fp1);
        int c2 = fgetc(fp2);
        // ignore '\r'
        if (c1 == '\r') {
            c1 = fgetc(fp1);
        }
        if (c2 == '\r') {
            c2 = fgetc(fp2);
        } 
        if (c1 == EOF && c2 == EOF) {
            break;
        }
        if (c1 != c2) {
            result = c2;
            break;
        }
    }
    fclose(fp1);
    fclose(fp2);
    return result;
}
    