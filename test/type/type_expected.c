#include <stdio.h>

int main(void) {
    typedef struct {
        int x;
        int y;
    } Point;
    Point p = {.x = 3, .y = 4};
    printf("%d\n", p.x + p.y);
    return 0;
}
