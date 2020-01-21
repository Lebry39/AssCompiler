# Stack Machine で使用できる命令
# 命令1
構文: `func level addr`
func  = LOD, STO, CAL, RET
level = ブロックのレベル
addr  = ブロック先頭からの相対アドレス

### [注意]
サブルーチンでのアドレス指定に癖がある

**サブルーチン呼び出し**
1. 引数をスタックに格納
2. disp[level]の退避をスタックの`sp`に格納
3. 現在の`ip`をスタックの`sp+1`に格納
4. disp[level]に現在の`sp`(2と同値)を格納する

よって、f(x, y)のように呼び出した時スタックは以下のようになる。  
f関数では変数aを宣言しているとする。
```
|       x     | -2
|       y     | -1
| disp[level] | <----- sp ( disp[level] )
|       ip    | +1
|       a     | +2
```
つまり、引数がn個ある関数の、m番目の引数の相対アドレスは`(m-1)-n`
よって上記のスタックの時、レベルがlの関数内で引数xをロードする命令は
```
    lod disp[1] -2
```
のように呼び出すことになる。

# 命令2
LIT value :スタックへ値をPush  
INC value :spをvalue進める  
JMP addr  :addrをipへ代入  
JPC addr  :スタック最上位が0でないとき、addrをipへ代入  

# 命令3
OPR xxx
```
ADD, SUB, MUL, DIV, MOD,        // 演算
OR, AND, NOT, XOR, SHR, SHL,    // ビット演算
EQ, NEQ, LT, LE, GT, GE, ODD,   // 条件
WRT, WRL                        // printf
```

# 命令4
CPY value: スタックの最上位をvalue回Push  
NOP      : 何もしない  
IGN      : 完全に無視される  
END      : プログラムを終了する  
