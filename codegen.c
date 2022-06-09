#include <stdio.h>

#include "dcl.h"

int label_num = 2;
char *arg_reg_int[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_stmt(node *nd);

void gen_lval(node *nd){
    switch(nd->kind){
        case ND_GLOBAL:
            printf("    lea rax, %.*s[rip]\n", nd->len, nd->name);
            printf("    push rax\n");
            return;
        case ND_LOCAL:
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n", nd->offset);
            printf("    push rax\n");
            return;
        case ND_DEREF:
            gen_stmt(nd->op1);
            return;
    }
    fprintf(stderr, "it is not lvalue\n");
}

void gen_stmt(node *nd){
    switch(nd->kind){
        int l1, l2;
        case ND_IF:
            gen_stmt(nd->op1);
            
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            
            if(nd->op3){
                l1 = label_num;
                l2 = label_num + 1;
                label_num += 2;

                printf("    je .L%d\n", l1);
                gen_stmt(nd->op2);
                printf("    jmp .L%d\n", l2);
                
                printf(".L%d:\n", l1);
                
                gen_stmt(nd->op3);
                
                printf(".L%d:\n", l2);
            }else{
                l1 = label_num;
                label_num += 1;

                printf("    je .L%d\n", l1);
                gen_stmt(nd->op2);

                printf(".L%d:\n", l1);
            }
            return;
        case ND_WHILE:
            l1 = label_num;
            l2 = label_num + 1;
            label_num += 2;

            printf(".L%d:\n", l1);

            gen_stmt(nd->op1);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", l2);
            gen_stmt(nd->op2);
            printf("    jmp .L%d\n", l1);
            
            printf(".L%d:\n", l2);
            return;
        case ND_FOR:
            l1 = label_num;
            l2 = label_num + 1;
            label_num += 2;

            gen_stmt(nd->op1);

            printf(".L%d:\n", l1);

            gen_stmt(nd->op2);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", l2);
            gen_stmt(nd->op4);
            gen_stmt(nd->op3);
            printf("    jmp .L%d\n", l1);

            printf(".L%d:\n", l2);
            return;
        case ND_BLOCK:
            for(node *cur = nd->head->next; cur; cur = cur->next){
                gen_stmt(cur);
                printf("    pop rax\n");
            }
            return;
        case ND_RET:
            gen_stmt(nd->op1);

            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_ASSIGN:
            gen_lval(nd->op1);
            gen_stmt(nd->op2);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
        case ND_ADR:
            gen_lval(nd->op1);
            return;
        case ND_DEREF:
            gen_stmt(nd->op1);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
        case ND_FUNC_CALL:{
            func *fn = nd->fn;
            int num_stack_var;
            if(fn->arg_num > 6) num_stack_var = fn->arg_num - 6;
            else num_stack_var = 0;

            // adjust stack alignment
            printf("    push rsp\n");
            printf("    push [rsp]\n");
            if(num_stack_var % 2 == 0){
                printf("    add rsp, 8\n");
                printf("    and rsp, -0x10\n");
            }else{
                printf("    and rsp, -0x10\n");
                printf("    add rsp, 8\n");
            }

            // move arguments to a register or the stack
            int i = fn->arg_num - 1;
            node *cur = fn->args_head;
            while(cur){
                gen_stmt(cur);
                if(i < 6) printf("    pop %s\n", arg_reg_int[i]);
                cur = cur->next;
                i--;
            }

            printf("    call %.*s\n", fn->len, fn->name);
            for(int i = 0; i < num_stack_var; i++){
                printf("    pop rdi\n");
            }
            printf("    mov rsp, [rsp]\n");
            printf("    push rax\n");
            return;
        }
        case ND_GLOBAL:{
            printf("    mov rax, %.*s[rip]\n", nd->len, nd->name);
            printf("    push rax\n");
            return;
        }
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

    gen_stmt(nd->op1);
    gen_stmt(nd->op2);
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

void gen_ext(node *nd){
    switch(nd->kind){
        case ND_FUNC_DEF:{
            func *fn = nd->fn;

            printf(".text\n");
            printf("%.*s:\n", fn->len, fn->name);
            
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");
            printf("    sub rsp, %d\n", fn->local_size); // region of local variables

            // move arguments from registers or the stack on the rbp to the stack under rbp.
            for(int i = 0; i < fn->arg_num; i++){
                if(i < 6){
                    printf("    mov rax, rbp\n");
                    printf("    sub rax, %d\n", 8 * (i + 1));
                    printf("    mov [rax], %s\n", arg_reg_int[i]);
                }else{
                    printf("    mov rax, rbp\n");
                    printf("    sub rax, %d\n", 8 * (i + 1));
                    printf("    mov rbx, rbp\n");
                    printf("    add rbx, %d\n", 8 * (i - 4));
                    printf("    mov rbx, [rbx]\n");
                    printf("    mov [rax], rbx\n");
                }
            }

            gen_stmt(fn->stmt);

            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");

            break;
        }
        case ND_GLOBAL_DEF:{
            printf(".data\n");
            printf("%.*s:\n", nd->len, nd->name);
            printf("    .zero %d\n", nd->offset);

            break;
        }
    }
}

void gen_code(node **prg){
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    for(int i = 0; prg[i]; i++){
        gen_ext(prg[i]);
    }    
}