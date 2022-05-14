#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dcl.h"

token *tk;
local *local_head;

node *func_def();
node *stmt();
node *expr();
node *assign();
node *equal();
node *relational();
node *add();
node *mul();
node *unary();
node *primary();

int get_number(){
    if(tk->kind != TK_NUM){
        error(tk, "unexpected token");
    }
    int val = tk->val;
    tk = tk->next;
    return val;
}

bool expect(char *op){
    if(tk->len == strlen(op) && memcmp(tk->str, op, tk->len) == 0){
        tk = tk->next;
        return true;
    }else{
        return false;
    }
}

char *get_id(){
    if(tk->kind != TK_ID){
        error(tk, "unexpected token");
    }
    char *str = tk->str;
    tk = tk->next;
    return str;
}

bool is_eof(){
    return tk->kind == TK_EOF;
}

local *find_local(){
    for(local *var = local_head; var; var = var->next){
        if(var->len == tk->len && memcmp(var->name, tk->str, var->len) == 0){
            return var;
        }
    }
    return NULL;
}

node *node_binary(node_kind kind, node *lhs, node *rhs){
    node *nd = calloc(1, sizeof(node));
    nd->kind = kind;
    nd->lhs = lhs;
    nd->rhs = rhs;
    return nd;
}

node *node_unary(node_kind kind, node *op){
    node *nd = calloc(1, sizeof(node));
    nd->kind = kind;
    nd->lhs = op;
    return nd;
}

node *node_num(int val){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_NUM;
    nd->val = val;
    return nd;
}

node **program(token *token_head){
    tk = token_head;

    node **prg = calloc(100, sizeof(node*));
    int i = 0;
    while(!is_eof()){
        prg[i] = func_def();
        i++;
    }
    prg[i] = NULL;
    return prg;
}

node *func_def(){
    if(tk->kind == TK_ID && tk->next->kind != TK_EOF && tk->next->str[0] == '('){
        node *nd = calloc(1, sizeof(node));
        func *fn = calloc(1, sizeof(func));
        local_head = calloc(1, sizeof(local));

        nd->kind = ND_FUNC_DEF;
        nd->fn = fn;
        fn->name = tk->str;
        fn->len = tk->len;
        fn->arg_num = 0;

        tk = tk->next;
        expect("(");
        while(!expect(")")){
            expr(); // var_def
            fn->arg_num++;
            if(expect(",")){
                continue;
            }else{
                if(expect(")")){
                    break;
                }else{
                    error(tk, "expected ',' or ')'");
                }
            }
        }
        if(tk->str[0] == '{'){
            fn->stmt = stmt();
            fn->local_num = local_head->index;
            return nd;
        }else{
            error(tk, "expected '{'");
        }
    }
}

node *stmt(){
    node *nd = calloc(1, sizeof(node));
    
    if(expect("if")){
        if(!expect("(")) error(tk, "expected '('");

        nd->kind = ND_IF;
        nd->elms = calloc(3, sizeof(node));
        nd->elms[0] = expr();

        if(!expect(")")) error(tk, "expected ')'");

        nd->elms[1] = stmt();

        if(expect("else")){
            nd->elms[2] = stmt();
        }
    }
    else if(expect("while")){
        if(!expect("(")) error(tk, "expected '('");

        nd->kind = ND_WHILE;
        nd->elms = calloc(2, sizeof(node));
        nd->elms[0] = expr();

        if(!expect(")")) error(tk, "expected ')'");

        nd->elms[1] = stmt();
    }
    else if(expect("for")){
        if(!expect("(")) error(tk, "expected '('");

        nd->kind = ND_FOR;
        nd->elms = calloc(4, sizeof(node));
        nd->elms[0] = expr();

        if(!expect(";")) error(tk, "expected ';'");

        nd->elms[1] = expr();

        if(!expect(";")) error(tk, "expected ';'");

        nd->elms[2] = expr();

        if(!expect(")")) error(tk, "expected ')'");

        nd->elms[3] = stmt();
    }
    else if(expect("return")){
        nd->kind = ND_RET;
        nd->lhs = expr();
        if(!expect(";")) error(tk, "expected ';'");
    }
    else if(expect("{")){
        nd->kind = ND_BLOCK;
        nd->head = calloc(1, sizeof(node));
        node *cur = nd->head;
        while(!expect("}")){
            cur->next = stmt();
            cur = cur->next;
        }
    }
    else{
        nd = expr();
        if(!expect(";")) error(tk, "expected ';'");
    }
    return nd;
}

node *expr(){
    return assign();
}

node *assign(){
    node *nd = equal();
    if(expect("=")){
        nd = node_binary(ND_ASSIGN, nd, assign());
    }
    return nd;
}

node *equal(){
    node *nd = relational();
    while(true){
        if(expect("==")){
            nd = node_binary(ND_EQ, nd, relational());
            continue;
        }
        if(expect("!=")){
            nd = node_binary(ND_NEQ, nd, relational());
            continue;
        }
        return nd;
    }
}

node *relational(){
    node *nd = add();
    while(true){
        if(expect("<")){
            nd = node_binary(ND_LT, nd, add());
            continue;
        }
        if(expect("<=")){
            nd = node_binary(ND_LEQ, nd, add());
            continue;
        }
        if(expect(">")){
            nd = node_binary(ND_LT, add(), nd);
            continue;
        }
        if(expect(">=")){
            nd = node_binary(ND_LEQ, add(), nd);
            continue;
        }
        return nd;
    }
}

node *add(){
    node *nd = mul();
    while(true){
        if(expect("+")){
            nd = node_binary(ND_ADD, nd, mul());
            continue;
        }
        if(expect("-")){
            nd = node_binary(ND_SUB, nd, mul());
            continue;
        }
        return nd;
    }
}

node *mul(){
    node *nd = unary();
    while(true){
        if(expect("*")){
            nd = node_binary(ND_MUL, nd, unary());
            continue;
        }
        if(expect("/")){
            nd = node_binary(ND_DIV, nd, unary());
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
        return node_binary(ND_SUB, node_num(0), unary());
    }
    if(expect("&")){
        return node_unary(ND_ADR, primary());
    }
    if(expect("*")){
        return node_unary(ND_DEREF, primary());
    }
    return primary();
}

node *primary(){
    if(expect("(")){
        node *nd = expr();
        expect(")");
        return nd;
    }
    if(tk->kind == TK_ID && tk->next->kind != TK_EOF && tk->next->str[0] == '('){
        node *nd = calloc(1, sizeof(node));
        func *fn = calloc(1, sizeof(func));
        nd->kind = ND_FUNC_CALL;
        nd->fn = fn;
        fn->name = tk->str;
        fn->len = tk->len;
        fn->arg_num = 0;

        tk = tk->next;
        expect("(");
        node *cur;
        while(!expect(")")){
            cur = expr();
            cur->next = fn->args_head;
            fn->args_head = cur;
            fn->arg_num++;
            if(expect(",")){
                continue;
            }else{
                if(expect(")")){
                    break;
                }else{
                    error(tk, "expected ',' or ')'");
                }
            }
        }
        return nd;
    }
    if(tk->kind == TK_ID){
        node *nd = calloc(1, sizeof(node));
        nd->kind = ND_LOCAL;

        local *var = find_local();
        if(var){
            nd->offset = 8 * var->index;
        }else{
            var = calloc(1, sizeof(local));
            var->next = local_head;
            var->name = tk->str;
            var->len = tk->len;
            var->index = local_head->index + 1;

            nd->offset = 8 * var->index;

            local_head = var;
        }
        tk = tk->next;
        return nd;
    }
    return node_num(get_number());
}