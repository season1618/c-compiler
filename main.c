#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "dcl.h"

char *code_head;

void error(token *tk, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = tk->str - code_head;
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
    token *token_head = tokenize(code_head);
    node *node_head = program(token_head);
    gen_code(node_head);
    return 0;
}