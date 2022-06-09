typedef enum {
    TK_KEYWORD,
    TK_TYPE,
    TK_ID,
    TK_NUM,
    TK_PUNCT,
    TK_EOF,
} token_kind;

typedef enum {
    // definition
    ND_FUNC_DEF,
    ND_GLOBAL_DEF,

    // statement
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_RET,

    // assignment
    ND_ASSIGN,

    // binary operator
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NEQ,
    ND_LT,
    ND_LEQ,

    // unary operator
    ND_ADR,
    ND_DEREF,

    // primary
    ND_FUNC_CALL,
    ND_GLOBAL,
    ND_LOCAL,
    ND_NUM,
} node_kind;

typedef enum {
    PTR,
    ARRAY,
    INT,
} type_kind;

typedef struct token token;
typedef struct func func;
typedef struct symb symb;
typedef struct node node;
typedef struct type type;

struct token {
    token_kind kind;
    token *next;
    int val; // kind == TK_NUM
    char *str;
    int len;
};

struct type {
    type_kind kind;
    type *ptr_to;
    size_t arr_size;
};

struct symb {
    symb *next;
    type *ty;
    char *name;
    int len;
    int offset;
};

struct func {
    type *ty;
    char *name;
    int len;
    node *args_head;
    int arg_num;
    node *stmt;
    int local_size;
};

struct node {
    node_kind kind;
    node *op1, *op2, *op3, *op4; // ND_IF, ND_WHILE, ND_FOR, operator
    node *head; // ND_BLOCK
    node *next; // ND_BLOCK, ND_FUNC, ND_FUNC_DEF
    
    func *fn; // ND_FUNC_DEF, ND_FUNC
    
    type *ty;
    char *name;
    int len;
    int offset; // ND_LOCAL
    int val; // ND_NUM
};

extern void error(token *token, char *fmt, ...);
extern token *tokenize(char *p);
extern node **program(token *token_head);
extern void gen_code(node **prg);