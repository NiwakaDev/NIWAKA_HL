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
    return ascii_code == ' ' || ascii_code == '\t' || ascii_code == '\n' || ascii_code == '\r';
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
            while(strchr("=+-*/!%&~|<>?:.#", s[i + len])!=NULL){
                len++;
            }
        }else{
            fprintf(stderr, "syntax error\n");
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
    int tc[SIZE];//ユニークでないトークン配列
    pc1 = Parser(s, tc);
    tc[pc1] = GetTc(";", 1);
    pc1++;
    token_size = pc1;
    //PrintToken(tc);
    for(pc=0; pc<pc1; pc++){
        if(tc[pc+1]==GetTc(":", 1)){
            var[tc[pc]] = pc+2;//ラベルの次を示すようにする。
        }
    }
    int semi = GetTc(";", 1);//semiのトークンIDを返す
    for(pc=0; pc<pc1;){
        if(tc[pc+1]==GetTc("=", 1)&&tc[pc+3]==semi){
            var[tc[pc]] = var[tc[pc+2]];
        }else if(tc[pc+1]==GetTc("=", 1)&&tc[pc+3]==GetTc("+", 1)&&tc[pc+5]==semi){
            var[tc[pc]] = var[tc[pc+2]] + var[tc[pc+4]];
        }else if(tc[pc+1]==GetTc("=", 1)&&tc[pc+3]==GetTc("-", 1)&&tc[pc+5]==semi){
            var[tc[pc]] = var[tc[pc+2]] - var[tc[pc+4]];
        }else if(tc[pc+1]==GetTc("=", 1)&&tc[pc+3]==GetTc("*", 1)&&tc[pc+5]==semi){
            var[tc[pc]] = var[tc[pc+2]] * var[tc[pc+4]];
        }else if(tc[pc+1]==GetTc(":", 1)){
            pc += 2;
            continue;
        }else if(tc[pc]==GetTc("goto", 4)&&tc[pc+2]==semi){
            pc = var[tc[pc+1]];
            continue;
        }else if(tc[pc]==GetTc("if", 2)&&tc[pc+1]==GetTc("(", 1)&&tc[pc+5]==GetTc(")", 1)&&tc[pc+6]==GetTc("goto", 4)&&tc[pc+8]==semi){//if ( 変数1 条件式 変数2 ) goto 
            int label = var[tc[pc+7]];
            int v0  = var[tc[pc+2]];//条件式の左辺
            int v1  = var[tc[pc+4]];//条件式の右辺
            if(tc[pc+3]==GetTc("!=", 2)&&v0!=v1){
                pc = label;
                continue;
            }else if(tc[pc+3]==GetTc("==", 2)&&v0==v1){
                pc = label;
                continue;
            }else if(tc[pc+3]==GetTc("<", 1)&&v0<v1){
                pc = label;
                continue;
            }
        }else if(tc[pc]==GetTc("time", 4)&&tc[pc+1]==semi){
            printf("time: %.3f[sec]\n", (clock()-t0) / (double) CLOCKS_PER_SEC);
        }else if(tc[pc]==GetTc("print", 5)&&tc[pc + 2]==semi){
            printf("%d\n", var[tc[pc+1]]);
        }else if(tc[pc]==semi){
            //semiが2回来ることがあるが、無視
        }else{
            goto error;
        }
        while(tc[pc]!=semi){
            pc++;
        }
        pc++;
    }
    return 0;
error:
    fprintf(stderr, "syntax error : %s %s %s %s\n", ts[tc[pc]], ts[tc[pc + 1]], ts[tc[pc + 2]], ts[tc[pc + 3]]);
    return 1;
}

int main(int argc, char** argv){
    unsigned char buff[SIZE];
    int i;
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