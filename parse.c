#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dcl.h"

token *cur;
node **prg;
symb *global_head;
symb *local_head;
int prg_num = 0;
int lc_num = 0;

void dcl();
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
    if(cur->len == strlen(op) && memcmp(cur->str, op, cur->len) == 0){
        cur = cur->next;
        return true;
    }else{
        return false;
    }
}

type *type_base(type_kind kind){
    type *ty = calloc(1, sizeof(type));
    ty->kind = kind;
    return ty;
}

type *type_ptr(type *base){
    type *ty = calloc(1, sizeof(type));
    ty->kind = PTR;
    ty->ptr_to = base;
    return ty;
}

type *type_array(type *base, size_t size){
    type *ty = calloc(1, sizeof(type));
    ty->kind = ARRAY;
    ty->ptr_to = base;
    ty->arr_size = size;
    return ty;
}

int size_of(type *ty){
    switch(ty->kind){
        case CHAR:
            return 1;
        case INT:
            return 4;
        case PTR:
            return 8;
        case ARRAY:
            return size_of(ty->ptr_to) * ty->arr_size;
    }
}

type *get_type(){
    if(cur->kind != TK_TYPE){
        error(cur, "unexpected token");
    }
    type *ty = calloc(1, sizeof(type));
    if(expect("char")){
        ty->kind = CHAR;
    }else if(expect("int")){
        ty->kind = INT;
    }
    while(expect("*")){
        ty = type_ptr(ty);
    }
    return ty;
}

int get_number(){
    if(cur->kind != TK_NUM){
        error(cur, "unexpected token");
    }
    int val = cur->val;
    cur = cur->next;
    return val;
}

bool is_eof(){
    return cur->kind == TK_EOF;
}

void *add_global(type *ty, token *id){
    symb *var = calloc(1, sizeof(symb));
    var->next = global_head;
    var->ty = ty;
    var->name = id->str;
    var->len = id->len;

    global_head = var;
}

void *add_local(type *ty, token *tk){
    symb *var = calloc(1, sizeof(symb));
    var->next = local_head;
    var->ty = ty;
    var->name = tk->str;
    var->len = tk->len;
    
    switch(ty->kind){
        case INT:
            var->offset = local_head->offset + 8;
            break;
        case PTR:
            var->offset = local_head->offset + 8;
            break;
        case ARRAY:
            var->offset = local_head->offset + 8 * ty->arr_size;
            break;
    }

    local_head = var;
}

bool match_type(type *t1, type *t2){
    if(t1->kind != t2->kind){
        return false;
    }
    switch(t1->kind){
        case PTR:
        case ARRAY:
            return match_type(t1->ptr_to, t2->ptr_to);
        case CHAR:
        case INT:
            return true;
    }
}

node *node_global_def(type *ty, token *id){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_GLOBAL_DEF;
    nd->ty = ty;
    nd->name = id->str;
    nd->len = id->len;
    nd->offset = size_of(ty);
    return nd;
}

node *node_binary(node_kind kind, node *lhs, node *rhs){
    node *nd = calloc(1, sizeof(node));
    nd->kind = kind;
    nd->op1 = lhs;
    nd->op2 = rhs;

    switch(kind){
        case ND_ASSIGN:
            if(lhs->ty->kind == PTR && rhs->ty->kind == ARRAY && match_type(lhs->ty->ptr_to, rhs->ty->ptr_to)){
                nd->ty = lhs->ty;
            }
            else{
                nd->ty = rhs->ty;
            }
            break;
        case ND_EQ:
        case ND_NEQ:
        case ND_LT:
        case ND_LEQ:
            if(match_type(lhs->ty, rhs->ty)){
                nd->ty = type_base(INT);
            }else{
                error(cur, "invalid operands to relational operator");
            }
            break;
        case ND_ADD:
            if((lhs->ty->kind == PTR || lhs->ty->kind == ARRAY) && rhs->ty->kind == INT){
                nd->ty = type_ptr(lhs->ty->ptr_to);
                rhs->val *= size_of(lhs->ty->ptr_to);
            }else if((rhs->ty->kind == PTR || rhs->ty->kind == ARRAY) && lhs->ty->kind == INT){
                nd->ty = type_ptr(rhs->ty->ptr_to);
                lhs->val *= size_of(rhs->ty->ptr_to);
            }else if(lhs->ty->kind == INT && rhs->ty->kind == INT){
                nd->ty = lhs->ty;
            }else{
                error(cur, "invalid operands to binary +");
            }
            break;
        case ND_SUB:
            if(lhs->ty->ptr_to && match_type(lhs->ty->ptr_to, rhs->ty)){
                nd->ty = lhs->ty;
                rhs->val *= size_of(rhs->ty);
            }else if(lhs->ty->kind == INT && rhs->ty->kind == INT){
                nd->ty = lhs->ty;
            }else{
                error(cur, "invalid operands to binary -");
            }
            break;
        case ND_MUL:
        case ND_DIV:
            if(lhs->ty->kind == INT && rhs->ty->kind == INT){
                nd->ty = type_base(INT);
            }else{
                error(cur, "invalid operands to binary * or /");
            }
            break;
    }
    return nd;
}

node *node_unary(node_kind kind, node *op){
    node *nd = calloc(1, sizeof(node));
    nd->kind = kind;
    nd->op1 = op;

    switch(kind){
        case ND_ADR:
            nd->ty = type_ptr(op->ty);
            break;
        case ND_DEREF:
            if(op->ty->kind == PTR || op->ty->kind == ARRAY){
                nd->ty = op->ty->ptr_to;
                break;
            }
            error(cur, "invalid type argument of unary '*'");
    }
    return nd;
}

node *node_sizeof(node *op){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_NUM;
    nd->ty = type_base(INT);
    nd->val = size_of(op->ty);
    return nd;
}

node *node_symbol(token *id){
    node *nd = calloc(1, sizeof(node));

    for(symb *var = local_head; var; var = var->next){
        if(var->len == id->len && memcmp(var->name, id->str, var->len) == 0){
            nd->kind = ND_LOCAL;
            nd->ty = var->ty;
            nd->offset = var->offset;
            return nd;
        }
    }
    for(symb *var = global_head; var; var = var->next){
        if(var->len == id->len && memcmp(var->name, id->str, var->len) == 0){
            nd->kind = ND_GLOBAL;
            nd->ty = var->ty;
            nd->name = var->name;
            nd->len = var->len;
            return nd;
        }
    }
    error(id, "'%.*s' is undeclared", id->len, id->str);
}

node *node_num(int val){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_NUM;
    nd->ty = type_base(INT);
    nd->val = val;
    return nd;
}

node *node_string(){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_STRING;
    nd->ty = type_ptr(type_base(CHAR));
    nd->offset = lc_num;

    cur = cur->next;
    lc_num++;

    return nd;
}

node **program(token *token_head){
    cur = token_head;

    prg = calloc(100, sizeof(node*));
    while(!is_eof()){
        type *ty = get_type();
        token *id = cur;
        cur = cur->next;

        // gloval variable
        if(expect(";")){
            add_global(ty, id);
            prg[prg_num] = node_global_def(ty, id);
            prg_num++;
            continue;
        }

        // array
        if(expect("[")){
            int size = get_number();
            if(!expect("]")){
                error(cur, "expect ']'");
            }
            if(!expect(";")){
                error(cur, "expected ';'");
            }
            ty = type_array(ty, size);
            add_global(ty, id);
            prg[prg_num] = node_global_def(ty, id);
            prg_num++;
            continue;
        }

        // function
        if(expect("(")){
            node *nd = calloc(1, sizeof(node));
            func *fn = calloc(1, sizeof(func));
            local_head = calloc(1, sizeof(symb));

            nd->kind = ND_FUNC_DEF;
            nd->fn = fn;
            fn->ty = ty;
            fn->name = id->str;
            fn->len = id->len;
            fn->arg_num = 0;

            while(!expect(")")){
                if(cur->kind == TK_TYPE){
                    ty = get_type();
                    id = cur;
                    cur = cur->next;
                    add_local(ty, id);
                }else{
                    error(cur, "expected type");
                }
                fn->arg_num++;
                if(expect(",")){
                    continue;
                }else{
                    if(expect(")")){
                        break;
                    }else{
                        error(cur, "expected ',' or ')'");
                    }
                }
            }

            if(cur->str[0] == '{'){
                fn->stmt = stmt();
                fn->local_size = local_head->offset;
                prg[prg_num] = nd;
                prg_num++;
                continue;
            }else{
                error(cur, "expected '{'");
            }
        }
    }
    prg[prg_num] = NULL;
    return prg;
}

void dcl(){
    type *ty = get_type();
    token *id = cur;
    cur = cur->next;

    // local variable
    if(expect(";")){
        add_local(ty, id);
        return;
    }

    // array
    if(expect("[")){
        int size = get_number();
        if(!expect("]")){
            error(cur, "expect ']'");
        }
        if(!expect(";")){
            error(cur, "expected ';'");
        }
        ty = type_array(ty, size);
        add_local(ty, id);
    }
}

node *stmt(){
    node *nd = calloc(1, sizeof(node));
    
    if(expect("{")){
        nd->kind = ND_BLOCK;
        nd->head = calloc(1, sizeof(node));
        node *elm = nd->head;
        while(!expect("}")){
            if(cur->kind == TK_TYPE){
                dcl();
            }
            else{
                elm->next = stmt();
                elm = elm->next;
            }
        }
    }
    else if(expect("if")){
        if(!expect("(")) error(cur, "expected '('");

        nd->kind = ND_IF;
        nd->op1 = expr();

        if(!expect(")")) error(cur, "expected ')'");

        nd->op2 = stmt();

        if(expect("else")){
            nd->op3 = stmt();
        }
    }
    else if(expect("while")){
        if(!expect("(")) error(cur, "expected '('");

        nd->kind = ND_WHILE;
        nd->op1 = expr();

        if(!expect(")")) error(cur, "expected ')'");

        nd->op2 = stmt();
    }
    else if(expect("for")){
        if(!expect("(")) error(cur, "expected '('");

        nd->kind = ND_FOR;
        nd->op1 = expr();

        if(!expect(";")) error(cur, "expected ';'");

        nd->op2 = expr();

        if(!expect(";")) error(cur, "expected ';'");

        nd->op3 = expr();

        if(!expect(")")) error(cur, "expected ')'");

        nd->op4 = stmt();
    }
    else if(expect("return")){
        nd->kind = ND_RET;
        nd->op1 = expr();
        if(!expect(";")) error(cur, "expected ';'");
    }
    else{
        nd = expr();
        if(!expect(";")) error(cur, "expected ';'");
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
    if(expect("sizeof")){
        return node_sizeof(unary());
    }
    return primary();
}

node *primary(){
    if(expect("(")){
        node *nd = expr();
        expect(")");
        return nd;
    }
    if(cur->kind == TK_ID){
        token *id = cur;
        cur = cur->next;

        if(expect("[")){
            node *nd = node_symbol(id);
            node *size = expr();
            if(!expect("]")){
                error(cur, "expect ']'");
            }
            return node_unary(ND_DEREF, node_binary(ND_ADD, nd, size));
        }
        if(expect("(")){
            node *nd = calloc(1, sizeof(node));
            func *fn = calloc(1, sizeof(func));
            nd->kind = ND_FUNC_CALL;
            nd->fn = fn;
            fn->name = id->str;
            fn->len = id->len;
            fn->arg_num = 0;

            nd->ty = calloc(1, sizeof(type));
            nd->ty->kind = INT;
            for(int i = 0; i < 100; i++){
                if(prg[i] && prg[i]->kind == ND_FUNC_DEF && memcmp(prg[i]->fn->name, fn->name, fn->len) == 0){
                    nd->ty = prg[i]->fn->ty;
                    break;
                }
            }

            node *arg;
            while(!expect(")")){
                arg = expr();
                arg->next = fn->args_head;
                fn->args_head = arg;
                fn->arg_num++;
                if(expect(",")){
                    continue;
                }else{
                    if(expect(")")){
                        break;
                    }else{
                        error(cur, "expected ',' or ')'");
                    }
                }
            }
            return nd;
        }else{
            return node_symbol(id);
        }
    }
    if(cur->kind == TK_NUM){
        return node_num(get_number());
    }
    if(cur->kind == TK_STRING){
        node *nd = calloc(1, sizeof(node));
        nd->kind = ND_LOCAL_CONST;
        nd->name = cur->str;
        nd->len = cur->len;
        nd->offset = lc_num;
        prg[prg_num] = nd;
        prg_num++;
        return node_string();
    }
    error(cur, "unexpected token");
}