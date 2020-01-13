#ifndef TBL

#define MAX_NAME  30
#define HASH_SIZE 50
#define MAX_HASH  100

typedef enum defkind{
    undefined,
    function,
    variable,
    constant
} defkind;

struct def_recode{
    char name[MAX_NAME+1];
    defkind kind;

    int level;  // 宣言されたレベル
    int addr;   // 相対アドレス
    int argc;   // 関数の引数の数

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
    int get_level(){
        return level;
    }
    void in_block(){
        level++;
        if(level >= MAX_LEVEL){
            printf("ERROR: Over the Max level!!\n");
            exit(1);
        }

        // レベルの初期化
        last_buffer[level] = HASH_SIZE;
        for(int i=0; i<HASH_SIZE; i++)
            hash_map[level][i].kind = undefined;
    }
    void out_block(){
        level--;
        if(level < 0){
            printf("ERROR: level < 0 !!\n");
            exit(1);
        }
    }
    void dump(){
        int idx;
        unsigned int search_level;
        def_recode *recode;
        printf("--- DUMP ---\n");
        for(search_level=0; search_level<=level; search_level++){
            printf("*level=%d\n", search_level);
            for(idx=0; idx<MAX_HASH; idx++){
                recode = &hash_map[search_level][idx];
                if(recode->kind != undefined){
                    printf("(%d) ", idx);
                    printf("name=%s ", recode->name);
                    printf("kind=%d ", recode->kind);
                    printf("next=%d\n", recode->next_index);
                }
            }
        }
        printf("------------\n");
    }

    SymbolTable(){
        int idx;
        level = 0;
        last_buffer[level] = HASH_SIZE;
        for(idx=0; idx<MAX_HASH; idx++){
            hash_map[level][idx].kind = undefined;
        }
    }
};

#endif
