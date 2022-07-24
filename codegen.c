#include <stdio.h>
#include <stdlib.h>

#include "dcl.h"

int label_num = 2;
block *block_top;

void push_block(node_kind kind, int begin, int end){
    block *stmt = calloc(1, sizeof(block));
    stmt->kind = kind;
    stmt->next = block_top;
    stmt->begin = begin;
    stmt->end = end;
    
    block_top = stmt;
}

void pop_block(){
    block_top = block_top->next;
}

char *rax[4] = {"al", "ax", "eax", "rax"};
char *rbx[4] = {"bl", "bx", "ebx", "rbx"};

char *rdi[4] = {"dil",  "di", "edi", "rdi"};
char *rsi[4] = {"sil",  "si", "esi", "rsi"};
char *rdx[4] = { "dl",  "dx", "edx", "rdx"};
char *rcx[4] = { "cl",  "cx", "ecx", "rcx"};
char  *r8[4] = {"r8b", "r8w", "r8d", "r8" };
char  *r9[4] = {"r9b", "r9w", "r9d", "r9" };

char **int_arg_reg[6] = {rdi, rsi, rdx, rcx, r8, r9};

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

void mov_memory_from_register(char *dest[4], char *src[4], type *ty){
    switch(ty->kind){
        case BOOL:
        case CHAR:
            printf("    mov BYTE PTR [%s], %s\n", dest[3], src[0]);
            break;
        case INT:
            printf("    mov DWORD PTR [%s], %s\n", dest[3], src[2]);
            break;
        case PTR:
            printf("    mov QWORD PTR [%s], %s\n", dest[3], src[3]);
            break;
    }
}

void mov_register_from_memory(char *dest[4], char *src[4], type *ty){
    switch(ty->kind){
        case BOOL:
        case CHAR:
            printf("    movsx %s, BYTE PTR [%s]\n", dest[3], src[3]);
            break;
        case INT:
            printf("    movsx %s, DWORD PTR [%s]\n", dest[3], src[3]);
            break;
        case PTR:
            printf("    mov %s, QWORD PTR [%s]\n", dest[3], src[3]);
            break;
    }
}

void gen_ext();
void gen_alloc();
void gen_stmt();
void gen_expr();
void gen_binary();
void gen_lval();
void gen_rval();

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
            for(symb *param = nd->ty->head; param; param = param->next){
                if(num_param_int < 6){
                    printf("    mov rax, rbp\n");
                    printf("    sub rax, %d\n", param->offset);
                    mov_memory_from_register(rax, int_arg_reg[num_param_int], param->ty);
                }else{
                    printf("    mov rax, rbp\n");
                    printf("    sub rax, %d\n", param->offset);
                    printf("    mov rbx, rbp\n");
                    printf("    add rbx, %d\n", 8 * (num_param_int - 4));
                    printf("    mov rbx, [rbx]\n");
                    mov_memory_from_register(rax, rbx, param->ty);
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
            gen_alloc(nd->ty, nd->head);
            break;
        case ND_LOCAL_CONST:
            printf(".data\n");
            printf(".LC%d:\n", nd->offset);
            printf("    .string %.*s\n", nd->len, nd->name);
            break;
    }
}

void gen_alloc(type *ty, node *init){
    if(!init){
        printf("    .zero %d\n", size_of(ty));
        return;
    }
    switch(ty->kind){
        case CHAR:
            if(init->val == 0) printf("    .zero 1\n");
            else printf("    .byte %d\n", init->val);
            break;
        case INT:
            if(init->val == 0) printf("    .zero 4\n");
            else printf("    .long %d\n", init->val);
            break;
        case PTR:
            if(init->kind == ND_ADR) printf("    .quad %.*s\n", init->op1->len, init->op1->name);
            else if(init->kind == ND_STRING) printf("    .quad .LC%d\n", init->offset);
            break;
        case ARRAY:
            if(ty->ptr_to->kind == CHAR && init->kind == ND_STRING){
                printf("    .string %.*s\n", init->len, init->name);
                break;
            }
            int i = 0;
            for(node *item = init->head; item; item = item->next){
                if(i >= ty->size) fprintf(stderr, "excess elements in array initilizer");
                gen_alloc(ty->ptr_to, item);
                i++;
            }
            int rest = size_of(ty->ptr_to) * (ty->size - i);
            if(rest) printf("    .zero %d\n", rest);
            break;
        case STRUCT:{
            int offset = 0;
            symb *memb = ty->head;
            node *item = init->head;
            for(; item; memb = memb->next, item = item->next){
                if(!memb) fprintf(stderr, "excess elements in struct initilizer");
                if(offset < memb->offset) printf("    .zero %d\n", memb->offset - offset);
                gen_alloc(memb->ty, item);
                offset = memb->offset + size_of(memb->ty);
            }
            if(offset < size_of(ty)) printf("    .zero %d\n", size_of(ty) - offset);
            break;
        }
    }
}

void gen_stmt(node *nd){
    switch(nd->kind){
        int l1, l2;
        case ND_BLOCK:
            for(node *cur = nd->head; cur; cur = cur->next){
                gen_stmt(cur);
            }
            return;
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
        case ND_SWITCH:
            nd->offset = label_num;
            int label_begin = label_num;
            int label_default = label_num + nd->val;
            int label_end = label_num + nd->val + 1;
            push_block(ND_SWITCH, -1, label_end);
            label_num += nd->val + 2;

            int i = label_begin + nd->val - 1;
            for(node *con = nd->head; con; con = con->next){
                gen_expr(nd->op1);
                gen_expr(con);
                printf("    pop rdi\n");
                printf("    pop rax\n");
                printf("    cmp rax, rdi\n");
                printf("    je .L%d\n", i);
                i--;
            }
            // printf("    jmp .L%d\n", label_default);
            
            gen_stmt(nd->op2);

            printf(".L%d:\n", label_end);
            return;
        case ND_CASE:
            printf(".L%d:\n", nd->op1->offset + nd->offset);
            gen_stmt(nd->op2);
            return;
        case ND_WHILE:
            l1 = label_num;
            l2 = label_num + 1;
            label_num += 2;
            push_block(ND_WHILE, l1, l2);

            printf(".L%d:\n", l1);

            gen_expr(nd->op1);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", l2);
            gen_stmt(nd->op2);
            printf("    jmp .L%d\n", l1);
            
            printf(".L%d:\n", l2);

            pop_block();
            return;
        case ND_FOR:
            l1 = label_num;
            l2 = label_num + 1;
            label_num += 2;
            push_block(ND_FOR, l1, l2);

            gen_stmt(nd->op1);

            printf(".L%d:\n", l1);

            gen_expr(nd->op2);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", l2);
            gen_stmt(nd->op4);
            gen_expr(nd->op3);
            printf("    pop rax\n");
            printf("    jmp .L%d\n", l1);

            printf(".L%d:\n", l2);

            pop_block();
            return;
        case ND_CONTINUE:
            for(block *blk = block_top; blk; blk = blk->next){
                if(blk->kind == ND_WHILE || blk->kind == ND_FOR){
                    printf("    jmp .L%d\n", blk->begin);
                    return;
                }
            }
            fprintf(stderr, "continue statement not within a loop");
            return;
        case ND_BREAK:
            if(block_top){
                printf("    jmp .L%d\n", block_top->end);
                return;
            }
            fprintf(stderr, "continue statement not within a loop or switch");
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
    printf("    pop rax\n");
}

void gen_expr(node *nd){
    switch(nd->kind){
        // assignment
        case ND_ASSIGN:
            gen_lval(nd->op1);
            gen_expr(nd->op2);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            mov_memory_from_register(rax, rdi, nd->ty);
            printf("    push rdi\n");
            break;
        case ND_COND:;
            int l1 = label_num;
            int l2 = label_num + 1;
            label_num += 2;

            gen_expr(nd->op1);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .L%d\n", l1);
            
            gen_expr(nd->op2);
            printf("    jmp .L%d\n", l2);
            
            printf(".L%d:\n", l1);
            gen_expr(nd->op3);
            
            printf(".L%d:\n", l2);
            break;
            
        // binary operation
        case ND_COMMA:
        case ND_LOG_OR:
        case ND_LOG_AND:
        case ND_EQ:
        case ND_NEQ:
        case ND_LT:
        case ND_LEQ:
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
        case ND_MOD:
            gen_binary(nd);
            break;

        // unary operation
        case ND_NEG:
            gen_expr(nd->op1);
            printf("    pop rax\n");
            printf("    neg rax\n");
            printf("    push rax\n");
            break;
        case ND_LOG_NOT:
            gen_expr(nd->op1);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    sete al\n");
            printf("    movsx rax, al\n");
            printf("    push rax\n");
            break;
        case ND_ADR:
            gen_lval(nd->op1);
            break;
        
        // right value
        case ND_GLOBAL:
        case ND_LOCAL:
        case ND_DEREF:
        case ND_DOT:
        case ND_ARROW:
            gen_rval(nd);
            break;

        // primary
        case ND_NUM:
            printf("    push %d\n", nd->val);
            return;
        case ND_STRING:
            printf("    lea rax, .LC%d[rip]\n", nd->offset);
            printf("    push rax\n");
            break;
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
            for(node *cur = nd->head; cur; cur = cur->next){
                gen_expr(cur);
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
            break;
        }
    }
}

void gen_binary(node *nd){
    gen_expr(nd->op1);
    gen_expr(nd->op2);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    
    switch(nd->kind){
        case ND_COMMA:
            printf("    mov rax, rdi\n");
            break;
        case ND_LOG_OR:
            printf("    cmp rax, 0\n");
            printf("    setne al\n");
            printf("    movsx rax, al\n");
            printf("    cmp rdi, 0\n");
            printf("    setne dil\n");
            printf("    movsx rdi, dil\n");
            printf("    or rax, rdi\n");
            break;
        case ND_LOG_AND:
            printf("    cmp rax, 0\n");
            printf("    setne al\n");
            printf("    movsx rax, al\n");
            printf("    cmp rdi, 0\n");
            printf("    setne dil\n");
            printf("    movsx rdi, dil\n");
            printf("    and rax, rdi\n");
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
        case ND_MOD:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            printf("    mov rax, rdx\n");
            break;
    }
    printf("    push rax\n");
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
            gen_expr(nd->op1);
            return;
        case ND_DOT:
            gen_lval(nd->op1);
            printf("    pop rax\n");
            printf("    add rax, %d\n", nd->offset);
            printf("    push rax\n");
            return;
        case ND_ARROW:
            gen_expr(nd->op1);
            printf("    pop rax\n");
            printf("    add rax, %d\n", nd->offset);
            printf("    push rax\n");
            return;
    }
    fprintf(stderr, "it is not lvalue\n");
}

void gen_rval(node *nd){
    gen_lval(nd);
    printf("    pop rax\n");
    switch(nd->kind){
        case ND_GLOBAL:
        case ND_LOCAL:
            switch(nd->ty->kind){
                case BOOL:
                case CHAR:
                case INT:
                case PTR:
                    mov_register_from_memory(rax, rax, nd->ty);
                    break;
            }
            break;
        case ND_DEREF:
        case ND_DOT:
        case ND_ARROW:
            mov_register_from_memory(rax, rax, nd->ty);
            break;
    }
    printf("    push rax\n");
}