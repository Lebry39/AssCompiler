#include <stdio.h>
#include <string.h>

#define CODE_N 255
#define STACK_N 255
#define MAX_LEVEL 64
#define MAX_LINE 64

enum functype {
    ign,    // ignore
    lod, sto, cal, ret,  // Func, Level, Locate
    lit, inc, jmp, jpc,  // Func, Value
    opr,    // opr, oprtype
    cpy,    // cpy num
    nop,    // Do nothing
    end     // End code
};

enum oprtype {
    add, sub, mul, div,       // 演算
    eq, neq, ls, lt, gr, gt,  // 条件
    wrt, wrl                  // printf
};

struct address{
    int level;
    int addr;
};

typedef struct inst{
    functype func;      // 命令
    union{
        address addr;   // lod, sto, cal, ret で使う
        int value;      // lit, inc, jmp, jmc で使う
        oprtype opcode; // add, sub, mul,... 演算、比較で使う
    } u;
} instraction;

// stdlib を include するとdivの定義が使えなくなるから直書き
int atoi(char s[]) {
    int i, n, sign;

    for( i = 0; s[i]==' '; i++ );

    sign = ( s[i] == '-' ) ? -1 : 1;
    if( s[i] == '-' || s[i] == '+' )
        i++;
    for( n = 0; '0' <= s[i] && s[i] <= '9'; i++)
        n = 10 * n + ( s[i] - '0' );
    return sign * n;
}

int is_empty(char c){
    switch (c) {
        case ' ':
        case '\n':
        case ',':
        case '\t':
            return 1;
        default:
            break;
    }
    return 0;
}

void dump_stack(int *stack, int sp){
    int i;
    printf("--- Dump ---\n");
    for(i=0; i<sp; i++){
        printf("%d: %d\n", i, stack[i]);
    }
    printf("------------\n");
}

void execute(instraction *code){
    int stack[STACK_N];
    int disp[MAX_LEVEL];

    instraction ireg;  // 実行する命令
    int sp = 0;   // スタックポインタ
    int ip = 0;   // インタラクションポインタ

    int i, lev, ret_val;

    // Initialize
    disp[0] = 0;
    stack[0] = 0;

    printf(">> Execute\n");

    while(1){
        if(ip >= CODE_N){
            printf("Error: Too many code.\n");
            return;
        }
        if(sp >= STACK_N){
            printf("Error: Stack Over fllow.\n");
            return;
        }

        // dump_stack(stack, sp);

        ireg = code[ip++];
        switch(ireg.func){
            case nop:
                continue;
            case cpy:
                for(i=0; i<ireg.u.value; i++){
                    stack[sp++] = stack[sp-2];
                }
                break;
            case lod:
                // Push
                stack[sp++] = stack[disp[ireg.u.addr.level] + ireg.u.addr.addr];
                break;
            case sto:
                // Pop
                stack[disp[ireg.u.addr.level] + ireg.u.addr.addr] = stack[--sp];
                break;
            case cal:
                // Call
                lev = ireg.u.addr.level + 1;
                stack[sp] = disp[lev];  // disp の退避
                stack[sp+1] = ip;       // 戻り先codeの退避
                disp[lev] = sp;         // spの退避
                ip = ireg.u.addr.addr;  // サブルーチンアドレスへ
                break;
            case ret:
                // Return
                ret_val = stack[--sp];  // 戻り値を一時領域へ

                lev = ireg.u.addr.level;
                sp = disp[lev];         // sp の回復
                disp[lev] = stack[sp];  // disp の回復
                ip = stack[sp+1];       // 戻りcodeアドレスへ
                sp -= ireg.u.addr.addr; // 引数を削除

                stack[sp++] = ret_val;  // 戻り値をpush
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
                }
                continue;

            case opr:
                switch (ireg.u.opcode) {
                    case add:
                        stack[sp-2] = stack[sp-2] + stack[sp-1];
                        sp--; break;
                    case sub:
                        stack[sp-2] = stack[sp-2] - stack[sp-1];
                        sp--; break;
                    case mul:
                        stack[sp-2] = stack[sp-2] * stack[sp-1];
                        sp--; break;
                    case div:
                        stack[sp-2] = stack[sp-2] / stack[sp-1];
                        sp--; break;
                    case eq:
                        stack[sp-2] = stack[sp-2] == stack[sp-1];
                        sp--; break;
                    case neq:
                        stack[sp-2] = stack[sp-2] != stack[sp-1];
                        sp--; break;
                    case ls:
                        stack[sp-2] = stack[sp-2] < stack[sp-1];
                        sp--; break;
                    case lt:
                        stack[sp-2] = stack[sp-2] <= stack[sp-1];
                        sp--; break;
                    case gr:
                        stack[sp-2] = stack[sp-2] > stack[sp-1];
                        sp--; break;
                    case gt:
                        stack[sp-2] = stack[sp-2] <= stack[sp-1];
                        sp--; break;
                    case wrt:
                        printf("%d", stack[--sp]);
                        break;
                    case wrl:
                        printf("\n");
                        break;
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

        printf("%4d: ", ip-1);
        switch(ireg.func){
            case cpy: printf("cpy"); break;
            case nop: printf("nop"); break;
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
            switch (ireg.func) {
                case lod:
                case sto:
                case cal:
                case ret:
                    printf("%d, %d", ireg.u.addr.level, ireg.u.addr.addr);
                    break;
                case jpc:
                case jmp:
                    printf("%d", ireg.u.addr.addr);
                    break;
                case lit:
                case inc:
                case cpy:
                    printf("%d", ireg.u.value);
                    break;
                case nop:
                default:
                    break;
            }

        }else{
            switch (ireg.u.opcode) {
                case add :
                    printf("add"); break;
                case sub :
                    printf("sub"); break;
                case mul :
                    printf("mul"); break;
                case div :
                    printf("div"); break;
                case eq :
                    printf("eq");  break;
                case neq :
                    printf("neq"); break;
                case ls :
                    printf("ls");  break;
                case lt :
                    printf("lt");  break;
                case gr :
                    printf("gr");  break;
                case gt :
                    printf("gt");  break;
                case wrt :
                    printf("wrt"); break;
                case wrl :
                    printf("wrl"); break;
            }
        }
        printf("\n");
    }
}

instraction gen_line(char *line){
    instraction inst;
    int lp = 0;
    int i;
    char func[4];
    char a[MAX_LINE];
    char b[MAX_LINE];

    a[0] = '\0';
    b[0] = '\0';

    // line = "func, a, b"

    // 空白を捨てる
    while(is_empty(line[lp])) lp++;

    // 何もない行及びコメントのための";"を無視する
    if(line[lp] == '\0' || line[lp] == ';'){
        inst.func = ign;
        return inst;
    }

    // funcを読み取る
    func[0] = line[lp++];
    func[1] = line[lp++];
    func[2] = line[lp++];
    func[3] = '\0';

    while(is_empty(line[lp])) lp++;  // 空白を捨てる

    // カンマか空白、終端文字、改行文字まで捨てる
    i = 0;
    while(1){
        if(is_empty(line[lp]) || line[lp] == '\0')
            break;
        a[i++] = line[lp++];
    }
    a[i++] = '\0';
    lp++;

    // 次の文字が終端文字じゃないときbがある可能性がある
    i = 0;
    if(line[lp] != '\0'){
        while(is_empty(line[lp])) lp++;  // 空白を捨てる

        // bがあった
        if(line[lp] != '\0'){
            while(1){
                if(is_empty(line[lp]) || line[lp] == '\0')
                    break;
                b[i++] = line[lp++];
            }
            b[i++] = '\0';
        }
    }

    if(strcmp("lod", func) == 0){
        inst.func = lod;
    }else if(strcmp("sto", func) == 0){
        inst.func = sto;
    }else if(strcmp("cal", func) == 0){
        inst.func = cal;
    }else if(strcmp("ret", func) == 0){
        inst.func = ret;
    }else if(strcmp("lit", func) == 0){
        inst.func = lit;
    }else if(strcmp("inc", func) == 0){
        inst.func = inc;
    }else if(strcmp("jmp", func) == 0){
        inst.func = jmp;
    }else if(strcmp("jpc", func) == 0){
        inst.func = jpc;
    }else if(strcmp("opr", func) == 0){
        inst.func = opr;
    }else if(strcmp("cpy", func) == 0){
        inst.func = cpy;
    }else if(strcmp("nop", func) == 0){
        inst.func = nop;
        return inst;
    }else{
        inst.func = end;
        return inst;
    }

    // 演算
    if(inst.func == opr){
        if(strcmp("add", a) == 0){
            inst.u.opcode = add;
        }else if(strcmp("sub", a) == 0){
            inst.u.opcode = sub;
        }else if(strcmp("mul", a) == 0){
            inst.u.opcode = mul;
        }else if(strcmp("div", a) == 0){
            inst.u.opcode = div;
        }else if(strcmp("eq", a) == 0){
            inst.u.opcode = eq;
        }else if(strcmp("neq", a) == 0){
            inst.u.opcode = neq;
        }else if(strcmp("ls", a) == 0){
            inst.u.opcode = ls;
        }else if(strcmp("lt", a) == 0){
            inst.u.opcode = lt;
        }else if(strcmp("gr", a) == 0){
            inst.u.opcode = gr;
        }else if(strcmp("gt", a) == 0){
            inst.u.opcode = gt;
        }else if(strcmp("wrt", a) == 0){
            inst.u.opcode = wrt;
        }else if(strcmp("wrl", a) == 0){
            inst.u.opcode = wrl;
        }else{
            inst.func = end;
            return inst;
        }
    }else{
        if(a[0] != '\0'){
            switch (inst.func) {
                case lod:
                case sto:
                case cal:
                case ret:
                    if(b[0] != '\0'){
                        inst.u.addr.level = atoi(a);
                        inst.u.addr.addr = atoi(b);
                    }else{
                        // Error: expected b
                        inst.func = end;
                        return inst;
                    }
                    break;
                case jpc:
                case jmp:
                    inst.u.addr.addr = atoi(a);
                    break;
                case lit:
                case inc:
                case cpy:
                    inst.u.value = atoi(a);
                    break;
                case nop:
                default:
                    break;
            }
        }else{
            inst.func = end;
            return inst;
        }
    }

    return inst;
}

int read_code(char *filename, instraction *code){
    FILE *fp;
    char line[MAX_LINE];
    int i = 0;

    fp = fopen(filename, "r");
    if(fp == NULL){
        printf("Not search \"%s\" file.\n", filename);
        return -1;
    }
    while(fgets(line, MAX_LINE, fp) != NULL){
        code[i] = gen_line(line);
        if(code[i].func == ign)
            continue;
        if(code[i].func == end)
            break;
        i++;
    }

    fclose(fp);
    return 0;
}

int main(int argc, char const *argv[]) {
    instraction code[CODE_N];
    int i = -1;
    int is_invalid_code = 0;

    if(argc == 1){
        printf("ERROR: Plz input source's file.\n");
        return -1;
    }

    is_invalid_code = read_code((char*)argv[1], code) == -1;
    if(is_invalid_code){
        return -1;  // Error
    }

    print_code(code);
    execute(code);

    return 0;
}
