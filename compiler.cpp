#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "typedefine.hpp"
#include "table.hpp"
#include "token_reader.hpp"
#include "stackmachine.hpp"

SymbolTable table;
def_recode recode;
TokenReader tr;

// 生成したコードを格納する
instruction inst;
instruction code[MAX_CODE];
int code_index = 0;
int code_sp = 0;

void gen_statement();
void gen_variable();
void gen_function();
void gen_assignment();
void gen_reserved_statement();
void gen_expression();


void gen_variable(){
    printf("VARIABLE:");
    while(tr.token_kind != semicolon){
        tr.next_token();
        if(tr.token_kind == ident){
            printf(" %s", tr.token);
            strcpy(recode.name, tr.token);
            recode.kind = variable;
            table.put(&recode);
        }
    }
    printf("\n");
}

void gen_function(){
    printf("FUNCTION: ");

    tr.next_token();
    if(tr.token_kind == ident){  // 関数名
        printf("%s", tr.token);
        strcpy(recode.name, tr.token);
        recode.kind = function;
        table.put(&recode);
    }else{
        printf("Error: '%s' is Invalid function name.\n", tr.token);
        exit(1);
    }

    printf("{\n");

    table.in_block();

    // 引数
    printf(" ARGUMENTS:");
    while (1) {
        tr.next_token();
        if(strcmp(tr.token, ")") == 0) break;

        // 引数
        if(tr.token_kind == ident){
            printf(" %s", tr.token);
            strcpy(recode.name, tr.token);
            recode.kind = variable;
            table.put(&recode);
        }
    }
    printf("\n");  // 引数ここまで

    // {statement}
    while (1) {
        tr.next_token();
        if(strcmp(tr.token, "}") == 0) break;
        gen_statement();
    }
    printf("\n");

    table.out_block();
    printf("}\n");
}

void gen_assignment(){
    recode = table.take(tr.token);
    if(recode.kind == function){
        printf("Error: '%s' is function, Excepted Variable!!!\n");
        exit(1);
    }

    printf("Assign: %s = ", recode.name);

    tr.next_token();
    if(strcmp(tr.token, "=") != 0){
        printf("Your need opcode '='\n");
        exit(1);
    }

    gen_expression();
    printf("\n");
}

void gen_reserved_statement(){
    if(tr.keyword_kind == stmt_print){
        printf("Print: ");
        gen_expression();
        inst.func = OPR;
        inst.u.opcode = WRT;
        code[code_index++] = inst;
        printf("\n");
    }else if(tr.keyword_kind == stmt_println){
        printf("Print-ln: ");
        gen_expression();
        inst.func = OPR;
        inst.u.opcode = WRT;
        code[code_index++] = inst;
        inst.func = OPR;
        inst.u.opcode = WRL;
        code[code_index++] = inst;
        printf("\n");
    }else if(tr.keyword_kind == stmt_return){
        printf(" RETURN: ");
        gen_expression();
    }
}

void gen_expression(){
    oprtype oprstack[100];
    int priorities[100];
    int sp = 0;
    int p_bias = 0;
    int i, priority;

    int is_opr_excepted = 0;

    printf("{ ");
    while (1) {
        tr.next_token();
        if(tr.token_kind == semicolon) break;

        if(tr.token_kind == opcode){  // 演算子
            if(!is_opr_excepted){  // 一番初めは + か -
                if(tr.opcode_kind == calc_sub){
                    // ex) -6  => 0 - 6 に変換する
                    inst.func = LIT;
                    inst.u.value = 0;
                    code[code_index++] = inst;
                }else if(tr.opcode_kind == calc_add){
                    continue;  // Ignore add opcode
                }else{
                    printf("[?]\nError: Value Excepted\n");
                    exit(1);
                }
            }
            // printf("[P]");
            priority = (int)tr.opcode_kind + p_bias;

            // 読み込んだ優先度以上のものをポップ
            while(priorities[sp-1] >= priority && sp > 0){
                inst.func = OPR;
                inst.u.opcode = oprstack[--sp];
                code[code_index++] = inst;
            }

            // 演算子をプッシュ
            switch (tr.opcode_kind) {
                case calc_add:
                    oprstack[sp] = ADD; break;
                case calc_sub:
                    oprstack[sp] = SUB; break;
                case calc_mul:
                    oprstack[sp] = MUL; break;
                case calc_div:
                    oprstack[sp] = DIV; break;
                case calc_mod:
                    oprstack[sp] = MOD; break;
                default:
                    printf("%d is Not suported opcode!!!\n", tr.opcode_kind);
                    exit(1);
            }
            priorities[sp++] = priority;
            is_opr_excepted = 0;
            printf("%s ", tr.token);

            continue;
        }else if(is_opr_excepted && (strcmp(tr.token, ")") != 0)){
            printf("[?]\nError: excepted opcode!!\n");
            exit(1);
        }

        if(tr.token_kind == ident){ // 関数 or 変数
            recode = table.take(tr.token);
            if(recode.kind == function){
                // printf("[F]");
            }else if(recode.kind == variable){
                // printf("[V]");
            }
            is_opr_excepted = 1;
        }else if(tr.token_kind == number){  // 定数
            // printf("[C]");
            inst.func = LIT;
            inst.u.value = atoi(tr.token);
            code[code_index++] = inst;
            is_opr_excepted = 1;
        }else if(strcmp(tr.token, "(") == 0){
            p_bias += 3;
        }else if(strcmp(tr.token, ")") == 0){
            p_bias -= 3;
            is_opr_excepted = 1;
        }else{
            printf("[;]\nError: Semicolon Expected!!\n");
            exit(1);
        }

        printf("%s ", tr.token);
    }

    // 残った演算子をすべてポップ
    while(sp > 0){
        inst.func = OPR;
        inst.u.opcode = oprstack[--sp];
        code[code_index++] = inst;
    }
    printf("}");
}

void gen_statement(){
    if(strcmp(tr.token, "var") == 0){  // 変数宣言
        gen_variable();
    }else if(strcmp(tr.token, "def") == 0){  // 関数宣言
        gen_function();
    }else if(tr.token_kind == ident){  // 代入式
        gen_assignment();
    }else if(tr.token_kind == keyword){  // 予約式
        gen_reserved_statement();
    }
}

int main(int argc, char const *argv[]) {
    int i=0;

    tr.open_src((char *)"test.src");

    while(true){
        tr.next_token();
        if(tr.token_kind == eof){
            code[code_index++].func = END;
            break;
        }
        gen_statement();
    }

    // printf("\n");
    // table.dump();

    print_code(code);

    execute_code(code);

    return 0;
}
