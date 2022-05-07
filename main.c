#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "dcl.h"

char *code_head;
token *tk;
local *local_head;
node *prg[100];

void error(token *token, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = token->str - code_head;
    fprintf(stderr, "%s\n", code_head);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "incorrect number of arguments");
        return 1;
    }

    code_head = argv[1];
    tk = tokenize(code_head);
    local_head = calloc(1, sizeof(local));
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    for(int i = 0; prg[i]; i++){
        gen(prg[i]);
        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}