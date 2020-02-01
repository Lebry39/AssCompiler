#ifndef TYPDEF

#define MAX_CODE 255
#define MAX_STACK 255
#define MAX_LEVEL 64
#define MAX_LINE 64

// 命令の定義
enum functype {
    LOD, LBI, STO, SBI, CAL, RET,  // Func, Level, Locate
    LIT, INC, JMP, JPC,  // Func, Value
    OPR,    // OPR, oprtype
    CPY,    // CPY num
    NOP,    // Do nothing
    IGN,    // ignore
    END,    // END code
};

// オペコードの定義
enum oprtype {
    ADD, SUB, MUL, DIV, MOD,        // 演算
    OR, AND, NOT, XOR, SHR, SHL,    // ビット演算
    EQ, NEQ, LT, LE, GT, GE, ODD,   // 条件
    WRT, WRL                        // printf
};

struct address{
    int level;
    int addr;
};

typedef struct inst{
    functype func;      // 命令
    union{
        address addr;   // LOD, STO, CAL, RET で使う
        int value;      // LIT, INC, JMP, jmc で使う
        oprtype opcode; // ADD, SUB, MUL,... 演算、比較で使う
    } u;
} instruction;

#endif
