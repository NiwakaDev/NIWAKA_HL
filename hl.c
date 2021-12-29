#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define SIZE 10000
#define END 0
#define VAR_SIZE 256
typedef unsigned char* String;

int LoadText(String path, String t){
    FILE *fp;
    unsigned char s[1000];
    int i = 0, j;
    if (path[0] == 34) { i = 1; }
    for (j = 0; ; j++) {
        if (path[i + j] == 0 || path[j + i] == 34)
            break; 
        s[j] = path[i + j];
    }
    s[j] = 0;
    fp = fopen(s, "rt"); 
    if (fp == 0) { 
        printf("fopen error : %s\n", path);
        return 1;
    }
    i = fread(t, 1, SIZE-1, fp);
    fclose(fp);
    t[i] = END; 
    return 0; 
}

#define MAX_TC 1000
String ts[MAX_TC+1];//ユニークなトークン名のポインタへの配列
int tl[MAX_TC+1];//ユニークなトークンの長さ
unsigned char tcBuff[(MAX_TC+1)*10];//ユニークなトークン名をフラットに並べていくバッファ
int tcs=0, tcb=0;//tcs:ユニークなトークン列のサイズ
int var[MAX_TC+1];
int token_size=0;
int tc[SIZE];//ユニークでないトークン配列

enum { TcSemi = 0, TcDot, TcWiCard, Tc0, Tc1, Tc2, Tc3, Tc4, Tc5, Tc6, Tc7, Tc8, TcEEq, TcNEq, TcLt, TcGe, TcLe, TcGt };
char tcInit[] = "; . !!* 0 1 2 3 4 5 6 7 8 == != < >= <= >";

int phrCmp_tc[32*100];
int ppc1, wpc[9];//wpcはワイルドカードの直後に置かれたトークンの場所を管理する。

int phrCmp(int pid, String phr, int pc)
{
    int i0 = pid * 32, i, i1, j;
    if (phrCmp_tc[i0 + 31] == 0) {//0でないならキャッシュ済み
        i1 = Parser(phr, &phrCmp_tc[i0]);
        phrCmp_tc[i0 + 31] = i1;//末尾にはトークン列の長さが入る
    }
    i1 = phrCmp_tc[i0 + 31];
    for (i = 0; i < i1; i++) {
        if (phrCmp_tc[i0 + i] == TcWiCard) {
            i++;
            //!!*0ならば、0のトークン番号を取得する。
            //0のトークン番号はTc0と一致する。
            //Tc0-Tc0により、jは0になる。
            //!!*1ならば、1のトークン番号を取得する。
            //1のトークン番号はTc1と一致する。
            //Tc1-Tc0により、jは1になる。
            j = phrCmp_tc[i0 + i] - Tc0; // 後続の番号を取得.
            wpc[j] = pc;
            pc++;
            continue;
        }
        if (phrCmp_tc[i0 + i] != tc[pc]) return 0; // マッチせず.
        pc++;
    }
    ppc1 = pc;
    return 1; // マッチした.
}

//トークンIDを取得する。
//存在しない場合は登録して、そのIDを返す
int GetTc(String s, int len){
    int i;
    for(i=0; i<tcs; i++){
        if(len==tl[i]&&strncmp(s, ts[i], len)==0){
            break;
        }
    }
    if(i==tcs){
        if(tcs>=MAX_TC){
            fprintf(stderr, "too many tokens\n");
            exit(EXIT_FAILURE);
        }
        strncpy(&tcBuff[tcb], s, len);
        tcBuff[tcb+len] = 0;
        ts[i] = &tcBuff[tcb];
        tl[i] = len;
        tcb += len+1;
        tcs++;
        var[i] = strtol(ts[i], 0, 0);
    }
    return i;
}

bool IsAlphabetOrNumber(unsigned char ascii_code){
    if('0'<=ascii_code&&ascii_code<='9'){
        return true;
    }else if('a'<=ascii_code&&ascii_code<='z'){
        return true;
    }else if('A'<=ascii_code&&ascii_code<='Z'){
        return true;
    }else if(ascii_code=='_'){
        return true;
    }
    return false;
}

bool IsCtrl(unsigned char ascii_code){
    return ascii_code == ' ' || ascii_code == '\t' || ascii_code == '\n' || ascii_code == '\r'||ascii_code==0x12;
}

//字句解析
int Parser(String s, int* tc){
    int i=0, j=0, len;
    for(;;){
        if(IsCtrl(s[i])){
            i++;
            continue;
        }
        if(s[i]==END){
            return j;
        }
        len = 0;
        if(strchr("(){}[];,", s[i]) !=NULL){
            len = 1;
        }else if(IsAlphabetOrNumber(s[i])){
            while(IsAlphabetOrNumber(s[i+len])){
                len++;
            }
        }else if(strchr("=+-*/!%&~|<>?:.#", s[i])!=NULL){
            while(strchr("=+-*/!%&~|<>?:.#", s[i + len])!=NULL&&(s[i+len]!='\0')){//strchrは文字列の終端\0も判定していまう。なので、それを防ぐためにs[i+len]!='\0'を書いてる。
                len++;
            }
        }else{
            fprintf(stderr, "syntax error:\n", s[i]);
            exit(EXIT_FAILURE);
        }
        tc[j] = GetTc(&s[i], len);
        i = i + len;
        j++;
    }
}

void PrintToken(int* tc){
    for(int i=0; i<token_size; i++){
        fprintf(stderr, "%s\n", ts[tc[i]]);
    }
}

typedef int* IntP;
enum _OP_KIND { OpCpy = 0, OpAdd, OpSub, OpPrint, OpGoto, OpJeq, OpJne, OpJlt, OpJge, OpJle, OpJgt, OpTime, OpEnd, OpAdd1 };
typedef enum _OP_KIND OP_KIND;
IntP ic[SIZE];
IntP* icq;//ic上で色々操作をするための作業用ポインタ

//命令をここに書き込む
void PutIc(OP_KIND op, IntP p1, IntP p2, IntP p3, IntP p4){
    icq[0] = (IntP)op;//opはポインタではないが、便宜上ポインタとキャストしておいて、ポインタの配列に格納しておく
    icq[1] = p1;
    icq[2] = p2;
    icq[3] = p3;
    icq[4] = p4;
    icq    += 5;
}

int Compile(String s){
    int pc, pc1, i;
    IntP* icq1;
    pc1 = Parser(s, tc);
    tc[pc1++] = TcSemi;
    tc[pc1] = tc[pc1 + 1] = tc[pc1 + 2] = tc[pc1 + 3] = TcDot;    
    icq = ic;
    for(pc=0; pc<pc1;){
        if(phrCmp(1, "!!*0 = !!*1;", pc)){//単純代入
            PutIc(OpCpy, &var[tc[wpc[0]]], &var[tc[wpc[1]]], 0, 0);
        }else if(phrCmp(2, "print !!*0;", pc)){
            PutIc(OpPrint, &var[tc[wpc[0]]], 0, 0, 0);
        }else if(phrCmp(3, ";", pc)){
            //何もしない。
        }else{
            goto error;
        }
        pc = ppc1;
    }
    PutIc(OpEnd, 0, 0, 0, 0);
    icq1 = icq;
    return icq1-ic;
    error:
        fprintf(stderr, "Syntax error : %s %s %s %s\n", ts[tc[pc]], ts[tc[pc + 1]], ts[tc[pc + 2]], ts[tc[pc + 3]]);
        return -1;
}

void Execute(){
    clock_t t0 = clock();
    IntP* icp = ic;//ic配列の先頭の番地を入れる
    for(;;){
        switch ((OP_KIND)icp[0]){
            case OpCpy:
                *icp[1] = *icp[2];
                icp += 5;
                continue;
            case OpAdd:
                *icp[1] = *icp[2] + *icp[3];
                icp += 5;
                continue;
            case OpPrint:
                printf("%d\n", *icp[1]);
                icp += 5;
                continue;
            case OpEnd:
                return;
        }
    }
}

int Run(String s){
    if(Compile(s)<0){
        return 1;
    }
    Execute();
    return 0;
}

int main(int argc, char** argv){
    unsigned char buff[SIZE];
    int i;
    Parser(tcInit, tc);
    if(argc>=2){
        if(LoadText((String)argv[1], buff)==0){
            Run(buff);
        }
        exit(EXIT_SUCCESS);
    }
    for(;;){
        fprintf(stderr, "\n>");
        fgets(buff, SIZE, stdin);
        int buff_len = strlen(buff);
        if(strncmp(buff, "run ", 4)==0){
            if(buff[buff_len-1]=='\n'){
                buff[buff_len-1] = '\0';
            }
            if(LoadText(&buff[4], buff)==0){
                Run(buff);
            }
        }else if(strncmp(buff, "exit", 4)==0){
            exit(EXIT_SUCCESS);
        }else{//コマンド指定なしは、与えられた文字列をそのまま実行
            Run(buff);
        }
    }
}