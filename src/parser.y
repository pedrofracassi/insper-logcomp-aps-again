%{
#include <stdio.h>
#include <stdlib.h>
#include "src/ast.h"
#include "src/codegen.h"

void yyerror(const char *s);
extern int yylex();

ASTNode* root = NULL;
%}

%union {
    int number;
    char* identifier;
    ASTNode* node;
}

%start program

%token <number> NUMBER
%token <identifier> IDENTIFIER
%token PLUS MINUS TIMES DIVIDE
%token EQUALS GREATER LESS
%token AND OR NOT
%token ASSIGN
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token COMMA SEMICOLON
%token IF ELSE FUNC RETURN
%token STENCIL APPLY AT SIZE PAINT
%token VAR

%type <node> program statement_list statement expression rel_exp term factor
%type <node> assignment var_dec block if_statement func_declaration func_call
%type <node> expression_list stencil_declaration apply_statement directive_list
%type <node> coordinate location_directive size_directive paint_statement
%type <node> return_statement

%left OR
%left AND
%left EQUALS GREATER LESS
%left PLUS MINUS
%left TIMES DIVIDE
%right NOT

%%

program
    : statement_list { root = $1; }
    ;

statement_list
    : statement { $$ = $1; }
    | statement_list statement { $$ = create_list($1, $2); }
    ;

statement
    : assignment SEMICOLON { $$ = $1; }
    | func_call SEMICOLON { $$ = $1; }
    | block { $$ = $1; }
    | if_statement { $$ = $1; }
    | func_declaration { $$ = $1; }
    | stencil_declaration { $$ = $1; }
    | apply_statement SEMICOLON { $$ = $1; }
    | paint_statement SEMICOLON { $$ = $1; }
    | var_dec SEMICOLON { $$ = $1; }
    | return_statement SEMICOLON { $$ = $1; }
    ;

return_statement
    : RETURN expression { $$ = create_return($2); }
    ; 

expression
    : term { $$ = $1; }
    | expression PLUS term { $$ = create_binary_op(OP_PLUS, $1, $3); }
    | expression MINUS term { $$ = create_binary_op(OP_MINUS, $1, $3); }
    | expression OR term { $$ = create_binary_op(OP_OR, $1, $3); }
    ;

rel_exp
    : expression { $$ = $1; }
    | rel_exp EQUALS expression { $$ = create_binary_op(OP_EQUALS, $1, $3); }
    | rel_exp GREATER expression { $$ = create_binary_op(OP_GREATER, $1, $3); }
    | rel_exp LESS expression { $$ = create_binary_op(OP_LESS, $1, $3); }
    ;

term
    : factor { $$ = $1; }
    | term TIMES factor { $$ = create_binary_op(OP_TIMES, $1, $3); }
    | term DIVIDE factor { $$ = create_binary_op(OP_DIVIDE, $1, $3); }
    | term AND factor { $$ = create_binary_op(OP_AND, $1, $3); }
    ;

factor
    : NUMBER { $$ = create_number($1); }
    | PLUS factor { $$ = create_unary_op(OP_PLUS, $2); }
    | MINUS factor { $$ = create_unary_op(OP_MINUS, $2); }
    | NOT factor { $$ = create_unary_op(OP_NOT, $2); }
    | LPAREN rel_exp RPAREN { $$ = $2; }
    | IDENTIFIER { $$ = create_identifier($1); }
    | func_call { $$ = $1; }
    ;

assignment
    : IDENTIFIER ASSIGN rel_exp { $$ = create_assignment($1, $3); }
    ;

var_dec
    : VAR IDENTIFIER ASSIGN expression { $$ = create_var_dec($2, $4); }
    | VAR IDENTIFIER ASSIGN expression COMMA var_dec { $$ = create_list(create_var_dec($2, $4), $6); }
    | VAR IDENTIFIER { $$ = create_var_dec($2, NULL); }
    | VAR IDENTIFIER COMMA var_dec { $$ = create_list(create_var_dec($2, NULL), $4); }
    ;

block
    : LBRACE statement_list RBRACE { $$ = create_block($2); }
    ;

if_statement
    : IF LPAREN rel_exp RPAREN statement { $$ = create_if($3, $5, NULL); }
    | IF LPAREN rel_exp RPAREN statement ELSE statement { $$ = create_if($3, $5, $7); }
    ;

func_declaration
    : FUNC IDENTIFIER LPAREN RPAREN block { $$ = create_func_dec($2, NULL, $5); }
    | FUNC IDENTIFIER LPAREN var_dec RPAREN block { $$ = create_func_dec($2, $4, $6); }
    ;

func_call
    : IDENTIFIER LPAREN RPAREN { $$ = create_func_call($1, NULL); }
    | IDENTIFIER LPAREN expression_list RPAREN { $$ = create_func_call($1, $3); }
    ;

expression_list
    : expression { $$ = $1; }
    | expression_list COMMA expression { $$ = create_list($1, $3); }
    ;

stencil_declaration
    : STENCIL IDENTIFIER block { $$ = create_stencil($2, $3); }
    ;

apply_statement
    : APPLY IDENTIFIER directive_list { $$ = create_apply($2, $3); }
    ;

directive_list
    : /* empty */ { $$ = NULL; }
    | directive_list location_directive { $$ = create_list($1, $2); }
    | directive_list size_directive { $$ = create_list($1, $2); }
    ;

coordinate
    : LBRACKET factor COMMA factor RBRACKET { $$ = create_coordinate($2, $4); }
    ;

location_directive
    : AT coordinate { $$ = create_location_directive($2); }
    ;

size_directive
    : SIZE factor { $$ = create_size_directive($2); }
    ;

paint_statement
    : PAINT factor { $$ = create_paint($2); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(int argc, char** argv) {
    int result = yyparse();
    if (result == 0 && root) {
        // Generate LLVM code
        CodeGenContext* ctx = create_codegen_context(stdout);
        SymbolTable* table = create_symbol_table();
        
        generate_code(root, ctx, table);
        
        free_codegen_context(ctx);
        free_symbol_table(table);
        free_ast(root);
    }
    return result;
} 