#include <stdio.h>
#include <string.h>
#include "util.h"
#include "transpiler.h"

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("Error: args: c|py|js <hello.z> <hello_expect.c>\n");
        return -1;
    }
    char *cmd = argv[1];
    if (strcmp(cmd, "c") == 0) {
        trans_c(argv[2]);
        return compare_file("app.c", argv[3]);
    } else if (strcmp(cmd, "py") == 0) {
        trans_py(argv[2]);
        return compare_file("app.py", argv[3]);
    } else {
        trans_js(argv[2]);
        return compare_file("app.js", argv[3]);
    }
}