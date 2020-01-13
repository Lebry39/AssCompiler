#include <stdio.h>
#include <string.h>

struct name_t {
    char str[10];
    int num;
};

name_t fnc(){
    name_t a;
    strcpy(a.str, "UNK");
    a.num = 124;
    return a;
}

int main(int argc, char const *argv[]) {
    name_t arry[10];
    name_t a;
    name_t *b;

    strcpy(a.str, "hello");
    a.num = 5;

    arry[0] = a;
    b = &a;

    arry[1] = *b;

    for(int i=0; i<10; i++)
        printf("%s %d\n", arry[i].str, arry[i].num);

    printf("\n a %s %d\n", a.str, a.num);

    return 0;
}
