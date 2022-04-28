typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} token_kind;
typedef struct token token;
typedef enum {
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
typedef struct node node;

struct token {
    token_kind kind;
    token *next;
    int val; // kind == TK_NUM
    char *str;
    int len;
};
struct node {
    node_kind kind;
    node *lhs;
    node *rhs;
    int val;
};

extern token *tokenize(char *p);
extern node *expr();
extern void calc(node *nd);
extern char *code_head;
extern token *tk;