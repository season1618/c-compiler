#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include<stdio.h>

#include "dcl.h"

char *p;
token *cur;

int NUM_KEYWORD = 10;
int NUM_TYPE = 5;
int NUM_PUNCT = 34;
char *keywords[] = {"typedef", "return", "if", "else", "switch", "while", "for", "continue", "break", "sizeof"};
char *types[] = {"void", "char", "int", "struct", "enum"};
char *puncts[] = {
    "+=", "-=", "*=", "/=", "%=", "||", "&&", "==", "!=", "<=", ">=", "->", "++", "--",
    "=", "<", ">", "+", "-", "*", "/", "%", "&", "!", ".", ":", ",", ";", "(", ")", "{", "}", "[", "]"
};

bool is_alpha(char c){
    return c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_alnum(char c){
    return is_alpha(c) || isdigit(c);
}

bool fwdmatch(char *s){
    return memcmp(p, s, strlen(s)) == 0;
}

void next_token(token_kind kind, int len){
    token *nxt = calloc(1, sizeof(token));
    nxt->kind = kind;
    nxt->str = p;
    nxt->len = len;

    cur->next = nxt;
    cur = nxt;
    p += len;
}

void next_number(){
    token *nxt = calloc(1, sizeof(token));
    nxt->kind = TK_NUM;
    nxt->val = 0;
    nxt->str = p;
    nxt->len = 0;
    while(isdigit(*p)){
        nxt->val *= 10;
        nxt->val += *p - '0';
        nxt->len++;
        p++;
    }
    cur->next = nxt;
    cur = nxt;
}

void next_identifier(){
    token *nxt = calloc(1, sizeof(token));
    nxt->kind = TK_ID;
    nxt->str = p;
    nxt->len = 0;
    while(is_alnum(*p)){
        p++;
        nxt->len++;
    }
    cur->next = nxt;
    cur = nxt;
}

void next_char(){
    token *nxt = calloc(1, sizeof(token));
    nxt->kind = TK_CHAR;
    nxt->str = p;
    nxt->len = 1;
    p++;
    while(*p != '\''){
        p++;
        nxt->len++;
    }
    p++;
    nxt->len++;

    if(nxt->len > 3){
        error(cur, "multi character constant");
    }

    cur->next = nxt;
    cur = nxt;
}

void next_string(){
    token *nxt = calloc(1, sizeof(token));
    nxt->kind = TK_STRING;
    nxt->str = p;
    nxt->len = 1;
    p++;
    while(*p != '\"'){
        p++;
        nxt->len++;
    }
    p++;
    nxt->len++;

    cur->next = nxt;
    cur = nxt;
}

bool read_keyword(){
    for(int i = 0; i < NUM_KEYWORD; i++){
        if(fwdmatch(keywords[i]) && !is_alnum(p[strlen(keywords[i])])){
            next_token(TK_KEYWORD, strlen(keywords[i]));
            return true;
        }
    }
    return false;
}

bool read_type(){
    for(int i = 0; i < NUM_TYPE; i++){
        if(fwdmatch(types[i]) && !is_alnum(p[strlen(types[i])])){
            next_token(TK_TYPE, strlen(types[i]));
            return true;
        }
    }
    return false;
}

bool read_punct(){
    for(int i = 0; i < NUM_PUNCT; i++){
        if(fwdmatch(puncts[i])){
            next_token(TK_PUNCT, strlen(puncts[i]));
            return true;
        }
    }
    return false;
}

token *tokenize(char *code_head){
    p = code_head;

    token *head = calloc(1, sizeof(token));
    cur = head;

    while(*p){
        // skip white space
        if(isspace(*p)){
            p++;
            continue;
        }
        // skip line comment
        if(memcmp(p, "//", 2) == 0){
            p += 2;
            while(*p != '\n'){
                p++;
            }
            continue;
        }
        // skip block comment
        if(memcmp(p, "/*", 2) == 0){
            p += 2;
            while(memcmp(p, "*/", 2)){
                p++;
            }
            p += 2;
            continue;
        }
        
        if(*p == '\''){
            next_char();
            continue;
        }
        if(*p == '\"'){
            next_string();
            continue;
        }
        if(read_keyword()) continue;
        if(read_type()) continue;
        if(read_punct()) continue;
        if(isdigit(*p)){
            next_number();
            continue;
        }
        if(is_alpha(*p)){
            next_identifier();
            continue;
        }
        error(cur, "invalid token");
    }
    next_token(TK_EOF, 0);

    // cur = head->next;
    // while(cur->kind != TK_EOF){
    //     fprintf(stderr, "%.*s ", cur->len, cur->str);
    //     cur = cur->next;
    // }
    // fprintf(stderr, "\n");
    return head->next;
}