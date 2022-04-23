#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} token_kind;

typedef struct token token;

struct token {
    token_kind kind;
    token *next;
    int val; // kind == TK_NUM
    char *str;
};

token *next_token(token_kind kind, token *cur, char *p){
    token *nxt = calloc(1, sizeof(token));
    cur->next = nxt;
    nxt->kind = kind;
    nxt->str = p;
    return nxt;
}

char *code_head;

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

token *tokenize(char *p){
    token *head = calloc(1, sizeof(token));
    head->next = NULL;
    token *cur = head;

    while(*p){
        if(isspace(*p)){
            p++;
            continue;
        }
        if(*p == '+' || *p == '-'){
            cur = next_token(TK_RESERVED, cur, p);
            p++;
            continue;
        }
        if(isdigit(*p)){
            cur = next_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        
        error(cur, "can't tokenize");
    }
    next_token(TK_EOF, cur, p);
    return head->next;
}

token *tk;

int get_number(){
    if(tk->kind != TK_NUM){
        error(tk, "a first token is not a number");
    }
    int val = tk->val;
    tk = tk->next;
    return val;
}

bool expect(char op){
    if(tk->kind == TK_RESERVED && tk->str[0] == op){
        tk = tk->next;
        return true;
    }else{
        return false;
    }
}

bool is_eof(){
    return tk->kind == TK_EOF;
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "incorrect number of arguments");
        return 1;
    }

    code_head = argv[1];
    tk = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    printf("    mov rax, %d\n", get_number());
    while(!is_eof()){
        if(expect('+')){
            printf("    add rax, %d\n", get_number());
            continue;
        }
        if(expect('-')){
            printf("    sub rax, %d\n", get_number());
            continue;
        }

        error(tk, "unexpected operand");
    }
    printf("    ret\n");
    return 0;
}