#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "typedefine.hpp"
#include "table.hpp"
#include "token_reader.hpp"
#include "stackmachine.hpp"

SymbolTable table;
TokenReader tr;

// 生成したコードを格納する
instruction inst;
instruction code[MAX_CODE];
int code_index = 0;
int code_sp = 0;

// 宣言した変数の管理
int stack_top = 0;

void gen_statement();
void gen_variable();
void gen_function();
void gen_assignment();
void gen_reserved_statement();
void gen_expression();

void check_semicolon(){
    // 1文はセミコロンで終わる必要がある
    if(tr.token_kind != semicolon){
        printf("%s[;]\n\nError: Semicolon Expected!!\n", tr.token);
        exit(1);
    }
}

void gen_variable(){
    def_recode recode;
    int n = 0;

    printf("VARIABLE:");
    while(1){
        tr.next_token();

        // 宣言
        if(tr.token_kind == ident){
            printf("%s", tr.token);
            strcpy(recode.name, tr.token);
            recode.kind = variable;
            recode.addr = stack_top++;
            table.put(&recode);
        }
        tr.next_token();

        // 宣言とともに代入
        if(tr.token_kind == opcode && tr.opcode_kind == assign){
            // 宣言した領域を空ける
            if(n > 0){
                inst.func = INC;
                inst.u.value = n;
                code[code_index++] = inst;
                n = 0;
            }

            printf(" = ");
            gen_expression();
        }else{
            // 宣言のみ
            n++;
        }

        // 終了
        if(tr.token_kind == semicolon) break;

        printf(", ");
        if(strcmp(tr.token, ",") == 0) continue;
        else{
            printf("Error: Invalid Syntax!!\n");
            exit(1);
        }
    }

    // 宣言した領域を空ける
    if(n > 0){
        inst.func = INC;
        inst.u.value = n;
        code[code_index++] = inst;
        n = 0;
    }
    printf("\n");
    check_semicolon();
}

void gen_function(){
    def_recode func_recode;
    def_recode recode;
    int argc = 0;

    printf("FUNCTION: ");

    tr.next_token();
    if(tr.token_kind == ident){  // 関数名
        printf("%s", tr.token);
        strcpy(func_recode.name, tr.token);
        func_recode.kind = function;
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
            argc++;
        }
    }
    func_recode.argc = argc;
    table.put(&func_recode);
    printf("\n");  // 引数ここまで

    tr.next_token();
    if(strcmp(tr.token, "{") != 0){
        printf("Error: Invalid Syntax !!\n");
        exit(1);
    }

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
    def_recode recode;

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

    // 代入する値を計算
    gen_expression();

    // アドレスへ格納
    inst.func = STO;
    inst.u.addr.level = recode.level;
    inst.u.addr.addr = recode.addr;
    code[code_index++] = inst;

    printf("\n");
    check_semicolon();
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
    def_recode recode;

    instruction oprstack[100];
    int priorities[100];
    int sp = 0;
    int p_bias = 0;
    int priority;

    int i;
    int is_opr_excepted = 0;

    while (1) {
        tr.next_token();

        if(tr.token_kind == semicolon) break;

        if(tr.token_kind == opcode){  // 演算子
            if(!is_opr_excepted){  // 一番初めは + か - か ~ か !
                if(tr.opcode_kind == bit_not){
                    // pass
                }else if(tr.opcode_kind == logic_not){
                    // pass
                }else if(tr.opcode_kind == calc_sub){
                    // ex) -6  => 0 - 6 に変換する
                    inst.func = LIT;
                    inst.u.value = 0;
                    code[code_index++] = inst;
                }else if(tr.opcode_kind == calc_add){
                    continue;  // Ignore add opcode
                }else{
                    printf("[%s, %d!=%d]\nError: operand Excepted\n", tr.token, tr.opcode_kind, calc_sub);
                    exit(1);
                }
            }

            // 読み込んだ優先度以上のものをポップ
            priority = (int)tr.opcode_kind + p_bias;
            while(priorities[sp-1] >= priority && sp > 0){
                code[code_index++] = oprstack[--sp];
            }

            // 演算子をプッシュ

            switch (tr.opcode_kind) {
                case calc_add:
                    inst.func = OPR; inst.u.opcode = ADD; break;
                case calc_sub:
                    inst.func = OPR; inst.u.opcode = SUB; break;
                case calc_mul:
                    inst.func = OPR; inst.u.opcode = MUL; break;
                case calc_div:
                    inst.func = OPR; inst.u.opcode = DIV; break;
                case calc_mod:
                    inst.func = OPR; inst.u.opcode = MOD; break;
                case equal:
                    inst.func = OPR; inst.u.opcode = EQ; break;
                case not_equal:
                    inst.func = OPR; inst.u.opcode = NEQ; break;
                case grt:
                    inst.func = OPR; inst.u.opcode = GT; break;
                case les:
                    inst.func = OPR; inst.u.opcode = LT; break;
                case grt_equal:
                    inst.func = OPR; inst.u.opcode = GE; break;
                case les_equal:
                    inst.func = OPR; inst.u.opcode = LE; break;
                case bit_or:
                    inst.func = OPR; inst.u.opcode = OR; break;
                case bit_xor:
                    inst.func = OPR; inst.u.opcode = XOR; break;
                case bit_and:
                    inst.func = OPR; inst.u.opcode = AND; break;
                case bit_shl:
                    inst.func = OPR; inst.u.opcode = SHL; break;
                case bit_shr:
                    inst.func = OPR; inst.u.opcode = SHR; break;
                case bit_not:
                    inst.func = OPR; inst.u.opcode = NOT; break;
                // 論理演算はまだ実装してない
                // case logic_not:
                //     oprstack[sp] = SHL; break;
                // case logic_and:
                //     oprstack[sp] = SHR; break;
                // case logic_or:
                //     oprstack[sp] = NOT; break;
                default:
                    printf("%d is Not suported opcode!!!\n", tr.opcode_kind);
                    exit(1);
            }

            oprstack[sp] = inst;
            priorities[sp] = priority;
            sp++;

            is_opr_excepted = 0;
            printf("%s ", tr.token);

            continue;  // 次のトークンへ
        }else if(is_opr_excepted && (strcmp(tr.token, ")") != 0)){
            break;
        }

        if(tr.token_kind == ident){ // 関数 or 変数
            recode = table.take(tr.token);
            if(recode.kind == function){
                printf("[F]");
                while(1){
                    if(strcmp(tr.token, ")") == 0)
                        break;
                    else
                        tr.next_token();
                }
                is_opr_excepted = 1;
            }else if(recode.kind == variable){
                inst.func = LOD;
                inst.u.addr.level = recode.level;
                inst.u.addr.addr = recode.addr;
                code[code_index++] = inst;
            }
            is_opr_excepted = 1;
        }else if(tr.token_kind == number){  // 定数
            inst.func = LIT;
            inst.u.value = atoi(tr.token);
            code[code_index++] = inst;
            is_opr_excepted = 1;
        }else if(strcmp(tr.token, "(") == 0){
            p_bias += 30;
        }else if(strcmp(tr.token, ")") == 0){
            p_bias -= 30;
            is_opr_excepted = 1;
        }else{
            break;
        }

        printf("%s ", tr.token);
    }

    // 残った演算子をすべてポップ
    while(sp > 0){
        code[code_index++] = oprstack[--sp];
    }
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
    }else{
        printf("%s\nError: Invalid Syntax!!\n", tr.current_line);
        exit(1);
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

    print_code(code);

    execute_code(code);

    printf("\n");
    table.dump();

    return 0;
}
