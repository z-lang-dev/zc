#include <stdio.h>
#include "compiler.h"
#include "util.h"

int main(int argc, char** argv) {
    // 需要两个参数，一个是要编译的文件，例如hello.z；另一个是期望输出的内容，例如hello_expect.s
    if (argc < 3) {
        return -1;
    }
    // 编译文件并输出到app.s或app.asm
    build(argv[1]); // output to app.s/app.asm

    // 将app.s/app.asm与期望输出文件进行比较
    char *expected = argv[2];

#ifdef _WIN32
    return compare_file("app.asm", expected);
#else
    return compare_file("app.s", expected);
#endif
}