#include <stdio.h>

int main(void) {
    printf("{1, 2, 3}\n");
    double a[3] = {1.1, 2.1, 3.1};
    a[1] = 4.2;
    a[2] = 5.2;
    printf("%lf\n", a[2]);
    int b[3][2] = {{1, 2}, {4, 5}, {7, 8}};
    b[0][0] = 10;
    b[2][1] = 90;
    // print(b)
    printf("{");
    for (int i = 0; i < 3; ++i) {
        if (i > 0) printf(", ");
        printf("{");
        for (int j = 0; j < 2; ++j) {
            if (j > 0) printf(", ");
            printf("%d", b[i][j]);
        }
        printf("}");
    }
    printf("}");
    return 0;
}
