#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "typedefine.hpp"
#include "table.hpp"
#include "token_reader.hpp"

void SymbolTable::dump(){
    int idx;
    unsigned int search_level;
    def_recode *recode;
    printf("--- DUMP ---\n");
    printf("Current Level = %d\n", level);
    for(search_level=0; search_level<=level; search_level++){
        printf("*level=%d\n", search_level);
        for(idx=0; idx<MAX_HASH; idx++){
            recode = &hash_map[search_level][idx];
            if(recode->kind != undefined){
                printf("(%d) ", idx);
                printf("name=%s ", recode->name);
                printf("kind=%d ", recode->kind);
                printf("level=%d ", recode->level);
                printf("addr=%d ", recode->addr);
                printf("next=%d\n", recode->next_index);
            }
        }
    }
    printf("------------\n");
}

unsigned int SymbolTable::hash(char *name){
    int i, j;
    unsigned int index = 0;
    for(i=0; name[i] != '\0'; i++)
        index ^= name[i] + i * 137;
    return index % HASH_SIZE;
}

void SymbolTable::in_block(){
    level++;
    if(level >= MAX_LEVEL){
        printf("ERROR: Over the Max level!!\n");
        exit(1);
    }

    // レベルの初期化
    last_buffer[level] = HASH_SIZE;
    for(int i=0; i<MAX_HASH; i++){
        hash_map[level][i].kind = undefined;
        hash_map[level][i].next_index = -1;
    }
}
void SymbolTable::out_block(){
    level--;
    if(level < 0){
        printf("ERROR: level < 0 !!\n");
        exit(1);
    }
}

void SymbolTable::modify(def_recode *recode){
    unsigned int search_level;
    def_recode *target;
    int index;

    search_level = level;
    while(search_level != -1){
        index = hash(recode->name);
        target = &hash_map[search_level][index];
        if(strcmp(target->name, recode->name) == 0){  // 見つけた
            target->addr = recode->addr;
            target->argc = recode->argc;
            target->kind = recode->kind;
            return;
        }

        // 連結リストをたどる
        while(target->next_index != -1 && target->kind != undefined){
            index = target->next_index;
            target = &hash_map[search_level][index];
            if(strcmp(target->name, recode->name) == 0){  // 見つけた
                target->addr = recode->addr;
                target->argc = recode->argc;
                target->kind = recode->kind;
                return;
            }
        }

        // レベルを下げる
        search_level--;
    }

    // なかった
    printf("\nERROR: '%s' is not defined!! \n", recode->name);
    exit(1);
}

void SymbolTable::put(def_recode *recode){
    unsigned int i, index;
    def_recode *target;

    index = hash(recode->name);
    target = &hash_map[level][index];
    if(target->kind == undefined){  // 新しい挿入
        *target = *recode;
        target->next_index = -1;
        target->level = level;
    }else{  // 重複時、連結する
        // 連結されていないindexまで辿る
        while(target->next_index != -1){
            index = target->next_index;
            target = &hash_map[level][index];
        }

        // 既に宣言された
        if(strcmp(target->name, recode->name) == 0){
            printf("ERROR: '%s' has already defined.\n");
            exit(1);
        }

        // 挿入先を取得
        index = last_buffer[level]++;
        if(MAX_HASH <= index){
            printf("ERROR: Hash buffer over fllow!!\n");
            exit(1);
        }

        // 連結
        target->next_index = index;

        // 挿入
        target = &hash_map[level][index];
        *target = *recode;
        target->next_index = -1;
        target->level = level;
    }
}

def_recode SymbolTable::take(char *str){
    unsigned int i, index;
    unsigned int search_level;
    def_recode target;

    search_level = level;

    while(search_level != -1){
        index = hash(str);
        target = hash_map[search_level][index];
        if(strcmp(target.name, str) == 0){  // 見つけた
            return target;
        }

        // 連結リストをたどる
        while(target.next_index != -1 && target.kind != undefined){
            index = target.next_index;
            target = hash_map[search_level][index];
            if(strcmp(target.name, str) == 0)  // 見つけた
                return target;
        }

        // レベルをひとつ下げて再探索
        search_level--;
    }

    // なかった
    printf("\nERROR: '%s' is not defined!! \n", str);
    exit(1);
}
