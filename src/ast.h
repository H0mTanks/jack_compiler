#pragma once

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;

typedef enum DeclKind {
    DECL_NONE,
    DECL_CLASS,
    DECL_CLASSVAR,
    DECL_SUBROUTINE,
    DECL_VAR,
} DeclKind;

typedef enum TypeKind {
    TYPE_VOID,
    TYPE_INT,
    TYPE_CHAR,
    TYPE_BOOLEAN,
    TYPE_CLASSNAME,
} TypeKind;

typedef struct Type {
    TypeKind kind;
    const char* name;
} Type;

typedef enum ExprKind {
    EXPR_NONE,
    EXPR_INT,
    EXPR_STR,
    EXPR_KEYWORD,
    EXPR_NAME,
    EXPR_INDEX,
    EXPR_CALL,
    EXPR_BINARY,
    EXPR_UNARY,
} ExprKind;

typedef struct ExprList {
    Expr** exprs;
    size_t num_exprs;
} ExprList;

typedef enum CallKind {
    CALL_FUNCTION,
    CALL_METHOD,
} CallKind;

typedef struct SubCall {
    CallKind kind;
    ExprList expr_list;
    const char* field_name;
    const char* sub_name;
} SubCall;

//? Keyword Constant
typedef struct Expr {
    ExprKind kind;
    union {
        i32 int_val;
        const char* str_val;
        const char* name;
        const char* keyword;
        struct {
            Expr* expr;
        } index;
        SubCall call;
        struct {
            TokenKind op;
            Expr* expr;
        } unary;
        struct {
            TokenKind op;
            Expr* left;
            Expr* right;
        } binary;
    };
} Expr;

typedef enum StmtKind {
    STMT_LET,
    STMT_IF,
    STMT_WHILE,
    STMT_DO,
    STMT_RETURN,
} StmtKind;

typedef struct StmtList {
    SrcPos pos;
    Stmt** stmts;
    size_t num_stmts;
} StmtList;

typedef struct Stmt {
    StmtKind kind;
    union {
        struct {
            const char* name;
            Expr* index_expr;
            Expr* assign_expr;
        } let_stmt;
        struct {
            Expr* cond;
            StmtList then_block;
            StmtList else_block;
        } if_stmt;
        struct {
            Expr* cond;
            StmtList block;
        } while_stmt;
        struct {
            Expr* subroutine_call;
        } do_stmt;
        struct {
            Expr* expr;
        } return_stmt;
    };
} Stmt;

typedef enum VarType {
    VAR_NONE,
    VAR_STATIC,
    VAR_FIELD,
} VarType;

typedef struct VarDecl {
    Type* type;
    const char* name;
} VarDecl;

typedef struct ClassVarDecl {
    VarType var_type;
    Type* type;
    const char* name;
} ClassVarDecl;

typedef enum SubroutineType {
    SUB_CONSTRUCTOR,
    SUB_METHOD,
    SUB_FUNCTION,
} SubroutineType;

typedef struct Subroutine {
    SubroutineType sub_type;
    const char* name;
    VarDecl* params;
    size_t num_params;
    Type* ret_type;
    VarDecl* vars;
    size_t num_vars;
    StmtList block;
} Subroutine;

typedef struct ClassDecl {
    const char* name;
    ClassVarDecl* vars;
    size_t num_vars;
    Subroutine* subs;
    size_t num_subs;
} ClassDecl;