#include <stdio.h>
#include "interp.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Error: args: <z code>\n");
        return -1;
    }
    char *code = argv[1];
    interp(code);
    return 0;
}