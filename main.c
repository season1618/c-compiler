#include <stdio.h>
#include <stdlib.h>

#include "dcl.h"

char *code_head;
token *tk;
local *local_head;
node *code[100];

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "incorrect number of arguments");
        return 1;
    }

    code_head = argv[1];
    tk = tokenize(argv[1]);
    local_head = calloc(1, sizeof(local));
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("  sub rsp, 16\n");

    for(int i = 0; code[i]; i++){
        gen(code[i]);
        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}