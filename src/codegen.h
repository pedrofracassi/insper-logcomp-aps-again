#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

typedef struct {
    FILE* output;
    int label_counter;
    int temp_counter;
    int string_counter;
    char* current_function;
    int in_stencil;
} CodeGenContext;

typedef struct VarEntry {
    char* name;
    char* llvm_name;
    struct VarEntry* next;
} VarEntry;

typedef struct FuncEntry {
    char* name;
    int param_count;
    struct FuncEntry* next;
} FuncEntry;

typedef struct StencilEntry {
    char* name;
    struct StencilEntry* next;
} StencilEntry;

typedef struct {
    VarEntry* vars;
    FuncEntry* funcs;
    StencilEntry* stencils;
} SymbolTable;

CodeGenContext* create_codegen_context(FILE* output);
void free_codegen_context(CodeGenContext* ctx);

SymbolTable* create_symbol_table();
void free_symbol_table(SymbolTable* table);

void add_var(SymbolTable* table, const char* name, const char* llvm_name);
char* lookup_var(SymbolTable* table, const char* name);

void add_func(SymbolTable* table, const char* name, int param_count);
FuncEntry* lookup_func(SymbolTable* table, const char* name);

void add_stencil(SymbolTable* table, const char* name);
StencilEntry* lookup_stencil(SymbolTable* table, const char* name);

char* new_temp(CodeGenContext* ctx);
char* new_label(CodeGenContext* ctx);
char* new_string_const(CodeGenContext* ctx);

void generate_code(ASTNode* ast, CodeGenContext* ctx, SymbolTable* global_table);
char* generate_expression(ASTNode* node, CodeGenContext* ctx, SymbolTable* table);
void generate_statement(ASTNode* node, CodeGenContext* ctx, SymbolTable* table);
void generate_global_decls(ASTNode* node, CodeGenContext* ctx, SymbolTable* table);

void emit_runtime_functions(CodeGenContext* ctx);
void emit_main_function(CodeGenContext* ctx, SymbolTable* table);

#endif