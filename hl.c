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
// "; . !!* 0 1 2 3 4 5 6 7 8 == != < >= <= >";
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

int Run(String s){
    clock_t t0 = clock();
    int pc, pc1;
    pc1 = Parser(s, tc);
    tc[pc1] = TcSemi;
    pc1++;
    token_size = pc1;
    //PrintToken(tc);
    for(pc=0; pc<pc1; pc++){
        if (phrCmp(0, "!!*0:", pc)){//pcは番地でないので、呼び出し先で変更しても、呼び出し元ではラベルを示している。
            //var[tc[pc]] = pc+2;//ラベルの次を示すようにする。
            var[tc[pc]] = ppc1; // ラベル定義命令の次のpc値を記憶させておく.
        }
    }
    int semi = GetTc(";", 1);//semiのトークンIDを返す
    tc[pc1++] = TcSemi; 
    for(pc=0; pc<pc1;){
        if(phrCmp(1, "!!*0 = !!*1;", pc)){
            var[tc[wpc[0]]] = var[tc[wpc[1]]];
        }else if(phrCmp(2, "!!*0 = !!*1 + !!*2;", pc)){
            var[tc[wpc[0]]] = var[tc[wpc[1]]] + var[tc[wpc[2]]];
        }else if(phrCmp(3, "!!*0 = !!*1 - !!*2;", pc)){
            var[tc[wpc[0]]] = var[tc[wpc[1]]] - var[tc[wpc[2]]];
        }else if(phrCmp(4, "!!*0 = !!*1 * !!*2;", pc)){
            var[tc[wpc[0]]] = var[tc[wpc[1]]] * var[tc[wpc[2]]];
        }else if(phrCmp(0, "!!*0:", pc)){
            //forループの最後でpcは変更される
        }else if(phrCmp(5, "goto !!*0;", pc)){
            pc = var[tc[wpc[0]]];
            continue;//forループの最後でppcに変更される。それを防ぐためにcontinue
        }else if (phrCmp( 6, "if (!!*0 !!*1 !!*2) goto !!*3;", pc) && TcEEq <= tc[wpc[1]] && tc[wpc[1]] <= TcGt) {
            int label = var[tc[wpc[3]]], left = var[tc[wpc[0]]], cc = tc[wpc[1]], right = var[tc[wpc[2]]];
            if(cc == TcEEq && left != right){ 
                pc = label;
                continue; 
            }
            if(cc == TcNEq && left == right){ 
                pc = label;
                continue;
            }
            if(cc == TcLt  && left <  right){ 
                pc = label;
                continue;
            }
        }else if (phrCmp( 7, "time;", pc)) {
            printf("time: %.3f[sec]\n", (clock() - t0) / (double) CLOCKS_PER_SEC);
        }else if(phrCmp(8, "print !!*0;", pc)){
            printf("%d\n", var[tc[wpc[0]]]);
        }else if (phrCmp(9, ";", pc)) {
            //semiが2回来ることがあるが、無視
        }else{
            goto error;
        }
        pc = ppc1;
    }
    return 0;
error:
    fprintf(stderr, "syntax error : %s %s %s %s\n", ts[tc[pc]], ts[tc[pc + 1]], ts[tc[pc + 2]], ts[tc[pc + 3]]);
    return 1;
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