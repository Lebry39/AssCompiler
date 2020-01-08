#include <iostream>
#include <stdio.h>

#define MAX_TOKEN 64
#define MAX_LINE 64

using namespace std;
/*
    Number:
    Ident:
    Opecode:
*/
enum tokentype {
    number, // [0-9]{[0-9]}
    ident,  // [a-zA-Z]{[a-zA-Z0-9]}
    opcode, // 演算子、比較など
    symbol, // その他
    eof     // End of file
};

class TokenReader{
public:
    char token[MAX_TOKEN];
    tokentype kind;
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
            cout << "File not found." << endl;
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

    void forward_char();
    void next_token();

private:
    int is_opened_file;
    FILE *fp;
};

// 次の文字列に進め、その文字を next_char に格納
void TokenReader::forward_char(){
    if(line_index == -1){
        if(fgets(current_line, MAX_LINE, fp) != NULL){
            line_index = 0;
        }else{
            // EOFまでを読んだ
            next_char = '\0';
            return;
        }
    }

    next_char = current_line[line_index++];

    // 改行文字を読んだら次の行へ
    if(next_char == '\n' || next_char == '\0'){
        next_char = ' ';
        line_index = -1;
    }
}

int is_number(char c){
    return '0' <= c && c <= '9';
}
int is_ident(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
int is_opecode(char c){
    switch (c) {
        case '=':  // "=" or "=="
        case '!':  // "!" or "!="
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '|':  // "|" or "||"
        case '&':  // "&" or "&"
        case '~':
        case '^':
        case '<':  // "<<", "<" or "<="
        case '>':  // ">>", ">" or ">="
            return 1;
        default:
            return 0;
    }
}

// 次のトークンをtokenに、種類をkindへ書き込む
void TokenReader::next_token(){
    int i=0;
    char cur_char;

    // この時点で forward_char に次の文字が入ってる

    // 空白を捨てる
    while(1){
        if(next_char == ' ' || next_char == '\t') forward_char();
        else break;
    }

    if(next_char == '\0'){
        kind = eof;
        token[0] = '\0';
        return;
    }else if(is_number(next_char)){  // Number
        kind = number;
        while(is_number(next_char)){
            token[i++] = next_char;
            forward_char();
        }
    }else if(is_ident(next_char)){  // Ident
        kind = ident;
        while(is_ident(next_char) || is_number(next_char)){
            token[i++] = next_char;
            forward_char();
        }
    }else if(is_opecode(next_char)){  // Opcode
        cur_char = next_char;

        kind = opcode;
        token[i++] = next_char;
        forward_char();
        switch (next_char) {
            case '!':  // "!" or "!="
                if(next_char == '='){
                    token[i++] = next_char;
                    forward_char();
                }
                break;
            case '|':  // "|" or "||"
            case '&':  // "&" or "&&"
            case '=':  // "=" or "=="
                if(next_char == cur_char){
                    token[i++] = next_char;
                    forward_char();
                }
                break;
            case '<':  // "<<", "<" or "<="
            case '>':  // ">>", ">" or ">="
                if(next_char == cur_char){
                    token[i++] = next_char;  // "<<" or ">>"
                    forward_char();
                }else if(next_char == '='){
                    token[i++] = next_char; // "<=" or ">="
                    forward_char();
                }
                break;
            default:
                break;
        }
    }else{  // Symbol
        kind = symbol;
        token[i++] = next_char;
        forward_char();
    }

    if(i==0){
        printf("\"%s\" Inviled token!!\n", current_line);
        exit(1);
    }else{
        token[i++] = '\0';
    }
}

int main(int argc, char const *argv[]) {
    if(argc == 1){
        printf("ERROR: Plz input source's file.\n");
        return -1;
    }

    char c;
    TokenReader tr;

    tr.open_src((char *)argv[1]);

    while(true){
        tr.next_token();
        if(tr.kind != eof){
            if(tr.token[0] == ';'){
                printf(";\n");
            }else {
                printf("%s, ", tr.token);
            }
        }else{
            printf("[EOF]\n");
            break;
        }
    }

    return 0;
}
