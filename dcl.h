typedef enum {
    TK_KEYWORD,
    TK_ID,
    TK_NUM,
    TK_PUNCT,
    TK_EOF,
} token_kind;

typedef enum {
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_RET,

    ND_ASSIGN,
    ND_LOCAL,
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
    node **elms; // ND_IF, ND_WHILE, ND_FOR
    node *head; // ND_BLOCK
    node *next; // ND_BLOCK
    node *lhs; // operator
    node *rhs; // operator
    int offset; // ND_LOCAL
    int val; // ND_NUM
};

extern void error(token *token, char *fmt, ...);
extern token *tokenize(char *p);
extern node **program();
extern void gen(node *nd);
extern local *local_head;