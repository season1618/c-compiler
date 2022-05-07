#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dcl.h"

token *next_token(token_kind kind, token *cur, char *p, int len){
    token *nxt = calloc(1, sizeof(token));
    nxt->kind = kind;
    nxt->str = p;
    nxt->len = len;
    cur->next = nxt;
    return nxt;
}

bool isalpha_(char c){
    return c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool isalnum_(char c){
    return isalpha_(c) || isdigit(c);
}

bool fwdmatch(char *s, char *t){
    return memcmp(s, t, strlen(t)) == 0;
}

int len_id(char *p){
    int len = 0;
    while(isalnum_(*p)){
        p++;
        len++;
    }
    return len;
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
        if(fwdmatch(p, "return") && !isalnum_(p[6])){
            cur = next_token(TK_RET, cur, p, 6);
            p += 6;
            continue;
        }
        if(fwdmatch(p, "if") && !isalnum_(p[2])){
            cur = next_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }
        if(fwdmatch(p, "else") && !isalnum_(p[4])){
            cur = next_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }
        if(fwdmatch(p, "while") && !isalnum_(p[5])){
            cur = next_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }
        if(fwdmatch(p, "for") && !isalnum_(p[3])){
            cur = next_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }
        if(fwdmatch(p, "==") || fwdmatch(p, "!=") || fwdmatch(p, "<=") || fwdmatch(p, ">=")){
            cur = next_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if(*p == ';' || *p == '=' || *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '<' || *p == '>' || *p == '(' || *p == ')' || *p == '{' || *p == '}'){
            cur = next_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }
        if(isdigit(*p)){
            cur = next_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        if(isalpha_(*p)){
            cur = next_token(TK_ID, cur, p, 0);
            cur->len = len_id(p);
            p += cur->len;
            continue;
        }
        
        error(cur, "invalid token");
    }
    next_token(TK_EOF, cur, p, 0);
    // cur = head->next;
    // while(cur->kind != TK_EOF){
    //     for(int i = 0; i < cur->len; i++){
    //         printf("%c", cur->str[i]);
    //     }
    //     cur = cur->next;
    //     printf(" ");
    // }
    // printf("\n");
    return head->next;
}