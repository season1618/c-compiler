#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dcl.h"

int NUM_KEYWORD = 5;
int NUM_PUNCT = 18;
char *keywords[] = {"return", "if", "else", "while", "for"};
char *puncts[] = {"==", "!=", "<=", ">=", "=", "+", "-", "*", "/", ";", "<", ">", "(", ")", "{", "}", "[", "]"};

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
        bool flag = false;
        if(isspace(*p)){
            p++;
            continue;
        }
        for(int i = 0; i < NUM_KEYWORD; i++){
            if(fwdmatch(p, keywords[i]) && !isalnum_(p[strlen(keywords[i])])){
                cur = next_token(TK_KEYWORD, cur, p, strlen(keywords[i]));
                p += strlen(keywords[i]);
                flag = true;
                break;
            }
        }
        if(flag) continue;
        for(int i = 0; i < NUM_PUNCT; i++){
            if(fwdmatch(p, puncts[i])){
                cur = next_token(TK_PUNCT, cur, p, strlen(puncts[i]));
                p += strlen(puncts[i]);
                flag = true;
                break;
            }
        }
        if(flag) continue;
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