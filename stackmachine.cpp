#include <stdio.h>

#define CODE_N  255
#define STACK_N  255
#define MAX_LEVEL  64

enum functype {
    lod, sto, cal, ret,  // Func, Level, Locate
    lit, inc, jmp, jpc,  // Func, Value
    opr, end  // opr, oprtype
};

enum oprtype {
    add, sub, mul, div,  // 演算
    eq, neq, ls, lt, gr, gt,  // 条件
    wrt, wrl  // printf
};

struct address{
    int level;
    int addr;
};

typedef struct inst{
    functype func;  // 命令
    union{
        int value;
        address addr;
        oprtype opcode;
    } u;
} instraction;

void execute(instraction *code){
    int stack[STACK_N];
    int disp[MAX_LEVEL];
    disp[0] = 0;

    instraction ireg;

    int sp = 0;  // スタックポインタ
    int ip = 0;  // インタラクションポインタ
    int lev = 0;  // 実行中部分のレベル

    while(1){
        if(ip >= CODE_N){
            printf("Error: Too many code.\n");
            return;
        }
        ireg = code[ip++];
        switch(ireg.func){
            case lod:
                stack[sp++] = stack[disp[ireg.u.addr.level] + ireg.u.addr.addr];
                break;
            case sto:
                stack[disp[lev] + ireg.u.addr.addr] = stack[--sp];
                break;
            case cal:
                break;
            case ret:
                break;
            case lit:
                stack[sp++] = ireg.u.value;
                break;
            case inc:
                sp += ireg.u.value;
                break;
            case jmp:
                ip = ireg.u.addr.addr;
                continue;
            case jpc:
                if(stack[--sp] != 0){
                    ip = ireg.u.addr.addr;
                    continue;
                }
                break;

            case opr:
                if(ireg.u.opcode == add){  // 演算
                    stack[sp-2] = stack[sp-2] + stack[sp-1];
                    sp--;
                }else if(ireg.u.opcode == sub){
                    stack[sp-2] = stack[sp-2] - stack[sp-1];
                    sp--;
                }else if(ireg.u.opcode == mul){
                    stack[sp-2] = stack[sp-2] * stack[sp-1];
                    sp--;
                }else if(ireg.u.opcode == div){
                    stack[sp-2] = stack[sp-2] / stack[sp-1];
                    sp--;

                }else if(ireg.u.opcode == eq){  // 比較
                    stack[sp-2] = stack[sp-2] == stack[sp-1];
                    sp--;
                }else if(ireg.u.opcode == neq){
                    stack[sp-2] = stack[sp-2] != stack[sp-1];
                    sp--;
                }else if(ireg.u.opcode == ls){
                    stack[sp-2] = stack[sp-2] < stack[sp-1];
                    sp--;
                }else if(ireg.u.opcode == lt){
                    stack[sp-2] = stack[sp-2] <= stack[sp-1];
                    sp--;
                }else if(ireg.u.opcode == gr){
                    stack[sp-2] = stack[sp-2] > stack[sp-1];
                    sp--;
                }else if(ireg.u.opcode == gt){
                    stack[sp-2] = stack[sp-2] >= stack[sp-1];
                    sp--;

                }else if(ireg.u.opcode == wrt){  // 表示
                    printf("%d", stack[--sp]);
                }else if(ireg.u.opcode == wrl){
                    printf("%d\n", stack[--sp]);
                }
                break;

            default:
                return;

        }
    }
}

void print_code(instraction *code){
    instraction ireg;
    int ip = 0;

    while(1){
        if(ip >= CODE_N){
            printf("Error: Too many code.\n");
            return;
        }
        ireg = code[ip++];

        printf("%4d: ", ip);
        switch(ireg.func){
            case lod: printf("lod"); break;
            case sto: printf("sto"); break;
            case cal: printf("cal"); break;
            case ret: printf("ret"); break;
            case lit: printf("lit"); break;
            case inc: printf("inc"); break;
            case jmp: printf("jmp"); break;
            case jpc: printf("jpc"); break;
            case opr: printf("opr"); break;
            default:
                printf("end\n\n");
                return;
        }

        printf(", ");

        if(ireg.func != opr){
            if(ireg.func == sto){
                printf("%d, %d", ireg.u.addr.level, ireg.u.addr.addr);
            }else{
                printf("%d", ireg.u.value);
            }
        }else{
            if(ireg.u.opcode == add){       // 演算
                printf("add");
            }else if(ireg.u.opcode == sub){
                printf("sub");
            }else if(ireg.u.opcode == mul){
                printf("mul");
            }else if(ireg.u.opcode == div){
                printf("div");
            }else if(ireg.u.opcode == eq){  // 比較
                printf(" eq");
            }else if(ireg.u.opcode == neq){
                printf("neq");
            }else if(ireg.u.opcode == ls){
                printf(" ls");
            }else if(ireg.u.opcode == lt){
                printf(" lt");
            }else if(ireg.u.opcode == gr){
                printf(" gr");
            }else if(ireg.u.opcode == gt){
                printf(" gt");
            }else if(ireg.u.opcode == wrt){  // 表示
                printf("wrt");
            }else if(ireg.u.opcode == wrl){
                printf("wrl");
            }
        }
        printf("\n");
    }
}

int main() {
    instraction code[CODE_N];
    // 5 * 3 = 15

    code[0].func = lit;
    code[0].u.value = 3;

    code[1].func = lit;
    code[1].u.value = 7;

    code[2].func = opr;
    code[2].u.opcode = mul;

    code[3].func = opr;
    code[3].u.opcode = wrl;

    code[4].func = end;

    print_code(code);
    execute(code);

    return 0;
}
