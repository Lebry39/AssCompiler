#ifndef TBL

#define MAX_NAME  30
#define HASH_SIZE 50
#define MAX_HASH  100

typedef enum defkind{
    undefined,
    function,
    variable,
    array,
    constant
} defkind;

struct def_recode{
    char name[MAX_NAME+1];
    defkind kind;
    int level;  // 宣言されたレベル

    int addr;   // 相対アドレス
    int argc;   // 関数の引数の数

    int length;  // Array's index count

    int next_index;  // 次のインデックス
};

class SymbolTable{
private:
    unsigned int last_buffer[MAX_LEVEL];
    unsigned int hash(char *);
    unsigned int level;

public:
    def_recode hash_map[MAX_LEVEL][MAX_HASH];
    void modify(def_recode *);
    void put(def_recode *);
    def_recode take(char *);
    void in_block();
    void out_block();
    void dump();
    int get_level(){
        return level;
    }
    SymbolTable(){
        int idx;
        level = 0;
        last_buffer[level] = HASH_SIZE;
        for(idx=0; idx<MAX_HASH; idx++){
            hash_map[level][idx].kind = undefined;
            hash_map[level][idx].next_index = -1;
        }
    }
};

#endif
