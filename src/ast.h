#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>

typedef enum {
    AST_PROGRAM,
    AST_NUMBER,
    AST_IDENTIFIER,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_ASSIGNMENT,
    AST_VAR_DEC,
    AST_BLOCK,
    AST_IF,
    AST_FUNC_DEC,
    AST_FUNC_CALL,
    AST_STENCIL,
    AST_APPLY,
    AST_PAINT,
    AST_RETURN,
    AST_COORDINATE,
    AST_STATEMENT_LIST,
    AST_EXPRESSION_LIST,
    AST_PARAMETER_LIST,
    AST_DIRECTIVE_LIST,
    AST_LOCATION_DIRECTIVE,
    AST_SIZE_DIRECTIVE
} NodeType;

typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIVIDE,
    OP_EQUALS,
    OP_GREATER,
    OP_LESS,
    OP_AND,
    OP_OR,
    OP_NOT
} OpType;

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            int value;
        } number;
        
        struct {
            char* name;
        } identifier;
        
        struct {
            OpType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;
        
        struct {
            OpType op;
            struct ASTNode* operand;
        } unary_op;
        
        struct {
            char* name;
            struct ASTNode* value;
        } assignment;
        
        struct {
            char* name;
            struct ASTNode* value;
        } var_dec;
        
        struct {
            struct ASTNode* statements;
        } block;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_stmt;
            struct ASTNode* else_stmt;
        } if_stmt;
        
        struct {
            char* name;
            struct ASTNode* params;
            struct ASTNode* body;
        } func_dec;
        
        struct {
            char* name;
            struct ASTNode* args;
        } func_call;
        
        struct {
            char* name;
            struct ASTNode* body;
        } stencil;
        
        struct {
            char* name;
            struct ASTNode* directives;
        } apply;
        
        struct {
            struct ASTNode* value;
        } paint;
        
        struct {
            struct ASTNode* value;
        } return_stmt;
        
        struct {
            struct ASTNode* x;
            struct ASTNode* y;
        } coordinate;
        
        struct {
            struct ASTNode* head;
            struct ASTNode* tail;
        } list;
        
        struct {
            struct ASTNode* coordinate;
        } location_directive;
        
        struct {
            struct ASTNode* size;
        } size_directive;
    } data;
} ASTNode;

ASTNode* create_node(NodeType type);
ASTNode* create_number(int value);
ASTNode* create_identifier(char* name);
ASTNode* create_binary_op(OpType op, ASTNode* left, ASTNode* right);
ASTNode* create_unary_op(OpType op, ASTNode* operand);
ASTNode* create_assignment(char* name, ASTNode* value);
ASTNode* create_var_dec(char* name, ASTNode* value);
ASTNode* create_block(ASTNode* statements);
ASTNode* create_if(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt);
ASTNode* create_func_dec(char* name, ASTNode* params, ASTNode* body);
ASTNode* create_func_call(char* name, ASTNode* args);
ASTNode* create_stencil(char* name, ASTNode* body);
ASTNode* create_apply(char* name, ASTNode* directives);
ASTNode* create_paint(ASTNode* value);
ASTNode* create_return(ASTNode* value);
ASTNode* create_coordinate(ASTNode* x, ASTNode* y);
ASTNode* create_list(ASTNode* head, ASTNode* tail);
ASTNode* create_location_directive(ASTNode* coordinate);
ASTNode* create_size_directive(ASTNode* size);

void print_ast(ASTNode* node, int indent);
void free_ast(ASTNode* node);

#endif