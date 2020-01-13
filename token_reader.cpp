#include <iostream>
#include <stdio.h>
#include <string.h>

#include "token_reader.hpp"
#include "typedefine.hpp"

/*
    Number:
    Ident:
    opcode:
*/

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
keywordkind TokenReader::get_keyword_kind(char *token){
    keywordkind key = (keywordkind)-1;

    // リニアサーチになっている。もっと速くできるはず
    if(strcmp(token, "if") == 0){
        key = stmt_if;
    }else if(strcmp(token, "else") == 0){
        key = stmt_else;
    }else if(strcmp(token, "print") == 0){
        key = stmt_print;
    }else if(strcmp(token, "println") == 0){
        key = stmt_println;
    }else if(strcmp(token, "var") == 0){
        key = dcl_variable;
    }else if(strcmp(token, "while") == 0){
        key = stmt_while;
    }else if(strcmp(token, "return") == 0){
        key = stmt_return;
    }else if(strcmp(token, "def") == 0){
        key = def_function;
    }

    return key;
}
int TokenReader::is_number(char c){
    return '0' <= c && c <= '9';
}
int TokenReader::is_ident(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
int TokenReader::is_opcode(char c){
    switch (c) {
        case '=':  // "=" or "=="
        case '!':  // "!="  ※ "!" は使えない
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '|':  // "|"   ※ "||" は使えない
        case '&':  // "&"   ※ "&&" は使えない
        case '~':
        case '^':
        case '<':  // "<<", "<" or "<="
        case '>':  // ">>", ">" or ">="
            return 1;
        default:
            return 0;
    }
}

// 次のトークンをtokenに、種類をtoken_kindへ書き込む
void TokenReader::next_token(){
    int i=0;
    char cur_char;
    keywordkind key;

    // この時点で forward_char に次の文字が入ってる

    // 空白、コメントを捨てる
    while(1){
        cur_char = next_char;
        if(next_char == ' ' || next_char == '\t'){  // Blank
            forward_char();
        }else if(next_char == '#'){  // Comment
            line_index = -1;
            forward_char();
        }else{
            break;
        }
    }

    if(next_char == '\0'){
        token_kind = eof;
        token[0] = '\0';
        return;
    }else if(is_number(next_char)){  // Number
        token_kind = number;
        while(is_number(next_char)){
            token[i++] = next_char;
            forward_char();
        }
    }else if(is_ident(next_char)){  // Ident
        while(is_ident(next_char) || is_number(next_char)){
            token[i++] = next_char;
            forward_char();
        }
        token[i++] = '\0';
        key = get_keyword_kind(token);
        if(key == -1){
            token_kind = ident;
        }else{
            token_kind = keyword;
            keyword_kind = key;
        }
    }else if(is_opcode(next_char)){  // Opcode
        cur_char = next_char;

        token_kind = opcode;
        token[i++] = next_char;
        forward_char();
        switch (cur_char) {
            case '+':
                opcode_kind = calc_add; break;
            case '-':
                opcode_kind = calc_sub; break;
            case '*':
                opcode_kind = calc_mul; break;
            case '/':
                opcode_kind = calc_div; break;
            case '%':
                opcode_kind = calc_mod; break;
            case '|':
                opcode_kind = bit_or; break;
            case '&':
                opcode_kind = bit_and; break;
            case '~':
                opcode_kind = bit_not; break;
            case '^':
                opcode_kind = bit_xor; break;
            case '!':  // "!="
                if(next_char == '='){
                    opcode_kind = not_equal;
                    token[i++] = next_char;
                    forward_char();
                }else{
                    i = 0;  // error
                }
                break;
            case '=':  // "=" or "=="
                if(next_char == '='){
                    opcode_kind = equal;
                    token[i++] = next_char;
                    forward_char();
                }else{
                    opcode_kind = assign;
                }
                break;
            case '<':  // "<<", "<" or "<="
            case '>':  // ">>", ">" or ">="
                if(next_char == cur_char){
                    if(cur_char == '>') opcode_kind = bit_shr;
                    else opcode_kind = bit_shl;
                    token[i++] = next_char;  // "<<" or ">>"
                    forward_char();
                }else if(next_char == '='){
                    if(cur_char == '>') opcode_kind = grt_equal;
                    else opcode_kind = les_equal;
                    token[i++] = next_char; // "<=" or ">="
                    forward_char();
                }else{
                    if(cur_char == '>') opcode_kind = grt;
                    else opcode_kind = les;
                }
                break;
            default:
                break;
        }
    }else{  // Symbol
        if(next_char == ';'){
            token_kind = semicolon;
        }else{
            token_kind = symbol;
        }
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

void print_token_kind(tokenkind token){
    switch (token) {
        case number:
            printf("number"); break;
        case ident:
            printf("ident"); break;
        case keyword:
            printf("keyword"); break;
        case opcode:
            printf("opcode"); break;
        case symbol:
            printf("symbol"); break;
        case eof:
            printf("eof"); break;
    }
}
void print_keyword_kind(keywordkind key){
    switch (key) {
        case stmt_if:
            printf("stmt_if"); break;
        case stmt_else:
            printf("stmt_else"); break;
        case stmt_while:
            printf("stmt_while"); break;
        case stmt_for:
            printf("stmt_for"); break;
        case stmt_print:
            printf("stmt_print"); break;
        case stmt_println:
            printf("stmt_println"); break;
        case stmt_return:
            printf("stmt_return"); break;
        case def_function:
            printf("def_function"); break;
        case dcl_variable:
            printf("dcl_variable"); break;
    }
}

void print_opcode_kind(opcodekind opcode){
    switch (opcode) {
        case assign:
            printf("assign"); break;
        case equal:
            printf("equal"); break;
        case grt:
            printf("grt"); break;
        case les:
            printf("les"); break;
        case not_equal:
            printf("not_equal"); break;
        case grt_equal:
            printf("grt_equal"); break;
        case les_equal:
            printf("les_equal"); break;
        case calc_add:
            printf("calc_add"); break;
        case calc_sub:
            printf("calc_sub"); break;
        case calc_mul:
            printf("calc_mul"); break;
        case calc_div:
            printf("calc_div"); break;
        case calc_mod:
            printf("calc_mod"); break;
        case bit_and:
            printf("bit_and"); break;
        case bit_or:
            printf("bit_or"); break;
        case bit_not:
            printf("bit_not"); break;
        case bit_xor:
            printf("bit_xor"); break;
        case bit_shl:
            printf("bit_shl"); break;
        case bit_shr:
            printf("bit_shr"); break;
    }
}

// int main(int argc, char const *argv[]) {
//     if(argc == 1){
//         printf("ERROR: Plz input source's file.\n");
//         return -1;
//     }
//
//     char c;
//     TokenReader tr;
//
//     tr.open_src((char *)argv[1]);
//
//     while(true){
//         tr.next_token();
//         if(tr.token_kind != eof){
//             printf("%s  ", tr.token);
//             print_token_kind(tr.token_kind);
//             if(tr.token_kind == opcode){
//                 printf(" ");
//                 print_opcode_kind(tr.opcode_kind);
//             }else if(tr.token_kind == keyword){
//                 printf(" ");
//                 print_keyword_kind(tr.keyword_kind);
//             }
//             printf("\n");
//         }else{
//             printf("[EOF]\n");
//             break;
//         }
//     }
//
//     tr.close_src();
//     return 0;
// }
