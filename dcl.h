typedef enum {
    TK_RET,
    TK_ID,
    TK_NUM,
    TK_RESERVED,
    TK_EOF,
} token_kind;

typedef enum {
    ND_ASSIGN,
    ND_LOCAL,
    ND_RET,
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NEQ,
    ND_LT,
    ND_LEQ,
    ND_NUM,
} node_kind;

typedef struct token token;
typedef struct local local;
typedef struct node node;

struct token {
    token_kind kind;
    token *next;
    int val; // kind == TK_NUM
    char *str;
    int len;
};

struct local {
    local *next;
    char *name;
    int len;
    int offset;
};

struct node {
    node_kind kind;
    node *lhs;
    node *rhs;
    int val;
    int offset;
};

extern token *tokenize(char *p);
extern void program();
extern void gen(node *nd);
extern char *code_head;
extern token *tk;
extern local *local_head;
extern node *code[100];