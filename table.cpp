#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "typedefine.hpp"
#include "table.hpp"
#include "token_reader.hpp"

unsigned int SymbolTable::hash(char *name){
    int i, j;
    unsigned int index = 0;
    for(i=0; name[i] != '\0'; i++)
        index ^= name[i] + i * 137;
    return index % HASH_SIZE;
}

void SymbolTable::modify(def_recode *recode){
    unsigned int i, index;
    unsigned int search_level;
    def_recode *target;

    search_level = level;

    index = hash(recode->name);
    target = &hash_map[search_level][index];
    if(strcmp(target->name, recode->name) == 0){  // 見つけた
        *target = *recode;
        return;
    }else{
        // 連結リストをたどる
        while(search_level > 0){
            search_level--;
            while(target->next_index != -1){
                target = &hash_map[search_level][target->next_index];
                if(strcmp(target->name, recode->name) == 0){  // 見つけた
                    *target = *recode;
                    return;
                }
            }
        }
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

    index = hash(str);
    target = hash_map[search_level][index];
    if(strcmp(target.name, str) == 0){  // 見つけた
        return target;
    }else{
        // 連結リストをたどる
        while(search_level > 0){
            search_level--;
            while(target.next_index != -1){
                target = hash_map[search_level][target.next_index];
                if(strcmp(target.name, str) == 0)  // 見つけた
                    return target;
            }
        }
    }

    // なかった
    printf("\nERROR: '%s' is not defined!! \n", str);
    exit(1);
}
