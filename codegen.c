#include <stdio.h>

#include "dcl.h"

int label_num = 2;
char *arg_reg_int[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *int_arg_reg[6][4] = {
    {"dil",  "di", "edi", "rdi"},
    {"sil",  "si", "esi", "rsi"},
    { "dl",  "dx", "edx", "rdx"},
    { "cl",  "cx", "ecx", "rcx"},
    {"r8b", "r8w", "r8d", "r8" },
    {"r9b", "r9w", "r9d", "r9" }
};

char *rax[4] = {"al", "ax", "eax", "rax"};
char *rbx[4] = {"bl", "bx", "ebx", "rbx"};
char *rdi[4] = {"dil",  "di", "edi", "rdi"};

char *int_arg_register(int ord, int size){
    switch(size){
        case 1:
            return int_arg_reg[ord][0];
        case 2:
            return int_arg_reg[ord][1];
        case 4:
            return int_arg_reg[ord][2];
        case 8:
            return int_arg_reg[ord][3];
    }
}

void mov_register_to_memory(char *dest, char *src[4], type *ty){
    switch(ty->kind){
        case CHAR:
            printf("    mov BYTE PTR [%s], %s\n", dest, src[0]);
            break;
        case INT:
            printf("    mov DWORD PTR [%s], %s\n", dest, src[2]);
            break;
        case PTR:
            printf("    mov QWORD PTR [%s], %s\n", dest, src[3]);
            break;
    }
}

void mov_memory_to_register(char *dest[4], char *src, type *ty){
    switch(ty->kind){
        case CHAR:
            printf("    mov %s, BYTE PTR [%s]\n", dest[0], src);
            break;
        case INT:
            printf("    mov %s, DWORD PTR [%s]\n", dest[2], src);
            break;
        case PTR:
            printf("    mov %s, QWORD PTR [%s]\n", dest[3], src);
            break;
    }
}

void gen_ext();
void gen_stmt();
void gen_lval();
void gen_expr();

void gen_code(node *node_head){
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    for(node *nd = node_head; nd; nd = nd->next){
        gen_ext(nd);
    }    
}

void gen_ext(node *nd){
    switch(nd->kind){
        case ND_FUNC_DEF:{
            printf(".text\n");
            printf("%.*s:\n", nd->len, nd->name);
            
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");
            printf("    sub rsp, %d\n", (nd->offset + 15)/16 * 16); // region of local variables

            // move arguments from registers or the stack on the rbp to the stack under rbp.
            int num_param_int = 0;
            for(node *param = nd->head->next; param; param = param->next){
                if(num_param_int < 6){
                    printf("    mov rax, rbp\n");
                    printf("    sub rax, %d\n", param->offset);
                    mov_register_to_memory("rax", int_arg_reg[num_param_int], param->ty);
                }else{
                    printf("    mov rax, rbp\n");
                    printf("    sub rax, %d\n", param->offset);
                    printf("    mov rbx, rbp\n");
                    printf("    add rbx, %d\n", 8 * (num_param_int - 4));
                    printf("    mov rbx, [rbx]\n");
                    mov_register_to_memory("rax", rbx, param->ty);
                }
                num_param_int++;
            }

            gen_stmt(nd->op1);

            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");

            break;
        }
        case ND_GLOBAL_DEF:
            printf(".data\n");
            printf("%.*s:\n", nd->len, nd->name);
            printf("    .zero %d\n", nd->offset);
            break;
        case ND_LOCAL_CONST:
            printf(".data\n");
            printf(".LC%d:\n", nd->offset);
            printf("    .string %.*s\n", nd->len, nd->name);
            break;
    }
}

void gen_stmt(node *nd){
    switch(nd->kind){
        int l1, l2;
        case ND_IF:
            gen_expr(nd->op1);
            
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

            gen_expr(nd->op1);
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

            gen_expr(nd->op1);

            printf(".L%d:\n", l1);

            gen_expr(nd->op2);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", l2);
            gen_stmt(nd->op4);
            gen_expr(nd->op3);
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
            gen_expr(nd->op1);

            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
    }
    gen_expr();
}

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

void gen_expr(node *nd){
    switch(nd->kind){
        // unary operator
        case ND_ASSIGN:
            gen_lval(nd->op1);
            gen_expr(nd->op2);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            mov_register_to_memory("rax", rdi, nd->op1->ty);
            printf("    push rdi\n");
            return;
        case ND_ADR:
            gen_lval(nd->op1);
            return;
        case ND_DEREF:
            gen_expr(nd->op1);
            printf("    pop rax\n");
            mov_memory_to_register(rax, "rax", nd->op1->ty->ptr_to);
            printf("    push rax\n");
            return;
        
        // primary
        case ND_FUNC_CALL:{
            int num_stack_var;
            if(nd->val > 6) num_stack_var = nd->val - 6;
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

            // move arguments to the stack
            node *cur = nd->head;
            while(cur){
                gen_expr(cur);
                cur = cur->next;
            }

            // move first 6 arguments to registers
            for(int i = 0; i < (nd->val < 6 ? nd->val : 6); i++){
                printf("    pop %s\n", int_arg_reg[i][3]);
            }

            // variable arguments
            printf("    mov al, 0\n");

            printf("    call %.*s\n", nd->len, nd->name);
            for(int i = 0; i < num_stack_var; i++){
                printf("    pop rdi\n");
            }

            // adjust stack alignment
            printf("    mov rsp, [rsp]\n");

            printf("    push rax\n");
            return;
        }
        case ND_GLOBAL:{
            printf("    lea rax, %.*s[rip]\n", nd->len, nd->name);
            switch(nd->ty->kind){
                case CHAR:
                    printf("    movzb eax, BYTE PTR [rax]\n");
                    break;
                case INT:
                    printf("    mov eax, DWORD PTR [rax]\n");
                    break;
                case PTR:
                    printf("    mov rax, [rax]\n");
                    break;
            }
            printf("    push rax\n");
            return;
        }
        case ND_LOCAL:
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n", nd->offset);
            switch(nd->ty->kind){
                case CHAR:
                    printf("    movzb eax, BYTE PTR [rax]\n");
                    break;
                case INT:
                    printf("    mov eax, DWORD PTR [rax]\n");
                    break;
                case PTR:
                    printf("    mov rax, [rax]\n");
                    break;
            }
            printf("    push rax\n");
            return;
        case ND_NUM:
            printf("    push %d\n", nd->val);
            return;
        case ND_STRING:
            printf("    lea rax, .LC%d[rip]\n", nd->offset);
            printf("    push rax\n");
            return;
    }

    // binary operator
    gen_stmt(nd->op1);
    gen_stmt(nd->op2);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    
    switch(nd->kind){
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