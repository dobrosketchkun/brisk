/*
 * Brisk Language - Abstract Syntax Tree
 */

#ifndef BRISK_AST_H
#define BRISK_AST_H

#include <stdint.h>
#include <stdbool.h>
#include "token.h"

/* Forward declaration */
typedef struct AstNode AstNode;

/* Node types */
typedef enum {
    /* Literals */
    NODE_LITERAL_INT,
    NODE_LITERAL_FLOAT,
    NODE_LITERAL_STRING,
    NODE_LITERAL_BOOL,
    NODE_LITERAL_NIL,
    
    /* Expressions */
    NODE_IDENTIFIER,
    NODE_BINARY,
    NODE_UNARY,
    NODE_CALL,
    NODE_INDEX,
    NODE_FIELD,
    NODE_ARRAY,
    NODE_TABLE,
    NODE_LAMBDA,
    NODE_RANGE,
    NODE_ADDRESS_OF,
    
    /* Statements */
    NODE_VAR_DECL,
    NODE_CONST_DECL,
    NODE_ASSIGNMENT,
    NODE_EXPR_STMT,
    NODE_BLOCK,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_RETURN,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_FN_DECL,
    NODE_MATCH,
    NODE_DEFER,
    
    /* Special */
    NODE_PROGRAM,
    NODE_IMPORT,
    NODE_C_BLOCK,
    NODE_CTYPE,
    
    NODE_TYPE_COUNT
} NodeType;

/* Literal data */
typedef struct {
    int64_t value;
} IntLiteral;

typedef struct {
    double value;
} FloatLiteral;

typedef struct {
    char* value;
    int length;
} StringLiteral;

typedef struct {
    bool value;
} BoolLiteral;

/* Identifier data */
typedef struct {
    char* name;
    int name_length;
} Identifier;

/* Binary expression data */
typedef struct {
    TokenType operator;
    AstNode* left;
    AstNode* right;
} BinaryExpr;

/* Unary expression data */
typedef struct {
    TokenType operator;
    AstNode* operand;
} UnaryExpr;

/* Call expression data */
typedef struct {
    AstNode* callee;
    AstNode** arguments;
    int arg_count;
} CallExpr;

/* Index expression data */
typedef struct {
    AstNode* object;
    AstNode* index;
} IndexExpr;

/* Field access data */
typedef struct {
    AstNode* object;
    char* field_name;
    int field_name_length;
} FieldExpr;

/* Array literal data */
typedef struct {
    AstNode** elements;
    int element_count;
} ArrayLiteral;

/* Table literal data */
typedef struct {
    char** keys;
    int* key_lengths;
    AstNode** values;
    int count;
} TableLiteral;

/* Range expression data */
typedef struct {
    AstNode* start;
    AstNode* end;
} RangeExpr;

/* Variable declaration data */
typedef struct {
    char* name;
    int name_length;
    AstNode* initializer;
    bool is_const;
} VarDecl;

/* Assignment data */
typedef struct {
    AstNode* target;
    AstNode* value;
} Assignment;

/* Block data */
typedef struct {
    AstNode** statements;
    int statement_count;
} Block;

/* If statement data */
typedef struct {
    AstNode* condition;
    AstNode* then_branch;
    AstNode* else_branch;  /* Can be NULL or another if node for elif */
} IfStmt;

/* While statement data */
typedef struct {
    AstNode* condition;
    AstNode* body;
} WhileStmt;

/* For statement data */
typedef struct {
    char* iterator_name;
    int iterator_name_length;
    AstNode* iterable;
    AstNode* body;
} ForStmt;

/* Function declaration data */
typedef struct {
    char* name;
    int name_length;
    char** parameters;
    int* param_lengths;
    int param_count;
    AstNode* body;
} FnDecl;

/* Lambda (anonymous function) data */
typedef struct {
    char** parameters;
    int* param_lengths;
    int param_count;
    AstNode* body;
} Lambda;

/* Return statement data */
typedef struct {
    AstNode* value;  /* Can be NULL */
} ReturnStmt;

/* Match statement data */
typedef struct {
    AstNode* value;
    AstNode** patterns;
    AstNode** bodies;
    int arm_count;
} MatchStmt;

/* Defer statement data */
typedef struct {
    AstNode* statement;
} DeferStmt;

/* Import data */
typedef struct {
    char* path;
    int path_length;
} Import;

/* C block data */
typedef struct {
    char* code;
    int code_length;
} CBlock;

/* Program data */
typedef struct {
    AstNode** statements;
    int statement_count;
} Program;

/* Address-of expression data */
typedef struct {
    AstNode* operand;
} AddressOf;

/* AST Node structure */
struct AstNode {
    NodeType type;
    int line;
    int column;
    
    union {
        IntLiteral int_literal;
        FloatLiteral float_literal;
        StringLiteral string_literal;
        BoolLiteral bool_literal;
        Identifier identifier;
        BinaryExpr binary;
        UnaryExpr unary;
        CallExpr call;
        IndexExpr index;
        FieldExpr field;
        ArrayLiteral array;
        TableLiteral table;
        RangeExpr range;
        VarDecl var_decl;
        Assignment assignment;
        Block block;
        IfStmt if_stmt;
        WhileStmt while_stmt;
        ForStmt for_stmt;
        FnDecl fn_decl;
        Lambda lambda;
        ReturnStmt return_stmt;
        MatchStmt match_stmt;
        DeferStmt defer_stmt;
        Import import;
        CBlock c_block;
        Program program;
        AddressOf address_of;
    } as;
};

/* Node type name for debugging */
const char* node_type_name(NodeType type);

/* Node creation functions */
AstNode* ast_create_node(NodeType type, int line, int column);

/* Specific node constructors */
AstNode* ast_int_literal(int64_t value, int line, int column);
AstNode* ast_float_literal(double value, int line, int column);
AstNode* ast_string_literal(const char* value, int length, int line, int column);
AstNode* ast_bool_literal(bool value, int line, int column);
AstNode* ast_nil_literal(int line, int column);
AstNode* ast_identifier(const char* name, int length, int line, int column);
AstNode* ast_binary(TokenType op, AstNode* left, AstNode* right, int line, int column);
AstNode* ast_unary(TokenType op, AstNode* operand, int line, int column);
AstNode* ast_call(AstNode* callee, AstNode** args, int arg_count, int line, int column);
AstNode* ast_index(AstNode* object, AstNode* index, int line, int column);
AstNode* ast_field(AstNode* object, const char* field, int length, int line, int column);
AstNode* ast_array(AstNode** elements, int count, int line, int column);
AstNode* ast_table(char** keys, int* key_lengths, AstNode** values, int count, int line, int column);
AstNode* ast_range(AstNode* start, AstNode* end, int line, int column);
AstNode* ast_var_decl(const char* name, int length, AstNode* init, bool is_const, int line, int column);
AstNode* ast_assignment(AstNode* target, AstNode* value, int line, int column);
AstNode* ast_block(AstNode** stmts, int count, int line, int column);
AstNode* ast_if(AstNode* cond, AstNode* then_b, AstNode* else_b, int line, int column);
AstNode* ast_while(AstNode* cond, AstNode* body, int line, int column);
AstNode* ast_for(const char* iter, int iter_len, AstNode* iterable, AstNode* body, int line, int column);
AstNode* ast_fn_decl(const char* name, int name_len, char** params, int* param_lens, int param_count, AstNode* body, int line, int column);
AstNode* ast_lambda(char** params, int* param_lens, int param_count, AstNode* body, int line, int column);
AstNode* ast_return(AstNode* value, int line, int column);
AstNode* ast_break(int line, int column);
AstNode* ast_continue(int line, int column);
AstNode* ast_match(AstNode* value, AstNode** patterns, AstNode** bodies, int arm_count, int line, int column);
AstNode* ast_defer(AstNode* stmt, int line, int column);
AstNode* ast_import(const char* path, int length, int line, int column);
AstNode* ast_c_block(const char* code, int length, int line, int column);
AstNode* ast_program(AstNode** stmts, int count);
AstNode* ast_address_of(AstNode* operand, int line, int column);
AstNode* ast_expr_stmt(AstNode* expr, int line, int column);

/* Memory management */
void ast_free_node(AstNode* node);
void ast_free_tree(AstNode* root);

/* Debug printing */
void ast_print(AstNode* node, int indent);

#endif /* BRISK_AST_H */
