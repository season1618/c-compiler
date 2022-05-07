#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dcl.h"

char *p;
int NUM_KEYWORD = 5;
int NUM_PUNCT = 18;
char *keywords[] = {"return", "if", "else", "while", "for"};
char *puncts[] = {"==", "!=", "<=", ">=", "=", "+", "-", "*", "/", ";", "<", ">", "(", ")", "{", "}", "[", "]"};

token *next_token(token_kind kind, token *cur, int len){
    token *nxt = calloc(1, sizeof(token));
    nxt->kind = kind;
    nxt->str = p;
    nxt->len = len;

    cur->next = nxt;
    p += len;

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

int len_id(){
    int len = 0;
    while(isalnum_(*p)){
        p++;
        len++;
    }
    return len;
}

token *tokenize(char *code_head){
    p = code_head;

    token *head = calloc(1, sizeof(token));
    head->next = NULL;
    token *cur = head;

    while(*p){
        bool flag = false;
        if(isspace(*p)){
            p++;
            continue;
        }
        for(int i = 0; i < NUM_KEYWORD; i++){
            if(fwdmatch(p, keywords[i]) && !isalnum_(p[strlen(keywords[i])])){
                cur = next_token(TK_KEYWORD, cur, strlen(keywords[i]));
                flag = true;
                break;
            }
        }
        if(flag) continue;
        for(int i = 0; i < NUM_PUNCT; i++){
            if(fwdmatch(p, puncts[i])){
                cur = next_token(TK_PUNCT, cur, strlen(puncts[i]));
                flag = true;
                break;
            }
        }
        if(flag) continue;
        if(isdigit(*p)){
            cur = next_token(TK_NUM, cur, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        if(isalpha_(*p)){
            cur = next_token(TK_ID, cur, 0);
            cur->len = len_id();
            continue;
        }
        
        error(cur, "invalid token");
    }
    next_token(TK_EOF, cur, 0);
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