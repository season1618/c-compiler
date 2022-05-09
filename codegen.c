#include <stdio.h>

#include "dcl.h"

int label_num = 2;

void gen_lval(node *nd){
    if(nd->kind != ND_LOCAL){
        fprintf(stderr, "lvalue is not a variable.");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", nd->offset);
    printf("    push rax\n");
}

void gen_stmt(node *nd){
    switch(nd->kind){
        int l1, l2;
        func *fn;
        case ND_IF:
            gen_stmt(nd->elms[0]);
            
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            
            if(nd->elms[2]){
                l1 = label_num;
                l2 = label_num + 1;
                label_num += 2;

                printf("    je .L%d\n", l1);
                gen_stmt(nd->elms[1]);
                printf("    jmp .L%d\n", l2);
                
                printf(".L%d:\n", l1);
                
                gen_stmt(nd->elms[2]);
                
                printf(".L%d:\n", l2);
            }else{
                l1 = label_num;
                label_num += 1;

                printf("    je .L%d\n", l1);
                gen_stmt(nd->elms[1]);

                printf(".L%d:\n", l1);
            }
            return;
        case ND_WHILE:
            l1 = label_num;
            l2 = label_num + 1;
            label_num += 2;

            printf(".L%d:\n", l1);

            gen_stmt(nd->elms[0]);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", l2);
            gen_stmt(nd->elms[1]);
            printf("    jmp .L%d\n", l1);
            
            printf(".L%d:\n", l2);
            return;
        case ND_FOR:
            l1 = label_num;
            l2 = label_num + 1;
            label_num += 2;

            gen_stmt(nd->elms[0]);

            printf(".L%d:\n", l1);

            gen_stmt(nd->elms[1]);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", l2);
            gen_stmt(nd->elms[3]);
            gen_stmt(nd->elms[2]);
            printf("    jmp .L%d\n", l1);

            printf(".L%d:\n", l2);
            return;
        case ND_BLOCK:
            for(node *cur = nd->head; cur; cur = cur->next){
                gen_stmt(cur);
                printf("    pop rax\n");
            }
            return;
        case ND_RET:
            gen_stmt(nd->lhs);

            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_ASSIGN:
            gen_lval(nd->lhs);
            gen_stmt(nd->rhs);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
        case ND_FUNC:
            fn = nd->fn;
            // for(int i = 0; i < fn->num; i++){
            //     switch(i){
            //         case 0:
            //     }
            // }
            printf("    push rsp\n");
            printf("    push [rsp]\n");
            printf("    and rsp, -0x10\n");
            printf("    call %.*s\n", fn->len, fn->name);
            printf("    add rsp, 8\n");
            printf("    mov rsp, [rsp]\n");
            printf("    push rax\n");
            return;
        case ND_LOCAL:
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n", nd->offset);
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
        case ND_NUM:
            printf("    push %d\n", nd->val);
            return;
    }

    gen_stmt(nd->lhs);
    gen_stmt(nd->rhs);
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
        case ND_EQ:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NEQ:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LT:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LEQ:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
    }
    printf("    push rax\n");
}

void gen_code(node **prg){
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    for(int i = 0; prg[i]; i++){
        gen_stmt(prg[i]);
        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}