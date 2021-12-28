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

void LoadText(int argc, const char** argv, unsigned char* buff){
    FILE* input_stream;
    if(argc<2){
        fprintf(stderr, "Error: no input file\n");
        exit(EXIT_FAILURE);
    }
    input_stream = fopen(argv[1], "rt");
    if(input_stream==NULL){
        fprintf(stderr, "Error: fopen\n");
        exit(EXIT_FAILURE);
    }
    int read_size = fread(buff, 1, SIZE-1, input_stream);
    fclose(input_stream);
    buff[read_size] = END;
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

int main(int argc, char** argv){
    int pc, pc1;
    unsigned char buff[SIZE];
    int tc[SIZE];//ユニークでないトークン配列
    LoadText(argc, argv, buff);
    pc1 = Parser(buff, tc);
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
            printf("time: %.3f[sec]\n", clock() / (double) CLOCKS_PER_SEC);
        }else if(tc[pc]==GetTc("print", 5)&&tc[pc + 2]==semi){
            printf("%d\n", var[tc[pc+1]]);
        }else{
            goto error;
        }
        while(tc[pc]!=semi){
            pc++;
        }
        pc++;
    }
    exit(EXIT_SUCCESS);
error:
    fprintf(stderr, "syntax error : %s %s %s %s\n", ts[tc[pc]], ts[tc[pc + 1]], ts[tc[pc + 2]], ts[tc[pc + 3]]);
    exit(EXIT_FAILURE);
}