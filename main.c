#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcl.h"

char *code_head;

void error_(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(token *tk, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = tk->str - code_head;
    fprintf(stderr, "%s\n", code_head);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

char *read_file(char *path){
    FILE *fp = fopen(path, "r");
    if(!fp){
        error_("cannot open %s: %s", path, strerror(errno));
    }

    if(fseek(fp, 0, SEEK_END) == -1){
        error_("%s: fseek: %s", path, strerror(errno));
    }
    size_t size = ftell(fp);
    if(fseek(fp, 0, SEEK_SET) == -1){
        error_("%s: fseek: %s", path, strerror(errno));
    }

    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    if(size == 0 || buf[size - 1] != '\n'){
        buf[size] = '\n';
        size++;
    }
    buf[size] = '\0';
    fclose(fp);

    return buf;
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "incorrect number of arguments");
        return 1;
    }

    code_head = read_file(argv[1]);
    token *token_head = tokenize(code_head);
    node *node_head = program(token_head);
    gen_code(node_head);
    return 0;
}