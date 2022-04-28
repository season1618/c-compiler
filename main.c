#include <stdio.h>

#include "dcl.h"

char *code_head;
token *tk;

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "incorrect number of arguments");
        return 1;
    }

    code_head = argv[1];
    tk = tokenize(argv[1]);
    node *nd = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    calc(nd);
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}