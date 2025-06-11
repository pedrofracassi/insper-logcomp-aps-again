#include "codegen.h"
#include <stdlib.h>
#include <string.h>

CodeGenContext* create_codegen_context(FILE* output) {
    CodeGenContext* ctx = (CodeGenContext*)malloc(sizeof(CodeGenContext));
    ctx->output = output;
    ctx->label_counter = 0;
    ctx->temp_counter = 0;
    ctx->string_counter = 0;
    ctx->current_function = NULL;
    ctx->in_stencil = 0;
    return ctx;
}

void free_codegen_context(CodeGenContext* ctx) {
    if (ctx->current_function) {
        free(ctx->current_function);
    }
    free(ctx);
}

SymbolTable* create_symbol_table() {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->vars = NULL;
    table->funcs = NULL;
    table->stencils = NULL;
    return table;
}

void free_symbol_table(SymbolTable* table) {
    VarEntry* var = table->vars;
    while (var) {
        VarEntry* next = var->next;
        free(var->name);
        free(var->llvm_name);
        free(var);
        var = next;
    }
    
    FuncEntry* func = table->funcs;
    while (func) {
        FuncEntry* next = func->next;
        free(func->name);
        free(func);
        func = next;
    }
    
    StencilEntry* stencil = table->stencils;
    while (stencil) {
        StencilEntry* next = stencil->next;
        free(stencil->name);
        free(stencil);
        stencil = next;
    }
    
    free(table);
}

void add_var(SymbolTable* table, const char* name, const char* llvm_name) {
    VarEntry* entry = (VarEntry*)malloc(sizeof(VarEntry));
    entry->name = strdup(name);
    entry->llvm_name = strdup(llvm_name);
    entry->next = table->vars;
    table->vars = entry;
}

char* lookup_var(SymbolTable* table, const char* name) {
    VarEntry* entry = table->vars;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->llvm_name;
        }
        entry = entry->next;
    }
    return NULL;
}

void add_func(SymbolTable* table, const char* name, int param_count) {
    FuncEntry* entry = (FuncEntry*)malloc(sizeof(FuncEntry));
    entry->name = strdup(name);
    entry->param_count = param_count;
    entry->next = table->funcs;
    table->funcs = entry;
}

FuncEntry* lookup_func(SymbolTable* table, const char* name) {
    FuncEntry* entry = table->funcs;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

void add_stencil(SymbolTable* table, const char* name) {
    StencilEntry* entry = (StencilEntry*)malloc(sizeof(StencilEntry));
    entry->name = strdup(name);
    entry->next = table->stencils;
    table->stencils = entry;
}

StencilEntry* lookup_stencil(SymbolTable* table, const char* name) {
    StencilEntry* entry = table->stencils;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

char* new_temp(CodeGenContext* ctx) {
    char* temp = (char*)malloc(32);
    sprintf(temp, "%%tmp%d", ctx->temp_counter++);
    return temp;
}

char* new_label(CodeGenContext* ctx) {
    char* label = (char*)malloc(32);
    sprintf(label, "label%d", ctx->label_counter++);
    return label;
}

char* new_string_const(CodeGenContext* ctx) {
    char* str = (char*)malloc(32);
    sprintf(str, "@.str%d", ctx->string_counter++);
    return str;
}

void emit_runtime_functions(CodeGenContext* ctx) {
    FILE* out = ctx->output;
    
    // External functions for pixel operations
    fprintf(out, "; External functions\n");
    fprintf(out, "declare void @paint_pixel(i32, i32, i32)\n");
    fprintf(out, "declare i32 @get_canvas_width()\n");
    fprintf(out, "declare i32 @get_canvas_height()\n");
    fprintf(out, "\n");
}

char* generate_expression(ASTNode* node, CodeGenContext* ctx, SymbolTable* table) {
    if (!node) return NULL;
    
    FILE* out = ctx->output;
    char* result;
    
    switch (node->type) {
        case AST_NUMBER: {
            result = (char*)malloc(32);
            sprintf(result, "%d", node->data.number.value);
            return result;
        }
        
        case AST_IDENTIFIER: {
            char* var_name = lookup_var(table, node->data.identifier.name);
            if (var_name) {
                if (ctx->in_stencil) {
                    char* temp = new_temp(ctx);
                    fprintf(out, "  %s = load i32, i32* %s\n", temp, var_name);
                    return temp;
                } else {
                    return strdup(var_name);
                }
            } else if (ctx->in_stencil) {
                if (strcmp(node->data.identifier.name, "x") == 0) {
                    char* temp = new_temp(ctx);
                    fprintf(out, "  %s = load i32, i32* %%x\n", temp);
                    return temp;
                } else if (strcmp(node->data.identifier.name, "y") == 0) {
                    char* temp = new_temp(ctx);
                    fprintf(out, "  %s = load i32, i32* %%y\n", temp);
                    return temp;
                }
            }
            result = (char*)malloc(strlen(node->data.identifier.name) + 2);
            sprintf(result, "%%%s", node->data.identifier.name);
            return result;
        }
        
        case AST_BINARY_OP: {
            char* left = generate_expression(node->data.binary_op.left, ctx, table);
            char* right = generate_expression(node->data.binary_op.right, ctx, table);
            char* temp = new_temp(ctx);
            
            switch (node->data.binary_op.op) {
                case OP_PLUS:
                    fprintf(out, "  %s = add i32 %s, %s\n", temp, left, right);
                    break;
                case OP_MINUS:
                    fprintf(out, "  %s = sub i32 %s, %s\n", temp, left, right);
                    break;
                case OP_TIMES:
                    fprintf(out, "  %s = mul i32 %s, %s\n", temp, left, right);
                    break;
                case OP_DIVIDE:
                    fprintf(out, "  %s = sdiv i32 %s, %s\n", temp, left, right);
                    break;
                case OP_LESS: {
                    char* cmp_temp = new_temp(ctx);
                    fprintf(out, "  %s = icmp slt i32 %s, %s\n", cmp_temp, left, right);
                    fprintf(out, "  %s = zext i1 %s to i32\n", temp, cmp_temp);
                    break;
                }
                case OP_GREATER: {
                    char* cmp_temp = new_temp(ctx);
                    fprintf(out, "  %s = icmp sgt i32 %s, %s\n", cmp_temp, left, right);
                    fprintf(out, "  %s = zext i1 %s to i32\n", temp, cmp_temp);
                    break;
                }
                case OP_EQUALS: {
                    char* cmp_temp = new_temp(ctx);
                    fprintf(out, "  %s = icmp eq i32 %s, %s\n", cmp_temp, left, right);
                    fprintf(out, "  %s = zext i1 %s to i32\n", temp, cmp_temp);
                    break;
                }
                case OP_AND: {
                    char* left_bool = new_temp(ctx);
                    char* right_bool = new_temp(ctx);
                    char* and_temp = new_temp(ctx);
                    fprintf(out, "  %s = icmp ne i32 %s, 0\n", left_bool, left);
                    fprintf(out, "  %s = icmp ne i32 %s, 0\n", right_bool, right);
                    fprintf(out, "  %s = and i1 %s, %s\n", and_temp, left_bool, right_bool);
                    fprintf(out, "  %s = zext i1 %s to i32\n", temp, and_temp);
                    break;
                }
                case OP_OR: {
                    char* left_bool = new_temp(ctx);
                    char* right_bool = new_temp(ctx);
                    char* or_temp = new_temp(ctx);
                    fprintf(out, "  %s = icmp ne i32 %s, 0\n", left_bool, left);
                    fprintf(out, "  %s = icmp ne i32 %s, 0\n", right_bool, right);
                    fprintf(out, "  %s = or i1 %s, %s\n", or_temp, left_bool, right_bool);
                    fprintf(out, "  %s = zext i1 %s to i32\n", temp, or_temp);
                    break;
                }
            }
            
            free(left);
            free(right);
            return temp;
        }
        
        case AST_UNARY_OP: {
            char* operand = generate_expression(node->data.unary_op.operand, ctx, table);
            char* temp = new_temp(ctx);
            
            switch (node->data.unary_op.op) {
                case OP_MINUS:
                    fprintf(out, "  %s = sub i32 0, %s\n", temp, operand);
                    break;
                case OP_PLUS:
                    // No-op for unary plus
                    free(temp);
                    return operand;
                case OP_NOT: {
                    char* cmp_temp = new_temp(ctx);
                    fprintf(out, "  %s = icmp eq i32 %s, 0\n", cmp_temp, operand);
                    fprintf(out, "  %s = zext i1 %s to i32\n", temp, cmp_temp);
                    break;
                }
            }
            
            free(operand);
            return temp;
        }
        
        case AST_FUNC_CALL: {
            char* temp = new_temp(ctx);
            
            int arg_count = 0;
            char* args[100];
            ASTNode* arg_list = node->data.func_call.args;
            
            while (arg_list && arg_count < 100) {
                if (arg_list->type == AST_EXPRESSION_LIST) {
                    args[arg_count++] = generate_expression(arg_list->data.list.head, ctx, table);
                    arg_list = arg_list->data.list.tail;
                } else {
                    args[arg_count++] = generate_expression(arg_list, ctx, table);
                    break;
                }
            }
            
            // Generate call
            fprintf(out, "  %s = call i32 @%s(", temp, node->data.func_call.name);
            for (int i = 0; i < arg_count; i++) {
                if (i > 0) fprintf(out, ", ");
                fprintf(out, "i32 %s", args[i]);
                free(args[i]);
            }
            fprintf(out, ")\n");
            
            return temp;
        }
        
        default:
            return NULL;
    }
}

void generate_statement(ASTNode* node, CodeGenContext* ctx, SymbolTable* table) {
    if (!node) return;
    
    FILE* out = ctx->output;
    
    switch (node->type) {
        case AST_STATEMENT_LIST:
            if (node->data.list.head) {
                generate_statement(node->data.list.head, ctx, table);
            }
            if (node->data.list.tail) {
                generate_statement(node->data.list.tail, ctx, table);
            }
            break;
            
        case AST_VAR_DEC: {
            char* var_name = (char*)malloc(strlen(node->data.var_dec.name) + 2);
            sprintf(var_name, "%%%s", node->data.var_dec.name);
            
            fprintf(out, "  %s = alloca i32\n", var_name);
            
            if (node->data.var_dec.value) {
                char* value = generate_expression(node->data.var_dec.value, ctx, table);
                fprintf(out, "  store i32 %s, i32* %s\n", value, var_name);
                free(value);
            }
            
            add_var(table, node->data.var_dec.name, var_name);
            free(var_name);
            break;
        }
        
        case AST_ASSIGNMENT: {
            char* var_name = lookup_var(table, node->data.assignment.name);
            if (var_name) {
                char* value = generate_expression(node->data.assignment.value, ctx, table);
                fprintf(out, "  store i32 %s, i32* %s\n", value, var_name);
                free(value);
            }
            break;
        }
        
        case AST_BLOCK: {
            generate_statement(node->data.block.statements, ctx, table);
            break;
        }
        
        case AST_IF: {
            char* cond = generate_expression(node->data.if_stmt.condition, ctx, table);
            char* cond_bool = new_temp(ctx);
            char* then_label = new_label(ctx);
            char* else_label = new_label(ctx);
            char* end_label = new_label(ctx);
            
            fprintf(out, "  %s = icmp ne i32 %s, 0\n", cond_bool, cond);
            
            if (node->data.if_stmt.else_stmt) {
                fprintf(out, "  br i1 %s, label %%%s, label %%%s\n", cond_bool, then_label, else_label);
            } else {
                fprintf(out, "  br i1 %s, label %%%s, label %%%s\n", cond_bool, then_label, end_label);
            }
            
            fprintf(out, "%s:\n", then_label);
            generate_statement(node->data.if_stmt.then_stmt, ctx, table);
            fprintf(out, "  br label %%%s\n", end_label);
            
            if (node->data.if_stmt.else_stmt) {
                fprintf(out, "%s:\n", else_label);
                generate_statement(node->data.if_stmt.else_stmt, ctx, table);
                fprintf(out, "  br label %%%s\n", end_label);
            }
            
            fprintf(out, "%s:\n", end_label);
            
            free(cond);
            free(cond_bool);
            free(then_label);
            free(else_label);
            free(end_label);
            break;
        }
        
        case AST_RETURN: {
            char* value = generate_expression(node->data.return_stmt.value, ctx, table);
            fprintf(out, "  ret i32 %s\n", value);
            free(value);
            break;
        }
        
        case AST_PAINT: {
            if (ctx->in_stencil) {
                char* color = generate_expression(node->data.paint.value, ctx, table);
                char* rel_x = new_temp(ctx);
                char* rel_y = new_temp(ctx);
                char* offset_x = new_temp(ctx);
                char* offset_y = new_temp(ctx);
                char* abs_x = new_temp(ctx);
                char* abs_y = new_temp(ctx);
                
                // Load relative coordinates and offsets
                fprintf(out, "  %s = load i32, i32* %%x\n", rel_x);
                fprintf(out, "  %s = load i32, i32* %%y\n", rel_y);
                fprintf(out, "  %s = load i32, i32* %%ox\n", offset_x);
                fprintf(out, "  %s = load i32, i32* %%oy\n", offset_y);
                
                // Calculate absolute coordinates
                fprintf(out, "  %s = add i32 %s, %s\n", abs_x, rel_x, offset_x);
                fprintf(out, "  %s = add i32 %s, %s\n", abs_y, rel_y, offset_y);
                
                // Paint at absolute coordinates
                fprintf(out, "  call void @paint_pixel(i32 %s, i32 %s, i32 %s)\n", abs_x, abs_y, color);
                fprintf(out, "  ret void\n");
                
                free(color);
                free(rel_x); free(rel_y);
                free(offset_x); free(offset_y);
                free(abs_x); free(abs_y);
            }
            break;
        }
        
        case AST_FUNC_CALL: {
            char* result = generate_expression(node, ctx, table);
            free(result);
            break;
        }
        
        default:
            break;
    }
}

void generate_global_decls(ASTNode* node, CodeGenContext* ctx, SymbolTable* table) {
    if (!node) return;
    
    FILE* out = ctx->output;
    
    switch (node->type) {
        case AST_STATEMENT_LIST:
            if (node->data.list.head) {
                generate_global_decls(node->data.list.head, ctx, table);
            }
            if (node->data.list.tail) {
                generate_global_decls(node->data.list.tail, ctx, table);
            }
            break;
            
        case AST_VAR_DEC: {
            // Global variables
            fprintf(out, "@%s = global i32 ", node->data.var_dec.name);
            if (node->data.var_dec.value && node->data.var_dec.value->type == AST_NUMBER) {
                fprintf(out, "%d\n", node->data.var_dec.value->data.number.value);
            } else {
                fprintf(out, "0\n");
            }
            
            char* global_name = (char*)malloc(strlen(node->data.var_dec.name) + 2);
            sprintf(global_name, "@%s", node->data.var_dec.name);
            add_var(table, node->data.var_dec.name, global_name);
            free(global_name);
            break;
        }
        
        case AST_FUNC_DEC: {
            // Count parameters
            int param_count = 0;
            ASTNode* params = node->data.func_dec.params;
            while (params) {
                if (params->type == AST_PARAMETER_LIST) {
                    param_count++;
                    params = params->data.list.tail;
                } else {
                    param_count++;
                    break;
                }
            }
            
            add_func(table, node->data.func_dec.name, param_count);
            
            // Generate function
            fprintf(out, "define i32 @%s(", node->data.func_dec.name);
            
            SymbolTable* local_table = create_symbol_table();
            local_table->vars = table->vars; // Inherit global vars
            
            // Generate parameters
            params = node->data.func_dec.params;
            int param_idx = 0;
            while (params) {
                if (param_idx > 0) fprintf(out, ", ");
                
                ASTNode* param = params;
                if (params->type == AST_PARAMETER_LIST) {
                    param = params->data.list.head;
                    params = params->data.list.tail;
                } else {
                    params = NULL;
                }
                
                if (param && param->type == AST_VAR_DEC) {
                    fprintf(out, "i32 %%%s", param->data.var_dec.name);
                    
                    char* param_name = (char*)malloc(strlen(param->data.var_dec.name) + 2);
                    sprintf(param_name, "%%%s", param->data.var_dec.name);
                    add_var(local_table, param->data.var_dec.name, param_name);
                    free(param_name);
                }
                
                param_idx++;
            }
            
            fprintf(out, ") {\n");
            fprintf(out, "entry:\n");
            
            // Generate function body
            ctx->current_function = strdup(node->data.func_dec.name);
            generate_statement(node->data.func_dec.body, ctx, local_table);
            
            // Add default return if needed
            fprintf(out, "  ret i32 0\n");
            fprintf(out, "}\n\n");
            
            free(ctx->current_function);
            ctx->current_function = NULL;
            
            // Don't free local_table->vars since it points to global vars
            local_table->vars = NULL;
            free_symbol_table(local_table);
            break;
        }
        
        case AST_STENCIL: {
            add_stencil(table, node->data.stencil.name);
            
            // Generate stencil function with offset parameters
            fprintf(out, "define void @stencil_%s(i32 %%x_val, i32 %%y_val, i32 %%offset_x, i32 %%offset_y) {\n", node->data.stencil.name);
            fprintf(out, "entry:\n");
            fprintf(out, "  %%x = alloca i32\n");
            fprintf(out, "  %%y = alloca i32\n");
            fprintf(out, "  %%ox = alloca i32\n");
            fprintf(out, "  %%oy = alloca i32\n");
            fprintf(out, "  store i32 %%x_val, i32* %%x\n");
            fprintf(out, "  store i32 %%y_val, i32* %%y\n");
            fprintf(out, "  store i32 %%offset_x, i32* %%ox\n");
            fprintf(out, "  store i32 %%offset_y, i32* %%oy\n");
            
            SymbolTable* stencil_table = create_symbol_table();
            stencil_table->vars = table->vars; // Inherit global vars
            
            ctx->in_stencil = 1;
            generate_statement(node->data.stencil.body, ctx, stencil_table);
            ctx->in_stencil = 0;
            
            fprintf(out, "  ret void\n");
            fprintf(out, "}\n\n");
            
            // Don't free stencil_table->vars since it points to global vars
            stencil_table->vars = NULL;
            free_symbol_table(stencil_table);
            break;
        }
        
        default:
            break;
    }
}

void emit_main_function(CodeGenContext* ctx, SymbolTable* table) {
    FILE* out = ctx->output;
    
    fprintf(out, "define i32 @main() {\n");
    fprintf(out, "entry:\n");
    
    fprintf(out, "  ; Initialize canvas\n");
    
    fprintf(out, "  ret i32 0\n");
    fprintf(out, "}\n");
}

void generate_apply_statements(ASTNode* node, CodeGenContext* ctx, SymbolTable* table) {
    if (!node) return;
    
    FILE* out = ctx->output;
    
    switch (node->type) {
        case AST_STATEMENT_LIST:
            if (node->data.list.head) {
                generate_apply_statements(node->data.list.head, ctx, table);
            }
            if (node->data.list.tail) {
                generate_apply_statements(node->data.list.tail, ctx, table);
            }
            break;
            
        case AST_APPLY: {
            StencilEntry* stencil = lookup_stencil(table, node->data.apply.name);
            if (!stencil) break;
            
            int start_x = 0, start_y = 0;
            int size = 50;
            ASTNode* directive_queue[10];
            int queue_front = 0, queue_back = 0;
            
            if (node->data.apply.directives) {
                directive_queue[queue_back++] = node->data.apply.directives;
            }
            
            while (queue_front < queue_back) {
                ASTNode* current = directive_queue[queue_front++];
                if (!current) continue;
                
                if (current->type == AST_LOCATION_DIRECTIVE) {
                    ASTNode* coord = current->data.location_directive.coordinate;
                    if (coord && coord->type == AST_COORDINATE) {
                        if (coord->data.coordinate.x && coord->data.coordinate.x->type == AST_NUMBER) {
                            start_x = coord->data.coordinate.x->data.number.value;
                        }
                        if (coord->data.coordinate.y && coord->data.coordinate.y->type == AST_NUMBER) {
                            start_y = coord->data.coordinate.y->data.number.value;
                        }
                    }
                } else if (current->type == AST_SIZE_DIRECTIVE) {
                    if (current->data.size_directive.size && 
                        current->data.size_directive.size->type == AST_NUMBER) {
                        size = current->data.size_directive.size->data.number.value;
                    }
                } else if (current->type == AST_DIRECTIVE_LIST || current->type == AST_STATEMENT_LIST) {
                    if (current->data.list.head && queue_back < 9) {
                        directive_queue[queue_back++] = current->data.list.head;
                    }
                    if (current->data.list.tail && queue_back < 9) {
                        directive_queue[queue_back++] = current->data.list.tail;
                    }
                }
            }
            
            char* y_counter = new_temp(ctx);
            char* x_counter = new_temp(ctx);
            char* y_cond = new_temp(ctx);
            char* x_cond = new_temp(ctx);
            char* y_next = new_temp(ctx);
            char* x_next = new_temp(ctx);
            
            char* y_loop = new_label(ctx);
            char* y_body = new_label(ctx);
            char* y_exit = new_label(ctx);
            char* x_loop = new_label(ctx);
            char* x_body = new_label(ctx);
            char* x_exit = new_label(ctx);
            
            fprintf(out, "  ; Apply stencil %s\n", node->data.apply.name);
            
            fprintf(out, "  br label %%%s\n", y_loop);
            fprintf(out, "%s:\n", y_loop);
            fprintf(out, "  %s = phi i32 [0, %%entry], [%s, %%%s]\n", 
                    y_counter, y_next, x_exit);
            fprintf(out, "  %s = icmp slt i32 %s, %d\n", y_cond, y_counter, size);
            fprintf(out, "  br i1 %s, label %%%s, label %%%s\n", y_cond, y_body, y_exit);
            
            fprintf(out, "%s:\n", y_body);
            
            fprintf(out, "  br label %%%s\n", x_loop);
            fprintf(out, "%s:\n", x_loop);
            fprintf(out, "  %s = phi i32 [0, %%%s], [%s, %%%s]\n", 
                    x_counter, y_body, x_next, x_body);
            fprintf(out, "  %s = icmp slt i32 %s, %d\n", x_cond, x_counter, size);
            fprintf(out, "  br i1 %s, label %%%s, label %%%s\n", x_cond, x_body, x_exit);
            
            fprintf(out, "%s:\n", x_body);
            
            fprintf(out, "  call void @stencil_%s(i32 %s, i32 %s, i32 %d, i32 %d)\n", 
                    node->data.apply.name, x_counter, y_counter, start_x, start_y);
            
            fprintf(out, "  %s = add i32 %s, 1\n", x_next, x_counter);
            fprintf(out, "  br label %%%s\n", x_loop);
            
            fprintf(out, "%s:\n", x_exit);
            
            fprintf(out, "  %s = add i32 %s, 1\n", y_next, y_counter);
            fprintf(out, "  br label %%%s\n", y_loop);
            
            fprintf(out, "%s:\n", y_exit);
            
            free(y_counter); free(x_counter);
            free(y_cond); free(x_cond);
            free(y_next); free(x_next);
            free(y_loop); free(y_body); free(y_exit);
            free(x_loop); free(x_body); free(x_exit);
            break;
        }
        
        default:
            break;
    }
}

void generate_code(ASTNode* ast, CodeGenContext* ctx, SymbolTable* global_table) {
    FILE* out = ctx->output;
    
    // LLVM module header
    fprintf(out, "; ModuleID = 'stencil'\n");
    fprintf(out, "source_filename = \"stencil\"\n");
    fprintf(out, "target datalayout = \"e-m:o-i64:64-i128:128-n32:64-S128\"\n");
    fprintf(out, "target triple = \"arm64-apple-macosx14.0.0\"\n\n");
    
    // Emit runtime function declarations
    emit_runtime_functions(ctx);
    
    // First pass: generate global declarations, functions, and stencils
    generate_global_decls(ast, ctx, global_table);
    
    // Generate main function with apply statements
    fprintf(out, "define i32 @llvm_main() {\n");
    fprintf(out, "entry:\n");
    
    // Reset temp counter for main function
    ctx->temp_counter = 0;
    
    // Process apply statements
    generate_apply_statements(ast, ctx, global_table);
    
    fprintf(out, "  ret i32 0\n");
    fprintf(out, "}\n");
}