#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dcl.h"

token *tk;
node **prg;
local *local_head;

node *func_def();
void *dec();
node *stmt();
node *expr();
node *assign();
node *equal();
node *relational();
node *add();
node *mul();
node *unary();
node *primary();

bool expect(char *op){
    if(tk->len == strlen(op) && memcmp(tk->str, op, tk->len) == 0){
        tk = tk->next;
        return true;
    }else{
        return false;
    }
}

type *type_ptr(type *base){
    type *ty = calloc(1, sizeof(type));
    ty->kind = PTR;
    ty->ptr_to = base;
    return ty;
}

type *get_type(){
    if(tk->kind != TK_TYPE){
        error(tk, "unexpected token");
    }
    type *ty = calloc(1, sizeof(type));
    if(expect("int")){
        ty->kind = INT;
    }
    while(expect("*")){
        ty = type_ptr(ty);
    }
    return ty;
}

int get_number(){
    if(tk->kind != TK_NUM){
        error(tk, "unexpected token");
    }
    int val = tk->val;
    tk = tk->next;
    return val;
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

void *add_local(){
    local *var = calloc(1, sizeof(local));
    var->next = local_head;
    var->ty = get_type();
    var->name = tk->str;
    var->len = tk->len;
    var->offset = local_head->offset + 8;

    local_head = var;
    tk = tk->next;
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
    nd->ty = calloc(1, sizeof(type));

    switch(kind){
        case ND_ASSIGN:
            nd->ty = rhs->ty;
            break;
        case ND_EQ:
        case ND_NEQ:
        case ND_LT:
        case ND_LEQ:
            if(lhs->ty->kind == rhs->ty->kind && lhs->ty->ptr_to == rhs->ty->ptr_to){
                nd->ty->kind = INT;
            }else{
                error(tk, "invalid operands to binary operator");
            }
            break;
        case ND_ADD:
        case ND_SUB:
            if(lhs->ty->ptr_to == rhs->ty){
                nd->ty = lhs->ty;
            }else if(rhs->ty->ptr_to == lhs->ty){
                nd->ty = rhs->ty;
            }else if(lhs->ty->kind == INT && rhs->ty->kind == INT){
                nd->ty = lhs->ty;
            }else{
                error(tk, "invalid operands to binary + or -");
            }
            break;
        case ND_MUL:
        case ND_DIV:
            if(lhs->ty->kind == INT && rhs->ty->kind == INT){
                nd->ty->kind = INT;
            }else{
                error(tk, "invalid operands to binary * or ");
            }
            break;
    }
    return nd;
}

node *node_unary(node_kind kind, node *op){
    node *nd = calloc(1, sizeof(node));
    nd->kind = kind;
    nd->lhs = op;
    nd->ty = calloc(1, sizeof(type));

    switch(kind){
        case ND_ADR:
            nd->ty = calloc(1, sizeof(type));
            nd->ty->kind = PTR;
            nd->ty->ptr_to = op->ty;
            break;
        case ND_DEREF:
            if(op->ty->kind == INT){
                error(tk, "invalid type argument of unary '*' (have 'int')");
            }
            nd->ty = op->ty->ptr_to;
            break;
    }
    return nd;
}

node *node_num(int val){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_NUM;
    nd->ty = calloc(1, sizeof(type));
    nd->ty->kind = INT;
    nd->val = val;
    return nd;
}

node **program(token *token_head){
    tk = token_head;

    prg = calloc(100, sizeof(node*));
    int i = 0;
    while(!is_eof()){
        if(tk->kind == TK_TYPE){
            prg[i] = func_def();
            i++;
            continue;
        }
        // if(tk->kind == TK_TYPE){
        //     dec();
        //     continue;
        // }
        error(tk, "return type is unknown");
    }
    prg[i] = NULL;
    return prg;
}

node *func_def(){
    node *nd = calloc(1, sizeof(node));
    func *fn = calloc(1, sizeof(func));
    local_head = calloc(1, sizeof(local));

    nd->kind = ND_FUNC_DEF;
    nd->fn = fn;
    fn->ty = get_type();
    fn->name = tk->str;
    fn->len = tk->len;
    fn->arg_num = 0;

    tk = tk->next;
    expect("(");
    while(!expect(")")){
        if(tk->kind == TK_TYPE){
            add_local();
        }else{
            error(tk, "expected type");
        }
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
        fn->local_size = local_head->offset;
        return nd;
    }else{
        error(tk, "expected '{'");
    }
}

void *dec(){
    add_local();
    if(!expect(";")) error(tk, "expected ';'");
}

node *stmt(){
    node *nd = calloc(1, sizeof(node));
    
    if(expect("{")){
        nd->kind = ND_BLOCK;
        nd->head = calloc(1, sizeof(node));
        node *cur = nd->head;
        while(!expect("}")){
            if(tk->kind == TK_TYPE){
                dec();
            }
            else{
                cur->next = stmt();
                cur = cur->next;
            }
        }
    }
    else if(expect("if")){
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
        return node_unary(ND_ADR, unary());
    }
    if(expect("*")){
        return node_unary(ND_DEREF, unary());
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

        nd->ty = calloc(1, sizeof(type));
        nd->ty->kind = INT;
        for(int i = 0; i < 100; i++){
            if(prg[i] && prg[i]->kind == ND_FUNC_DEF && memcmp(prg[i]->fn->name, fn->name, fn->len) == 0){
                nd->ty = prg[i]->fn->ty;
                break;
            }
        }

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
            nd->ty = var->ty;
            nd->offset = var->offset;
        }else{
            error(tk, "'%.*s' is undeclared", tk->len, tk->str);
        }
        tk = tk->next;
        return nd;
    }
    return node_num(get_number());
}