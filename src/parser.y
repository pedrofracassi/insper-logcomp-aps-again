%{
#include <stdio.h>
#include <stdlib.h>
void yyerror(const char *s);
extern int yylex();
%}

%union {
    int number;
    char* identifier;
}

%define parse.error verbose

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

%left OR
%left AND
%left EQUALS GREATER LESS
%left PLUS MINUS
%left TIMES DIVIDE
%right NOT

%%

program
    : statement_list
    ;

statement_list
    : statement
    | statement_list statement
    ;

statement
    : assignment SEMICOLON
    | func_call SEMICOLON
    | block
    | if_statement
    | func_declaration
    | stencil_declaration
    | apply_statement SEMICOLON
    | paint_statement SEMICOLON
    | var_dec SEMICOLON
    | return_statement SEMICOLON
    ;

return_statement
    : RETURN expression
    ; 

expression
    : term
    | expression PLUS term
    | expression MINUS term
    | expression OR term
    ;

rel_exp
    : expression
    | rel_exp EQUALS expression
    | rel_exp GREATER expression
    | rel_exp LESS expression
    ;

term
    : factor
    | term TIMES factor
    | term DIVIDE factor
    | term AND factor
    ;

factor
    : NUMBER
    | PLUS factor
    | MINUS factor
    | NOT factor
    | LPAREN rel_exp RPAREN
    | IDENTIFIER
    | func_call
    ;

assignment
    : IDENTIFIER ASSIGN rel_exp
    ;

var_dec
    : VAR IDENTIFIER ASSIGN expression
    | VAR IDENTIFIER ASSIGN expression COMMA var_dec
    | VAR IDENTIFIER
    | VAR IDENTIFIER COMMA var_dec
    ;

block
    : LBRACE statement_list RBRACE
    ;

if_statement
    : IF LPAREN rel_exp RPAREN statement
    | IF LPAREN rel_exp RPAREN statement ELSE statement
    ;

func_declaration
    : FUNC IDENTIFIER LPAREN RPAREN block
    | FUNC IDENTIFIER LPAREN var_dec RPAREN block
    ;

func_call
    : IDENTIFIER LPAREN RPAREN
    | IDENTIFIER LPAREN expression_list RPAREN
    ;

expression_list
    : expression
    | expression_list COMMA expression
    ;

stencil_declaration
    : STENCIL IDENTIFIER block
    ;

apply_statement
    : APPLY IDENTIFIER directive_list
    ;

directive_list
    : /* empty */
    | directive_list location_directive
    | directive_list size_directive
    ;

coordinate
    : LBRACKET factor COMMA factor RBRACKET
    ;

location_directive
    : AT coordinate
    ;

size_directive
    : SIZE factor
    ;

paint_statement
    : PAINT factor
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    yyparse();
    return 0;
} 