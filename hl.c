#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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
String ts[MAX_TC+1];//トークンの内容を記憶
int tl[MAX_TC+1];//トークンの長さ
unsigned char tcBuff[(MAX_TC+1)*10];
int tcs=0, tcb=0;
int var[MAX_TC+1];

int GetTc(String s, int len){
    int i;
    for(i=0; i<tcs; i++){
        if(len==tl[i]&&strncmp(s, ts[i], len)==0){
            break;
        }
    }
    //末端まできたら、未登録の変数
    if(i==tcs){
        if(tcs>=MAX_TC){
            fprintf(stderr, "too many tokens");
            exit(EXIT_FAILURE);
        }
        strncpy(&tcBuff[tcb], s, len);
        tcBuff[tcb+len] = 0;
        tl[i] = len;
        tcb += len + 1;
        tcs++; 
        var[i] = strtol(ts[i], 0, 0);
    }
    return i;
}

bool isAlphabetOrNumber(unsigned char ascii_code){
    if('0'<=ascii_code&&ascii_code<='9'){
        return true;
    }else if('a'<=ascii_code&&ascii_code<='z'){
        return true;
    }else if('A'<=ascii_code&&ascii_code<='Z'){
        return true;
    }
    return false;
}

bool IsCtrl(unsigned char ascii_code){
    return ascii_code==' '||ascii_code=='\t' || ascii_code == '\n' || ascii_code == '\r';
}

int Parser(String s, int tc[]){
    int i=0;
    int j=0;//トークン列のサイズ
    int len;
    for(;;){
        if (IsCtrl(s[i])) {	// スペース、タブ、改行.
            i++;
            continue;
        }
        if (s[i] == END){
            return j;
        }
        len = 0;
        if(strchr("(){}[];,", s[i])!=NULL){
            len = 1;
        }else if(isAlphabetOrNumber(s[i])){
            while(isAlphabetOrNumber(s[i+len])){
                len++;
            }else if(strchr("=+-*/!%&~|<>?:.#", s[i])!=NULL){
                while(){
                    
                }
            }
        }
    }
}

int main(int argc, char** argv){

}