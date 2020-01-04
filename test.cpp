#include <stdio.h>

int main() {
    union {
        int addr;
        char value;
        int code;
    }u;

    u.addr = 6122;

    printf("%d\n", u.value);

    return 0;
}
