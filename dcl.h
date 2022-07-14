typedef enum {
    TK_KEYWORD,
    TK_TYPE,
    TK_PUNCT,
    TK_ID,
    TK_NUM,
    TK_CHAR,
    TK_STRING,
    TK_EOF,
} token_kind;

typedef enum {
    NOTYPE,
    VOID,
    CHAR,
    INT,
    PTR,
    ARRAY,
    FUNC,
} type_kind;

typedef enum {
    VAR,
    TYPE,
    STRUCT,
    ENUM,
} symb_kind;

typedef enum {
    // definition
    ND_FUNC_DEF,
    ND_GLOBAL_DEF,
    ND_LOCAL_CONST,

    // statement
    ND_BLOCK,
    ND_IF,
    ND_SWITCH,
    ND_WHILE,
    ND_FOR,
    ND_CONTINUE,
    ND_BREAK,
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
    ND_NEG,
    ND_ADR,
    ND_DEREF,

    // primary
    ND_FUNC_CALL,
    ND_GLOBAL,
    ND_LOCAL,
    ND_NUM,
    ND_CHAR,
    ND_STRING,
} node_kind;

typedef struct token token;
typedef struct type type;
typedef struct symb symb;
typedef struct node node;
typedef struct block block;

struct token {
    token_kind kind;
    token *next;
    char *str;
    int len;
    int val;
};

struct type {
    type_kind kind;
    type *ptr_to;
    symb *param;
    size_t size;
};

struct symb {
    symb_kind kind;
    symb *next;
    type *ty;
    char *name;
    int len;
    int offset;
};

struct node {
    node_kind kind;
    node *op1, *op2, *op3, *op4;
    node *head;
    node *next;
    type *ty;
    char *name;
    int len;
    int offset;
    int val;
};

struct block {
    node_kind kind;
    block *next;
    int begin;
    int end;
};

extern void error();
extern token *tokenize();
extern node *program();
extern void gen_code();