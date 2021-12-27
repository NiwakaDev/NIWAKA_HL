#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define SIZE 10000
#define END 0
#define VAR_SIZE 256


void LoadText(int argc, const char** argv, uint8_t* buff){
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

void PrintBuff(uint8_t* buff){
    for(int i=0; buff[i]!=END; i++){
        fprintf(stderr, "%c", buff[i]);
    }
}

bool IsCtrlChar(char ascii_code){
    return ascii_code=='\n'||ascii_code=='\r'||ascii_code==' '||ascii_code=='\t'||ascii_code==';';
}

int main(const int argc, const char** argv){
    uint8_t* buff = (uint8_t*)malloc(SIZE);
    LoadText(argc, argv, buff);

    int var[VAR_SIZE];
    for(int i=0; i<VAR_SIZE; i++){
        var[i] = 0;
    }
    for(int i=0; i<10; i++){
        var['0'+i] = i;
    }
    for(int pc=0; buff[pc]!=END; pc++){
        if(IsCtrlChar(buff[pc])){
            continue;
        }else if(buff[pc+1]=='='&&buff[pc+3]==';'){//代入
            var[buff[pc]] = var[buff[pc+2]];
        }else if(buff[pc+1]=='='&&buff[pc+3]=='+'&&buff[pc+5]==';'){//足し算
            var[buff[pc]] = var[buff[pc+2]] + var[buff[pc+4]];
        }else if(buff[pc+1]=='='&&buff[pc+3]=='-'&&buff[pc+5]==';'){//引き算
            var[buff[pc]] = var[buff[pc+2]] - var[buff[pc+4]];
        }else if(buff[pc+1]=='='&&buff[pc+3]=='*'&&buff[pc+5]==';'){//掛け算
            var[buff[pc]] = var[buff[pc+2]] * var[buff[pc+4]];
        }else if(buff[pc] == 'p'&&buff[pc + 1]=='r'&&buff[pc + 5]==' '&&buff[pc + 7] == ';'){
            printf("%d\n", var[buff[pc+6]]);
        }else{
            goto error;
        }
        while(buff[pc]!=';'){//文の末端まで進める
            pc++;
        }
    }
    goto finish;
error:
    fprintf(stderr, "syntax error\n");
finish:
    free(buff);
}
