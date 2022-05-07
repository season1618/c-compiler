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

void gen(node *nd){
    switch(nd->kind){
        case ND_IF:
            gen(nd->elms[0]);
            
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            
            if(nd->elms[2]){
                printf("    je .L%d\n", label_num);
                
                gen(nd->elms[1]);
                
                printf("    jmp .L%d\n", label_num + 1);
                
                printf(".L%d:\n", label_num);
                
                gen(nd->elms[2]);
                
                printf(".L%d:\n", label_num + 1);

                label_num += 2;
            }else{
                gen(nd->elms[1]);

                label_num += 1;
            }
            return;
        case ND_WHILE:
            printf(".L%d:\n", label_num);

            gen(nd->elms[0]);
            
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", label_num + 1);
            
            gen(nd->elms[1]);
            
            printf("    jmp .L%d\n", label_num);
            
            printf(".L%d:\n", label_num + 1);

            label_num++;
            return;
        case ND_FOR:
            gen(nd->elms[0]);

            printf(".L%d:\n", label_num);

            gen(nd->elms[1]);

            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", label_num + 1);

            gen(nd->elms[3]);
            gen(nd->elms[2]);

            printf("    jmp .L%d\n", label_num);

            printf(".L%d:\n", label_num + 1);
            
            label_num++;
            return;
        case ND_RET:
            gen(nd->lhs);

            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_ASSIGN:
            gen_lval(nd->lhs);
            gen(nd->rhs);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
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

    gen(nd->lhs);
    gen(nd->rhs);
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