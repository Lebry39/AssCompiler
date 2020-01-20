#ifndef TKNRDR

#define MAX_TOKEN 64
#define MAX_LINE 64

typedef enum tokenkind {
    number,   // [0-9]{[0-9]}
    ident,    // [a-zA-Z]{[a-zA-Z0-9]}
    keyword,  // identの中も予約されている
    opcode,   // 演算子など、opcodeにoprtypeを格納する
    symbol,   // カンマ, 括弧など
    semicolon,// ';'
    eof       // End of file
} tokenkind;

typedef enum opcodekind {
    /* Low Priority */
    assign,     // "="

    assign_add, // "+="
    assign_sub, // "-="
    assign_mul, // "*="
    assign_div, // "/="
    assign_mod, // "%="

    equal,      // "=="
    not_equal,  // "!="
    grt,        // ">"
    les,        // "<"
    grt_equal,  // ">="
    les_equal,  // "<="
    logic_and,  // "&&"
    logic_or,   // "||"
    logic_not,  // "!"
    bit_or,     // "|"
    bit_xor,    // "^"
    bit_and,    // "&"
    bit_shl,    // "<<"
    bit_shr,    // ">>"
    calc_add,   // "+"
    calc_sub,   // "-"
    calc_mul,   // "*"
    calc_div,   // "/"
    calc_mod,   // "%"
    bit_not     // "~"
    /* Top Priority */
} opcodekind;

typedef enum keywordkind {
    stmt_if,        // "if"
    stmt_else,      // "else"
    stmt_while,     // "while"
    stmt_for,       // "for"
    stmt_print,     // "print"
    stmt_println,   // "println"
    stmt_return,    // "return"
    def_function,   // "def"
    dcl_variable    // "var"
} keywordkind;

class TokenReader{
public:
    // 出力
    char token[MAX_TOKEN];
    tokenkind token_kind;

    keywordkind keyword_kind;  // token_kind is keyword
    opcodekind opcode_kind;    // token_kind is opcode

    // 文字読み取り用
    char current_line[MAX_LINE];
    char next_char;
    int line_index;

    TokenReader(){
        line_index = -1;
        is_opened_file = 0;
    }
    ~TokenReader(){
        close_src();
    }

    void open_src(char *filename){
        fp = fopen(filename, "r");
        if(fp == NULL){
            std::cout << "File not found." << std::endl;
            exit(0);
        }else{
            is_opened_file = 1;
        }

        // 先読みをしておく
        forward_char();
    }
    void close_src(){
        fclose(fp);
    }

    keywordkind get_keyword_kind(char *);
    int is_ident(char);
    int is_number(char);
    int is_opcode(char);
    void forward_char();
    void next_token();

private:
    int is_opened_file;
    FILE *fp;
};

#endif
