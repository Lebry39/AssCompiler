#include <stdio.h>

#include "typedefine.h"
#include "stackmachine.h"

int main(int argc, char const *argv[]) {
    instraction code[100];
    int i = -1;
    int is_invalid_code = 0;

    if(argc == 1){
        printf("ERROR: Plz input source's file.\n");
        return -1;
    }

    is_invalid_code = read_code((char*)argv[1], code) == -1;
    if(is_invalid_code){
        return -1;
    }

    print_code(code);
    execute_code(code);

    return 0;
}
