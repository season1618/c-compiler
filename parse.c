#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
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

bool fwdmatch(char *s, char *t){
    return memcmp(s, t, strlen(t)) == 0;
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
        if(fwdmatch(p, "==") || fwdmatch(p, "!=") || fwdmatch(p, "<=") || fwdmatch(p, ">=")){
            cur = next_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '<' || *p == '>' || *p == '(' || *p == ')'){
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

node *new_node(node_kind kind, node *lhs, node *rhs){
    node *nd = calloc(1, sizeof(node));
    nd->kind = kind;
    nd->lhs = lhs;
    nd->rhs = rhs;
    return nd;
}

node *new_node_num(int val){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_NUM;
    nd->val = val;
    return nd;
}

node *expr();
node *equal();
node *relational();
node *add();
node *mul();
node *unary();
node *primary();
int get_number();
bool expect(char*);
bool is_eof();

node *expr(){
    return equal();
}

node *equal(){
    node *nd = relational();
    while(true){
        if(expect("==")){
            nd = new_node(ND_EQ, nd, relational());
            continue;
        }
        if(expect("!=")){
            nd = new_node(ND_NEQ, nd, relational());
            continue;
        }
        return nd;
    }
}

node *relational(){
    node *nd = add();
    while(true){
        if(expect("<")){
            nd = new_node(ND_LT, nd, add());
            continue;
        }
        if(expect("<=")){
            nd = new_node(ND_LEQ, nd, add());
            continue;
        }
        if(expect(">")){
            nd = new_node(ND_LT, add(), nd);
            continue;
        }
        if(expect(">=")){
            nd = new_node(ND_LEQ, add(), nd);
            continue;
        }
        return nd;
    }
}

node *add(){
    node *nd = mul();
    while(true){
        if(expect("+")){
            nd = new_node(ND_ADD, nd, mul());
            continue;
        }
        if(expect("-")){
            nd = new_node(ND_SUB, nd, mul());
            continue;
        }
        return nd;
    }
}

node *mul(){
    node *nd = unary();
    while(true){
        if(expect("*")){
            nd = new_node(ND_MUL, nd, unary());
            continue;
        }
        if(expect("/")){
            nd = new_node(ND_DIV, nd, unary());
            continue;
        }
        return nd;
    }
}

node *unary(){
    if(expect("+")){
        return unary();
    }
    if(expect("-")){
        return new_node(ND_SUB, new_node_num(0), unary());
    }
    return primary();
}

node *primary(){
    if(expect("(")){
        node *nd = expr();
        expect(")");
        return nd;
    }
    return new_node_num(get_number());
}

int get_number(){
    if(tk->kind != TK_NUM){
        error(tk, "unexpected token");
    }
    int val = tk->val;
    tk = tk->next;
    return val;
}

bool expect(char *op){
    if(tk->kind == TK_RESERVED && tk->len == strlen(op) && memcmp(tk->str, op, tk->len) == 0){
        tk = tk->next;
        return true;
    }else{
        return false;
    }
}

bool is_eof(){
    return tk->kind == TK_EOF;
}