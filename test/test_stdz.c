#include <stdio.h>

#include "stdz.h"

int main(int argc, char **argv) {
    write_file("write_file_test.txt", "hello world\n");
    read_file("write_file_test.txt");
    return 0;
}