%{
#include <stdio.h>
#include "src/ast.h"
#include "out/parser.tab.h"
%}

%%

[ \t\n]+                    ; /* Skip whitespace */
"+"                         { return PLUS; }
"-"                         { return MINUS; }
"*"                         { return TIMES; }
"/"                         { return DIVIDE; }
"=="                        { return EQUALS; }
">"                         { return GREATER; }
"<"                         { return LESS; }
"&&"                        { return AND; }
"||"                        { return OR; }
"!"                         { return NOT; }
"="                         { return ASSIGN; }
"("                         { return LPAREN; }
")"                         { return RPAREN; }
"{"                         { return LBRACE; }
"}"                         { return RBRACE; }
"["                         { return LBRACKET; }
"]"                         { return RBRACKET; }
","                         { return COMMA; }
";"                         { return SEMICOLON; }
"if"                        { return IF; }
"else"                      { return ELSE; }
"var"                       { return VAR; }
"func"                      { return FUNC; }
"return"                    { return RETURN; }
"stencil"                   { return STENCIL; } 
"apply"                     { return APPLY; }
"at"                        { return AT; }
"size"                      { return SIZE; }
"paint"                     { return PAINT; }
[0-9]+                      { yylval.number = atoi(yytext); return NUMBER; }
[a-zA-Z]+                   { yylval.identifier = strdup(yytext); return IDENTIFIER; }
.                           { printf("Unexpected character: %s\n", yytext); }

%%

int yywrap(void) {
    return 1;
} 