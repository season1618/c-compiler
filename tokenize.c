#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dcl.h"

char *p;
token *cur;

int NUM_TYPE = 17;
int NUM_KEYWORD = 12;
int NUM_PUNCT = 38;
int NUM_ESCAPE = 13;
char *types[] = {"extern", "const", "volatile", "static", "signed", "unsigned", "void", "_Bool", "char", "short", "int", "long", "float", "double", "struct", "union", "enum"};
char *keywords[] = {"include", "typedef", "return", "if", "else", "switch", "case", "while", "for", "continue", "break", "sizeof"};
char *puncts[] = {
    "+=", "-=", "*=", "/=", "%=", "||", "&&", "==", "!=", "<=", ">=", "<<", ">>", "++", "--", "->", 
    "#", "=", "?", ":", "<", ">", "+", "-", "*", "/", "%", "&", "!", ".", ",", ";", "(", ")", "{", "}", "[", "]"
};
char escape[2][13] = {
    { 'a',  'b',  'e',  'f',  'n',  'r',  't',  'v', '\\', '\'', '\"',  '?',  '0'},
    {'\a', '\b', '\e', '\f', '\n', '\r', '\t', '\v', '\\', '\'', '\"', '\?', '\0'}
};

bool is_alpha(char c){
    return c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_alnum(char c){
    return is_alpha(c) || isdigit(c);
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

void next_ident(){
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

void next_number(){
    token *nxt = calloc(1, sizeof(token));
    nxt->kind = TK_NUM;
    nxt->str = p;
    nxt->len = 0;
    nxt->val = 0;
    while(isdigit(*p)){
        nxt->val *= 10;
        nxt->val += *p - '0';
        nxt->len++;
        p++;
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
    while(true){
        if(*p == '\\'){
            p += 2;
            nxt->len += 2;
            continue;
        }
        if(*p != '\''){
            p++;
            nxt->len++;
            continue;
        }
        p++;
        nxt->len++;
        break;
    }

    if(nxt->len == 3){
        nxt->val = nxt->str[1];
    }
    if(nxt->len == 4 && nxt->str[1] == '\\'){
        for(int i = 0; i < NUM_ESCAPE; i++){
            if(nxt->str[2] == escape[0][i]){
                nxt->val = escape[1][i];
                break;
            }
        }
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
    while(true){
        if(*p == '\\'){
            p += 2;
            nxt->len += 2;
            continue;
        }
        if(*p != '\"'){
            p++;
            nxt->len++;
            continue;
        }
        p++;
        nxt->len++;
        break;
    }

    cur->next = nxt;
    cur = nxt;
}

bool read_type(){
    for(int i = 0; i < NUM_TYPE; i++){
        char *s = types[i];
        int len = strlen(s);
        if(memcmp(p, s, len) == 0 && !is_alnum(p[len])){
            next_token(TK_TYPE, len);
            return true;
        }
    }
    return false;
}

bool read_keyword(){
    for(int i = 0; i < NUM_KEYWORD; i++){
        char *s = keywords[i];
        int len = strlen(s);
        if(memcmp(p, s, len) == 0 && !is_alnum(p[len])){
            next_token(TK_RESERVED, len);
            return true;
        }
    }
    return false;
}

bool read_punct(){
    for(int i = 0; i < NUM_PUNCT; i++){
        char *s = puncts[i];
        int len = strlen(s);
        if(memcmp(p, s, len) == 0){
            next_token(TK_RESERVED, len);
            return true;
        }
    }
    return false;
}

token *tokenize(char *code_head){
    p = code_head;
    token *token_head = calloc(1, sizeof(token));
    cur = token_head;

    while(*p){
        // skip white space
        if(isspace(*p)){
            p++;
            continue;
        }
        // skip linemarker
        if(*p == '#'){
            while(*p != '\n'){
                p++;
            }
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
        // skip __attribute__
        if(memcmp(p, "__attribute__", 13) == 0){
            while(*p != ';') p++;
            continue;
        }
        if(memcmp(p, "__extension__", 13) == 0){
            p += 13;
            continue;
        }
        if(memcmp(p, "__inline", 8) == 0){
            p += 8;
            continue;
        }
        if(memcmp(p, "__asm__", 7) == 0){
            while(*p != '\n') p++;
            continue;
        }
        if(memcmp(p, "__restrict", 10) == 0){
            p += 10;
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
        if(read_type()) continue;
        if(read_keyword()) continue;
        if(read_punct()) continue;
        if(memcmp(p, "true", 4) == 0 && !is_alnum(p[4])){
            token *nxt = calloc(1, sizeof(token));
            nxt->kind = TK_NUM;
            nxt->str = p;
            nxt->len = 4;
            nxt->val = 1;
            cur->next = nxt;
            cur = nxt;
            p += 4;
            continue;
        }
        if(memcmp(p, "false", 5) == 0 && !is_alnum(p[5])){
            token *nxt = calloc(1, sizeof(token));
            nxt->kind = TK_NUM;
            nxt->str = p;
            nxt->len = 5;
            nxt->val = 0;
            cur->next = nxt;
            cur = nxt;
            p += 5;
            continue;
        }
        if(isdigit(*p)){
            next_number();
            continue;
        }
        if(is_alpha(*p)){
            next_ident();
            continue;
        }
        error(cur, "invalid token");
    }
    next_token(TK_EOF, 0);
    return token_head->next;
}