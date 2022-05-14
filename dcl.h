typedef enum {
    TK_KEYWORD,
    TK_ID,
    TK_NUM,
    TK_PUNCT,
    TK_EOF,
} token_kind;

typedef enum {
    ND_FUNC_DEF,

    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_RET,

    ND_ASSIGN,
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NEQ,
    ND_LT,
    ND_LEQ,
    ND_ADR,
    ND_DEREF,
    ND_FUNC_CALL,
    ND_LOCAL,
    ND_NUM,
} node_kind;

typedef struct token token;
typedef struct func func;
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
    int index;
};

struct func {
    char *name;
    int len;
    node *args_head;
    int arg_num;
    node *stmt;
    int local_num;
};

struct node {
    node_kind kind;
    node **elms; // ND_IF, ND_WHILE, ND_FOR
    node *head; // ND_BLOCK
    node *next; // ND_BLOCK, ND_FUNC, ND_FUNC_DEF
    node *lhs, *rhs; // operator
    func *fn; // ND_FUNC, ND_FUNC_DEF
    int offset; // ND_LOCAL
    int val; // ND_NUM
};

extern void error(token *token, char *fmt, ...);
extern token *tokenize(char *p);
extern node **program(token *token_head);
extern void gen_code(node **prg);