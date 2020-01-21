#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "typedefine.hpp"
#include "table.hpp"
#include "token_reader.hpp"
#include "stackmachine.hpp"


SymbolTable table;
TokenReader tr;

// 命令の一時保管場所
instruction inst;
instruction return_inst;
int can_return = 0;

// 生成したコードを格納する
instruction code[MAX_CODE];
unsigned int code_index = 0;
unsigned int code_sp = 0;

// 宣言した変数の管理
int stack_top = 0;


void gen_statement();
void gen_variable();
void gen_function();
void gen_assignment();
void gen_reserved_statement();
void gen_expression();
void gen_condition();
void gen_block();


void assert(char *str){
    if(strcmp(str, tr.token) != 0){
        printf("[Assert] I hate '%s'\n", tr.token);
        printf("Error: Expected '%s' !", str);
        exit(1);
    }
}

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

    printf("VARIABLE: ");
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
            tr.next_token();  // "="を捨てる
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
    char arg_names[HASH_SIZE][MAX_NAME];
    int argc = 0;
    int jmp_inst_index = code_index++;

    // 関数定義部分で実行されないように、ブロックの最後までジャンプ
    inst.func = JMP;
    inst.u.addr.addr = -1;
    code[jmp_inst_index] = inst;

    printf("FUNCTION: ");

    tr.next_token();
    if(tr.token_kind == ident){  // 関数名
        printf("%s", tr.token);
        strcpy(func_recode.name, tr.token);
        func_recode.kind = function;
        func_recode.addr = code_index;
        table.put(&func_recode);
    }else{
        printf("Error: '%s' is Invalid function name.\n", tr.token);
        exit(1);
    }

    table.in_block();

    // 引数
    printf("\n ARGUMENTS:");
    tr.next_token();
    assert((char *)"(");

    while (1) {
        tr.next_token();
        if(strcmp(tr.token, ")") == 0) break;

        // 引数
        if(tr.token_kind == ident){
            printf(" %s", tr.token);
            strcpy(recode.name, tr.token);
            strcpy(arg_names[argc], tr.token);
            recode.kind = variable;
            table.put(&recode);
            argc++;
        }
    }

    // 引数がargc個ある関数の、i番目の引数の相対アドレスを登録する
    for(int i=0; i<argc; i++){
        recode = table.take(arg_names[i]);
        recode.addr = i - argc;
        table.modify(&recode);
    }
    printf("\n");  // 引数ここまで

    // 関数の引数を登録する
    func_recode.argc = argc;
    table.modify(&func_recode);

    // return命令を生成
    return_inst.func = RET;
    return_inst.u.addr.level = table.get_level();
    return_inst.u.addr.addr = argc;

    // 返りアドレス、ディスプレイ回復の領域を確保
    inst.func = INC;
    inst.u.value = 2;
    stack_top += 1;
    code[code_index++] = inst;

    // { statements; }
    tr.next_token();
    can_return = 1;
    gen_block();
    can_return = 0;

    if(code[code_index-1].func != RET){
        // 最後にReturn命令が無い
        inst.func = LIT;
        inst.u.value = 0;
        code[code_index++] = inst;
        code[code_index++] = return_inst;
    }
    table.out_block();

    // 定義を飛ばす
    code[jmp_inst_index].u.addr.addr = code_index;
    stack_top -= 1;
    printf("\n");

}

void gen_reserved_statement(){
    if(tr.keyword_kind == stmt_print){
        printf("Print: ");
        tr.next_token();
        gen_expression();
        inst.func = OPR;
        inst.u.opcode = WRT;
        code[code_index++] = inst;
        printf("\n");
    }else if(tr.keyword_kind == stmt_println){
        printf("Print-ln: ");
        tr.next_token();
        gen_expression();
        inst.func = OPR;
        inst.u.opcode = WRT;
        code[code_index++] = inst;
        inst.func = OPR;
        inst.u.opcode = WRL;
        code[code_index++] = inst;
        printf("\n");
    }else if(tr.keyword_kind == stmt_if){
        unsigned int else_inst_index;
        unsigned int out_of_if_chain_index;

        printf("IF: ");
        tr.next_token();
        gen_condition();

        // Trueの時、blockへジャンプ
        inst.func = JPC;
        inst.u.addr.addr = code_index + 2;
        code[code_index++] = inst;

        // falseの時、blockの外(アドレス未定)にジャンプ
        else_inst_index = code_index++;
        inst.func = JMP;
        code[else_inst_index] = inst;

        gen_block();

        // Nop を挟む
        inst.func = NOP;
        out_of_if_chain_index = code_index++;
        code[out_of_if_chain_index] = inst;

        // false時のジャンプアドレスを NOP へ設定する
        code[else_inst_index].u.addr.addr = code_index-1;

        tr.next_token();
        if(tr.token_kind == keyword && tr.keyword_kind == stmt_else){
            printf("Else: ");
            code[else_inst_index].u.addr.addr = code_index;

            tr.next_token();
            if(strcmp(tr.token, "{") == 0){
                gen_block();
            }else if(tr.token_kind == keyword && tr.keyword_kind == stmt_if){
                gen_reserved_statement();
            }else{
                printf("Error: Excepted '{' or 'if'\n");
                exit(1);
            }

            // 成功時のジャンプアドレスを設定
            inst.func = JMP;
            inst.u.addr.addr = code_index;
            code[out_of_if_chain_index] = inst;
        }
    }else if(tr.keyword_kind == stmt_while){
        unsigned int jmp_inst_index;
        unsigned int condition_index = code_index;

        printf("WHILE: ");
        tr.next_token();
        gen_condition();

        // Trueの時、blockへジャンプ
        inst.func = JPC;
        inst.u.addr.addr = code_index + 2;
        code[code_index++] = inst;

        // falseの時、blockの外(アドレス未定)にジャンプ
        jmp_inst_index = code_index++;
        inst.func = JMP;
        code[jmp_inst_index] = inst;

        gen_block();

        // ループ条件へ戻る
        inst.func = JMP;
        inst.u.addr.addr = condition_index;
        code[code_index++] = inst;

        // false時のジャンプアドレスを code_index に設定する
        code[jmp_inst_index].u.addr.addr = code_index;

    }else if(tr.keyword_kind == stmt_return){
        if(can_return){
            printf(" Return: ");
            tr.next_token();  // "return" を削除
            gen_expression();
            check_semicolon();
            printf("\n");
            code[code_index++] = return_inst;  // return 命令を挿入
        }else{
            printf("Error: You dont return the block.\n");
            exit(1);
        }
    }else{
        printf("Sorry!! '%s' is not yet supported !!\n", tr.token);
        exit(1);
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
        if(tr.token_kind == semicolon) break;  // end exoression
        if(p_bias == 0 && strcmp(tr.token, ")") == 0) break;  // end condition

        // Generate Opcode
        if(tr.token_kind == opcode){
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
                    printf("[%s]\nError: operand Excepted\n", tr.token);
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
                case assign:
                    // "変数をload" を、"変数にwrite" に書き換える
                    inst = code[--code_index];
                    if(inst.func != LOD){
                        printf("Error: Variable Excepted!!\n");
                        exit(1);
                    }
                    inst.func = STO;
                    break;
                case assign_add:
                case assign_sub:
                case assign_mul:
                case assign_div:
                case assign_mod:
                    // sto Lv Ad, lod Lv Ad, opr [add, sub,...]
                    if(code[code_index-1].func != LOD){
                        printf("Error: Variable Excepted!!\n");
                        exit(1);
                    }

                    inst = code[code_index-1];
                    inst.func = STO;
                    oprstack[sp] = inst;
                    priorities[sp++] = priority;

                    inst.func = OPR;
                    if(tr.opcode_kind == assign_add)      inst.u.opcode = ADD;
                    else if(tr.opcode_kind == assign_sub) inst.u.opcode = SUB;
                    else if(tr.opcode_kind == assign_mul) inst.u.opcode = MUL;
                    else if(tr.opcode_kind == assign_div) inst.u.opcode = DIV;
                    else if(tr.opcode_kind == assign_mod) inst.u.opcode = MOD;
                    break;
                case logic_not:
                    // lit 0, opr eq
                    inst.func = OPR;
                    inst.u.opcode = EQ;
                    oprstack[sp] = inst;
                    priorities[sp++] = priority;

                    inst.func = LIT;
                    inst.u.value = 0;
                    break;
                case logic_and:
                    // lit 0, opr eq,  opr neq
                    inst.func = OPR;
                    inst.u.opcode = NEQ;
                    oprstack[sp] = inst;
                    priorities[sp++] = priority;

                    inst.func = OPR;
                    inst.u.opcode = EQ;
                    oprstack[sp] = inst;
                    priorities[sp++] = priority;

                    inst.func = LIT;
                    inst.u.value = 0;
                    break;
                case logic_or:
                    // lit 0, opr neq, opr neq
                    inst.func = OPR;
                    inst.u.opcode = NEQ;
                    oprstack[sp] = inst;
                    priorities[sp++] = priority;

                    inst.func = OPR;
                    inst.u.opcode = NEQ;
                    oprstack[sp] = inst;
                    priorities[sp++] = priority;

                    inst.func = LIT;
                    inst.u.value = 0;
                    break;
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
                default:
                    printf("\n'%s' is Not suported opcode!!!\n", tr.token);
                    exit(1);
            }

            oprstack[sp] = inst;
            priorities[sp++] = priority;

            is_opr_excepted = 0;
            printf("%s ", tr.token);

            tr.next_token();
            continue;  // 次のトークンへ
        }else if(is_opr_excepted && strcmp(tr.token, ")") != 0){
            break;
        }

        // Generate Operand
        if(tr.token_kind == ident){
            recode = table.take(tr.token);
            if(recode.kind == function){  // 関数呼び出し
                printf("Call %s (", tr.token);
                tr.next_token();
                assert((char *)"(");

                for(i=0; i<recode.argc; i++){
                    if(strcmp(tr.token, "(") == 0 || strcmp(tr.token, ",") == 0){
                        tr.next_token();
                        printf("\n Arg%d: ", i+1);
                        gen_expression();
                    }else{
                        printf("\nError: Too few arguments. Need %d args.\n", recode.argc);
                        exit(1);
                    }
                }

                // 引数が無い
                if(recode.argc == 0){
                    tr.next_token();
                }
                assert((char *)")");
                printf("\n)\n");

                inst.func = CAL;
                inst.u.addr.level = recode.level;
                inst.u.addr.addr = recode.addr;
                code[code_index++] = inst;

                is_opr_excepted = 1;
            }else if(recode.kind == variable){  // 変数
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
        tr.next_token();
    }

    // 残った演算子をすべてポップ
    while(sp > 0){
        code[code_index++] = oprstack[--sp];
    }
}

void gen_condition(){
    assert((char *)"(");
    tr.next_token();

    gen_expression();
    printf("%s\n", tr.token);

    assert((char *)")");
    tr.next_token();
}

void gen_statement(){
    if(strcmp(tr.token, "var") == 0){  // 変数宣言
        gen_variable();
    }else if(strcmp(tr.token, "def") == 0){  // 関数宣言
        gen_function();
    }else if(tr.token_kind == keyword){  // 予約式
        gen_reserved_statement();
    }else if(strcmp(tr.token, "{") == 0){
        gen_block();
        assert((char *)"}");
    }else{
        gen_expression();
        printf("\n");
    }
}

void gen_block(){

    assert((char *)"{");
    printf("{\n");

    // 引数などで既にレベルが上がってるので不要
    while(true){
        tr.next_token();
        if((strcmp(tr.token, "}") == 0)) break;

        if(tr.token_kind == eof){
            code[code_index++].func = END;
            break;
        }
        gen_statement();
    }

    assert((char *)"}");
    printf("}\n");
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

    return 0;
}
