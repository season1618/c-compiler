#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dcl.h"

token *cur;
node *node_head, *node_tail;
symb *tag_head;
symb *global_head;
symb *local_head;
int lc_num = 0;

void ext();
node *dcl();
symb *type_whole();
type *type_head();
symb *type_ident();
node *stmt();
node *expr();
node *assign();
node *cond();
node *log_or();
node *log_and();
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
    }
    return false;
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
    ty->size = size;
    return ty;
}

bool match_type(type *t1, type *t2){
    if(t1->kind != t2->kind){
        return false;
    }
    switch(t1->kind){
        case VOID:
        case CHAR:
        case INT:
            return true;
        case PTR:
        case ARRAY:
            return match_type(t1->ptr_to, t2->ptr_to);
        case FUNC:
            if(!match_type(t1->ptr_to, t2->ptr_to)) return false;
            for(symb *param1 = t1->head, *param2 = t2->head; param1 && param2; param1 = param1->next, param2 = param2->next){
                if(!match_type(param1->ty, param2->ty)) return false;
            }
            return true;
        case STRUCT:
            if(!t1->name || !t2->name) return false;
            return t1->len == t2->len && memcmp(t1->name, t2->name, t1->len) == 0;
    }
}

bool find_type(){
    if(cur->kind == TK_TYPE){
        return true;
    }
    for(symb *var = global_head; var; var = var->next){
        if(var->kind == TYPE && cur->len == var->len && memcmp(cur->str, var->name, var->len) == 0){
            return true;
        }
    }
    return false;
}

void validate_type(type *ty){
    if(ty->kind == VOID){
        error(cur, "variable or field declared void");
    }
    type *nested = ty;
    while(nested){
        if(nested->ptr_to){
            if(nested->kind == ARRAY && nested->ptr_to->kind == VOID){
                error(cur, "declaration as array of voids");
            }
            if(nested->kind == FUNC && nested->ptr_to->kind == ARRAY){
                error(cur, "declaration as function returning an array");
            }
        }
        nested = nested->ptr_to;
    }
}

int size_of(type *ty){
    switch(ty->kind){
        case VOID:
        case CHAR:
        case FUNC:
            return 1;
        case INT:
        case ENUM:
            return 4;
        case PTR:
            return 8;
        case ARRAY:
            return size_of(ty->ptr_to) * ty->size;
        case STRUCT:
            return ty->size;
    }
}

int align_of(type *ty){
    switch(ty->kind){
        case VOID:
        case CHAR:
        case FUNC:
            return 1;
        case INT:
        case ENUM:
            return 4;
        case PTR:
            return 8;
        case ARRAY:
            return size_of(ty->ptr_to);
        case STRUCT:
            return ty->align;
    }
}

void print_type(type *ty, int num){
    if(num == 0) return;
    num--;
    switch(ty->kind){
        case NOHEAD:
            fprintf(stderr, "NOHEAD");
            break;
        case VOID:
            fprintf(stderr, "VOID");
            break;
        case CHAR:
            fprintf(stderr, "CHAR");
            break;
        case INT:
            fprintf(stderr, "INT");
            break;
        case PTR:
            fprintf(stderr, "PTR -> ");
            print_type(ty->ptr_to, num);
            break;
        case FUNC:
            fprintf(stderr, "FUNC -> ");
            print_type(ty->ptr_to, num);
            break;
        case STRUCT:
            fprintf(stderr, "STRUCT { ");
            for(symb *sy = ty->head; sy; sy = sy->next){
                fprintf(stderr, "%.*s ", sy->len, sy->name);
                print_type(sy->ty, num);
                fprintf(stderr, ", ");
            }
            fprintf(stderr, "}");
            break;
    }
}

int get_number(){
    if(cur->kind != TK_NUM){
        error(cur, "expected number");
    }
    int val = cur->val;
    cur = cur->next;
    return val;
}

void push_ext(node *nd){
    node_tail->next = nd;
    node_tail = nd;
}

void push_tag(type *ty){
    symb *sy = calloc(1, sizeof(symb));
    sy->kind = TAG;
    sy->next = tag_head;
    sy->ty = ty;
    sy->name = ty->name;
    sy->len = ty->len;

    tag_head = sy;
}

void push_global(symb_kind kind, symb *sy){
    symb *var = calloc(1, sizeof(symb));
    var->kind = kind;
    var->next = global_head;
    var->ty = sy->ty;
    var->name = sy->name;
    var->len = sy->len;

    global_head = var;
}

void push_local(symb *sy){
    symb *var = calloc(1, sizeof(symb));
    var->next = local_head;
    var->ty = sy->ty;
    var->name = sy->name;
    var->len = sy->len;
    var->offset = local_head->offset + size_of(sy->ty);
    var->offset = (var->offset + align_of(sy->ty) - 1) / align_of(sy->ty) * align_of(sy->ty);

    local_head = var;
}

node *node_global_def(symb *var){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_GLOBAL_DEF;
    nd->ty = var->ty;
    nd->name = var->name;
    nd->len = var->len;
    nd->offset = size_of(var->ty);
    return nd;
}

node *node_binary(node_kind kind, node *lhs, node *rhs){
    node *nd = calloc(1, sizeof(node));
    nd->kind = kind;
    nd->op1 = lhs;
    nd->op2 = rhs;

    switch(kind){
        case ND_COMMA:
            nd->ty = rhs->ty;
            break;
        case ND_ASSIGN:
            if(match_type(lhs->ty, rhs->ty) || lhs->ty->kind == PTR && rhs->ty->kind == ARRAY && match_type(lhs->ty->ptr_to, rhs->ty->ptr_to)){
                nd->ty = lhs->ty;
            }
            else{
                error(cur, "invalid operands to assignment operator");
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
        case ND_MOD:
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
        case ND_NEG:
            if(op->ty->kind == CHAR || op->ty->kind == INT){
                nd->ty = op->ty;
                break;
            }
            error(cur, "invalid type argument of unary '-'");
        case ND_LOG_NOT:
            if(op->ty->kind == CHAR || op->ty->kind == INT || op->ty->kind == PTR || op->ty->kind == ARRAY){
                nd->ty = op->ty;
                break;
            }
            error(cur, "invalid type argument of unary '!'");
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

node *node_var(token *id){
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
            nd->len = var->len;//fprintf(stderr, "%.*s\n", var->len, var->name);
            return nd;
        }
    }
    error(id, "'%.*s' is undeclared", id->len, id->str);
}

node *node_num(type *ty, int val){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_NUM;
    nd->ty = ty;
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

node *node_func_call(node *var){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_FUNC_CALL;
    nd->ty = var->ty->ptr_to;
    nd->name = var->name;
    nd->len = var->len;
    nd->val = 0;

    node *arg;
    while(!expect(")")){
        arg = assign();
        arg->next = nd->head;
        nd->head = arg;
        nd->val++;
        if(expect(",")) continue;
        if(expect(")")) break;
        error(cur, "expected ',' or ')'");
    }
    return nd;
}

node *node_dot(node *var, token *id){
    if(var->ty->kind != STRUCT){
        error(cur, "type of variable is not a structure");
    }
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_DOT;
    nd->op1 = var;
    for(symb *memb = var->ty->head; memb; memb = memb->next){
        if(id->len == memb->len && memcmp(id->str, memb->name, memb->len) == 0){
            nd->ty = memb->ty;
            nd->offset = memb->offset;
            return nd;
        }
    }
    error(id, "this struct has no member named this");
}

node *node_arrow(node *var, token *id){
    if(var->ty->kind != PTR || var->ty->ptr_to->kind != STRUCT){
        error(cur, "type of variable is not a pointer to structure");
    }
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_ARROW;
    nd->op1 = var;
    for(symb *memb = var->ty->ptr_to->head; memb; memb = memb->next){
        if(id->len == memb->len && memcmp(id->str, memb->name, memb->len) == 0){
            nd->ty = memb->ty;
            nd->offset = memb->offset;
            return nd;
        }
    }
    error(id, "'this struct has no member named this");
}

node *program(token *token_head){
    cur = token_head;
    node_head = calloc(1, sizeof(node));
    node_tail = node_head;

    while(cur->kind != TK_EOF){
        ext();
    }
    return node_head->next;
}

void ext(){
    if(expect("typedef")){
        symb *var = type_ident();
        push_global(TYPE, var);

        if(!expect(";")) error(cur, "expected ';'");
        return;
    }
    else{
        type *head = type_head();
        while(true){
            symb *var = type_ident();
            var = type_whole(var, head);

            if(var->ty->kind == FUNC && var->is_func_def){
                push_global(VAR, var);
                node *nd = calloc(1, sizeof(node));
                nd->kind = ND_FUNC_DEF;
                nd->ty = var->ty;
                nd->name = var->name;
                nd->len = var->len;
                
                local_head = calloc(1, sizeof(symb));
                for(symb *param = var->ty->head; param; param = param->next){
                    push_local(param);
                    param->offset = local_head->offset;
                }
                nd->op1 = stmt();
                nd->offset = local_head->offset;
                
                push_ext(nd);
                return;
            }

            if(var->name){
                push_global(VAR, var);
                if(var->ty->kind != FUNC){
                    push_ext(node_global_def(var));
                }
            }
            if(expect(",")) continue;
            if(expect(";")) break;
            error(cur, "expected ';'");
        }
    }
}

node *init(node *lhs){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_BLOCK;
    nd->head = calloc(1, sizeof(node));
    node *item = nd->head;

    if(lhs->ty->kind == ARRAY){
        if(!expect("{")) error(cur, "expect '{'");
        
        int i = 0;
        while(!expect("}")){
            if(i >= lhs->ty->size){
                error(cur, "ecess elements in array initilizer");
            }
            item->next = init(node_unary(ND_DEREF, node_binary(ND_ADD, lhs, node_num(type_base(INT), i))));
            item = item->next;
            
            i++;
            if(expect(",")) continue;
            if(expect("}")) break;
            error(cur, "expect ','");
        }
        nd->head = nd->head->next;
    }else{
        nd->head = node_binary(ND_ASSIGN, lhs, assign());
    }
    return nd;
}

node *dcl(){
    node *nd = calloc(1, sizeof(node));
    nd->kind = ND_BLOCK;
    nd->head = calloc(1, sizeof(node));
    node *item = nd->head;
    // if(expect("typedef")){
    //     symb *var = type_ident();
    //     push_global(TYPE, var);

    //     if(!expect(";")) error(cur, "expected ';'");
    //     return;
    // }
    // else
    {
        type *head = type_head();
        while(true){
            symb *var = type_ident();
            var = type_whole(var, head);

            if(var->ty->kind == FUNC && var->is_func_def){
                error(cur, "a function defined in local scope");
            }

            if(var->name){
                push_local(var);
                if(expect("=")){
                    node *lhs = calloc(1, sizeof(node));
                    lhs->kind = ND_LOCAL;
                    lhs->ty = local_head->ty;
                    lhs->offset = local_head->offset;
                    item->next = init(lhs);
                    item = item->next;
                }
            }

            if(expect(",")) continue;
            if(expect(";")) break;
            error(cur, "expected ';'");
        }
        nd->head = nd->head->next;
        return nd;
    }
}

symb *type_whole(symb *ident, type *base){
    if(ident->ty->kind == NOHEAD){
        ident->ty = base;
        return ident;
    }
    type *ty = ident->ty;
    while(ty->ptr_to){
        switch(ty->ptr_to->kind){
            case NOHEAD:
                ty->ptr_to = base;
                return ident;
            case VOID:
            case CHAR:
            case INT:
                error(cur, "illegal type");
                break;
            case PTR:
            case ARRAY:
            case FUNC:
                ty = ty->ptr_to;
                break;
        }
    }
}

type *type_head(){
    if(expect("void")){
        return type_base(VOID);
    }
    if(expect("char")){
        return type_base(CHAR);
    }
    if(expect("int")){
        return type_base(INT);
    }
    if(expect("struct")){
        type *ty = calloc(1, sizeof(type));
        ty->kind = STRUCT;
        
        if(cur->kind == TK_ID){
            ty->name = cur->str;
            ty->len = cur->len;
            cur = cur->next;
        }
        if(expect("{")){
            ty->head = calloc(1, sizeof(symb));
            symb *item = ty->head;
            int offset = 0;
            int align = 0;
            while(!expect("}")){
                type *head = type_head();
                while(true){
                    symb *var = type_ident();
                    var = type_whole(var, head);
                    var->offset = (offset + align_of(var->ty) - 1) / align_of(var->ty) * align_of(var->ty);
                    item->next = var;
                    item = item->next;
                    offset = var->offset + size_of(var->ty);
                    if(align < align_of(var->ty)) align = align_of(var->ty);

                    if(expect(",")) continue;
                    if(expect(";")) break;
                    error(cur, "expect ';'");
                }
            }
            ty->head = ty->head->next;
            ty->size = (offset + align - 1) / align * align;
            ty->align = align;
        }

        if(ty->name){
            for(symb *tag = tag_head; tag; tag = tag->next){
                if(ty->len == tag->len && memcmp(ty->name, tag->name, tag->len) == 0){
                    if(ty->head){
                        tag->ty->head = ty->head;
                        tag->ty->size = ty->size;
                        tag->ty->align = ty->align;
                    }
                    return tag->ty;
                }
            }
            push_tag(ty);
        }
        return ty;
    }

    for(symb *var = global_head; var; var = var->next){
        if(var->kind == TYPE && cur->len == var->len && memcmp(cur->str, var->name, var->len) == 0){
            cur = cur->next;
            if(var->ty->len){
                for(symb *tag = tag_head; tag; tag = tag->next){
                    if(var->ty->len == tag->len && memcmp(var->ty->name, tag->name, tag->len) == 0){
                        return tag->ty;
                    }
                }
                error(cur, "undefined tag name");
            }
            return var->ty;
        }
    }
    return type_base(NOHEAD);
}

symb *type_ident(){
    type *base = type_head();
    while(expect("*")){
        base = type_ptr(base);
    }

    symb *ident = calloc(1, sizeof(symb));
    if(expect("(")){
        ident = type_ident();
        if(!expect(")")) error(cur, "expect ')'");
    }else{
        ident->ty = type_base(NOHEAD);
        if(cur->kind == TK_ID){
            ident->name = cur->str;
            ident->len = cur->len;
            cur = cur->next;
        }
    }

    while(true){
        if(expect("[")){
            base = type_array(base, get_number());
            if(expect("]")) continue;
            error(cur, "expected ']'");
        }
        if(expect("(")){
            type *ty = calloc(1, sizeof(type));
            ty->kind = FUNC;
            ty->ptr_to = base;
            ty->head = calloc(1, sizeof(symb));
            symb *param = ty->head;

            while(!expect(")")){
                param->next = type_ident();
                param = param->next;
                if(expect(",")) continue;
                if(expect(")")) break;
                error(cur, "expect ',' or ')'");
            }
            ty->head = ty->head->next;
            base = ty;
        }
        break;
    }

    if(cur->str[0] == '{'){
        ident->is_func_def = true;
    }

    return type_whole(ident, base);
}

node *stmt(){
    node *nd = calloc(1, sizeof(node));
    
    if(expect("{")){
        nd->kind = ND_BLOCK;
        nd->head = calloc(1, sizeof(node));
        node *elm = nd->head;
        while(!expect("}")){
            if(find_type()){
                elm->next = dcl();
                elm = elm->next;
            }else{
                elm->next = stmt();
                elm = elm->next;
            }
        }
        nd->head = nd->head->next;
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
    else if(expect("continue")){
        nd->kind = ND_CONTINUE;
        if(!expect(";")) error(cur, "expected ';'");
    }
    else if(expect("break")){
        nd->kind = ND_BREAK;
        if(!expect(";")) error(cur, "expected ';'");
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
    node *nd = assign();
    while(expect(",")){
        nd = node_binary(ND_COMMA, nd, assign());
    }
    return nd;
}

node *assign(){
    node *nd = cond();
    if(expect("=")){
        return node_binary(ND_ASSIGN, nd, assign());
    }
    if(expect("+=")){
        return node_binary(ND_ASSIGN, nd, node_binary(ND_ADD, nd, assign()));
    }
    if(expect("-=")){
        return node_binary(ND_ASSIGN, nd, node_binary(ND_SUB, nd, assign()));
    }
    if(expect("*=")){
        return node_binary(ND_ASSIGN, nd, node_binary(ND_MUL, nd, assign()));
    }
    if(expect("/=")){
        return node_binary(ND_ASSIGN, nd, node_binary(ND_DIV, nd, assign()));
    }
    if(expect("%=")){
        return node_binary(ND_ASSIGN, nd, node_binary(ND_MOD, nd, assign()));
    }
    return nd;
}

node *cond(){
    node *nd = log_or();
    return nd;
}

node *log_or(){
    node *nd = log_and();
    while(expect("||")){
        nd = node_binary(ND_LOG_OR, nd, log_and());
    }
    return nd;
}

node *log_and(){
    node *nd = equal();
    while(expect("&&")){
        nd = node_binary(ND_LOG_AND, nd, equal());
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
        if(expect("%")){
            nd = node_binary(ND_MOD, nd, unary());
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
        return node_unary(ND_NEG, unary());
    }
    if(expect("&")){
        return node_unary(ND_ADR, unary());
    }
    if(expect("!")){
        return node_unary(ND_LOG_NOT, unary());
    }
    if(expect("*")){
        return node_unary(ND_DEREF, unary());
    }
    if(expect("++")){
        node *nd = unary();
        return node_binary(ND_ASSIGN, nd, node_binary(ND_ADD, nd, node_num(type_base(INT), 1)));
    }
    if(expect("--")){
        node *nd = unary();
        return node_binary(ND_ASSIGN, nd, node_binary(ND_SUB, nd, node_num(type_base(INT), 1)));
    }
    if(expect("sizeof")){
        return node_num(type_base(INT), size_of(unary()->ty));
    }
    return primary();
}

node *primary(){
    node *nd;
    if(expect("(")){
        nd = expr();
        if(!expect(")")) error(cur, "expect ')'");
    }
    else if(cur->kind == TK_ID){
        nd = node_var(cur);
        cur = cur->next;
    }
    else if(cur->kind == TK_NUM){
        nd =  node_num(type_base(INT), cur->val);
        cur = cur->next;
    }
    else if(cur->kind == TK_CHAR){
        nd = node_num(type_base(CHAR), cur->str[1]);
        cur = cur->next;
    }
    else if(cur->kind == TK_STRING){
        nd = calloc(1, sizeof(node));
        nd->kind = ND_LOCAL_CONST;
        nd->name = cur->str;
        nd->len = cur->len;
        nd->offset = lc_num;
        push_ext(nd);
        nd = node_string();
    }
    else{
        error(cur, "unexpected token");
    }

    // postfix operation
    while(true){
        if(expect("[")){
            node *size = assign();
            nd = node_unary(ND_DEREF, node_binary(ND_ADD, nd, size));
            if(!expect("]")) error(cur, "expect ']'");
            continue;
        }
        if(expect("(")){
            nd = node_func_call(nd);
            continue;
        }
        if(expect(".")){
            nd = node_dot(nd, cur);
            cur = cur->next;
            continue;
        }
        if(expect("->")){
            nd = node_arrow(nd, cur);
            cur = cur->next;
            continue;
        }
        if(expect("++")){
            node *one1 = node_num(type_base(INT), 1);
            node *one2 = node_num(type_base(INT), 1);
            nd = node_binary(ND_SUB, node_binary(ND_ASSIGN, nd, node_binary(ND_ADD, nd, one1)), one2);
            continue;
        }
        if(expect("--")){
            node *one1 = node_num(type_base(INT), 1);
            node *one2 = node_num(type_base(INT), 1);
            nd = node_binary(ND_ADD, node_binary(ND_ASSIGN, nd, node_binary(ND_SUB, nd, one1)), one2);
            continue;
        }
        break;
    }
    return nd;
}