#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcl.h"

char *file_name;
char *code_head;

void error(token *tk, char *msg){
    char *begin = tk->str;
    char *end = tk->str;
    while(code_head < begin && *(begin - 1) != '\n') begin--;
    while(*end != '\n') end++;

    int line_num = 1;
    for(char *p = code_head; p < begin; p++){
        if(*p == '\n') line_num++;
    }

    int indent = fprintf(stderr, "%s:%d: ", file_name, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - begin), begin);

    int pos = tk->str - begin + indent;
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ %s\n", msg);
    
    exit(1);
}

char *read_file(char *path){
    FILE *fp = fopen(path, "r");
    if(!fp){
        fprintf(stderr, "cannot open %s: %s\n", path, strerror(errno));
        exit(1);
    }
    if(fseek(fp, 0, SEEK_END) == -1){
        fprintf(stderr, "%s: fseek: %s\n", path, strerror(errno));
        exit(1);
    }
    size_t size = ftell(fp);
    if(fseek(fp, 0, SEEK_SET) == -1){
        fprintf(stderr, "%s: fseek: %s\n", path, strerror(errno));
        exit(1);
    }

    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    if(size == 0 || buf[size - 1] != '\n'){
        buf[size] = '\n';
        size++;
    }
    buf[size] = '\0';
    fclose(fp);

    return buf;
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "incorrect number of arguments\n");
        return 1;
    }

    file_name = argv[1];
    code_head = read_file(file_name);
    token *token_head = tokenize(code_head);
    node *node_head = program(token_head);
    gen_code(node_head);
}