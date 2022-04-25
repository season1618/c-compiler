#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} token_kind;

typedef struct token token;

struct token {
    token_kind kind;
    token *next;
    int val; // kind == TK_NUM
    char *str;
};

token *next_token(token_kind kind, token *cur, char *p){
    token *nxt = calloc(1, sizeof(token));
    cur->next = nxt;
    nxt->kind = kind;
    nxt->str = p;
    return nxt;
}

char *code_head;

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

token *tokenize(char *p){
    token *head = calloc(1, sizeof(token));
    head->next = NULL;
    token *cur = head;

    while(*p){
        if(isspace(*p)){
            p++;
            continue;
        }
        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'){
            cur = next_token(TK_RESERVED, cur, p);
            p++;
            continue;
        }
        if(isdigit(*p)){
            cur = next_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        
        error(cur, "unexpected character");
    }
    next_token(TK_EOF, cur, p);
    return head->next;
}

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} node_kind;

typedef struct node node;

struct node {
    node_kind kind;
    node *lhs;
    node *rhs;
    int val;
};

node *new_node(node_kind kind, node *lhs, node *rhs){
    node *nd = (node*)calloc(1, sizeof(node));
    nd->kind = kind;
    nd->lhs = lhs;
    nd->rhs = rhs;
    return nd;
}

node *new_node_num(int val){
    node *nd = (node*)calloc(1, sizeof(node));
    nd->kind = ND_NUM;
    nd->val = val;
    return nd;
}

token *tk;

node *expr();
node *mul();
node *primary();
int get_number();
bool expect(char);
bool is_eof();

node *expr(){
    node *nd = mul();
    while(true){
        if(expect('+')){
            nd = new_node(ND_ADD, nd, mul());
            continue;
        }
        if(expect('-')){
            nd = new_node(ND_SUB, nd, mul());
            continue;
        }
        return nd;
    }
    return nd;
}

node *mul(){
    node *nd = primary();//printf("%d\n",nd->val);
    while(true){
        if(expect('*')){
            nd = new_node(ND_MUL, nd, primary());
            continue;
        }
        if(expect('/')){
            nd = new_node(ND_DIV, nd, primary());
            continue;
        }
        return nd;
    }
    return nd;
}

node *primary(){
    if(expect('(')){
        node *nd = expr();
        expect(')');
        return nd;
    }
    node *nd = new_node_num(get_number());
    // printf("%d\n",nd->val);
    return nd;
}

int get_number(){
    if(tk->kind != TK_NUM){
        error(tk, "unexpected token");
    }
    int val = tk->val;
    tk = tk->next;
    return val;
}

bool expect(char op){
    if(tk->kind == TK_RESERVED && tk->str[0] == op){
        tk = tk->next;
        return true;
    }else{
        return false;
    }
}

bool is_eof(){
    return tk->kind == TK_EOF;
}

void calc(node *nd){
    if(nd->kind == ND_NUM){
        printf("    push %d\n", nd->val);
        return;
    }

    calc(nd->lhs);
    calc(nd->rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    
    switch(nd->kind){
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
    }
    printf("    push rax\n");
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "incorrect number of arguments");
        return 1;
    }

    code_head = argv[1];
    tk = tokenize(argv[1]);
    node *nd = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    calc(nd);
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}