#include "ast.h"
#include <stdio.h>

ASTNode* create_node(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    return node;
}

ASTNode* create_number(int value) {
    ASTNode* node = create_node(AST_NUMBER);
    node->data.number.value = value;
    return node;
}

ASTNode* create_identifier(char* name) {
    ASTNode* node = create_node(AST_IDENTIFIER);
    node->data.identifier.name = strdup(name);
    return node;
}

ASTNode* create_binary_op(OpType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = create_node(AST_BINARY_OP);
    node->data.binary_op.op = op;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

ASTNode* create_unary_op(OpType op, ASTNode* operand) {
    ASTNode* node = create_node(AST_UNARY_OP);
    node->data.unary_op.op = op;
    node->data.unary_op.operand = operand;
    return node;
}

ASTNode* create_assignment(char* name, ASTNode* value) {
    ASTNode* node = create_node(AST_ASSIGNMENT);
    node->data.assignment.name = strdup(name);
    node->data.assignment.value = value;
    return node;
}

ASTNode* create_var_dec(char* name, ASTNode* value) {
    ASTNode* node = create_node(AST_VAR_DEC);
    node->data.var_dec.name = strdup(name);
    node->data.var_dec.value = value;
    return node;
}

ASTNode* create_block(ASTNode* statements) {
    ASTNode* node = create_node(AST_BLOCK);
    node->data.block.statements = statements;
    return node;
}

ASTNode* create_if(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt) {
    ASTNode* node = create_node(AST_IF);
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_stmt = then_stmt;
    node->data.if_stmt.else_stmt = else_stmt;
    return node;
}

ASTNode* create_func_dec(char* name, ASTNode* params, ASTNode* body) {
    ASTNode* node = create_node(AST_FUNC_DEC);
    node->data.func_dec.name = strdup(name);
    node->data.func_dec.params = params;
    node->data.func_dec.body = body;
    return node;
}

ASTNode* create_func_call(char* name, ASTNode* args) {
    ASTNode* node = create_node(AST_FUNC_CALL);
    node->data.func_call.name = strdup(name);
    node->data.func_call.args = args;
    return node;
}

ASTNode* create_stencil(char* name, ASTNode* body) {
    ASTNode* node = create_node(AST_STENCIL);
    node->data.stencil.name = strdup(name);
    node->data.stencil.body = body;
    return node;
}

ASTNode* create_apply(char* name, ASTNode* directives) {
    ASTNode* node = create_node(AST_APPLY);
    node->data.apply.name = strdup(name);
    node->data.apply.directives = directives;
    return node;
}

ASTNode* create_paint(ASTNode* value) {
    ASTNode* node = create_node(AST_PAINT);
    node->data.paint.value = value;
    return node;
}

ASTNode* create_return(ASTNode* value) {
    ASTNode* node = create_node(AST_RETURN);
    node->data.return_stmt.value = value;
    return node;
}

ASTNode* create_coordinate(ASTNode* x, ASTNode* y) {
    ASTNode* node = create_node(AST_COORDINATE);
    node->data.coordinate.x = x;
    node->data.coordinate.y = y;
    return node;
}

ASTNode* create_list(ASTNode* head, ASTNode* tail) {
    ASTNode* node = create_node(AST_STATEMENT_LIST);
    node->data.list.head = head;
    node->data.list.tail = tail;
    return node;
}

ASTNode* create_location_directive(ASTNode* coordinate) {
    ASTNode* node = create_node(AST_LOCATION_DIRECTIVE);
    node->data.location_directive.coordinate = coordinate;
    return node;
}

ASTNode* create_size_directive(ASTNode* size) {
    ASTNode* node = create_node(AST_SIZE_DIRECTIVE);
    node->data.size_directive.size = size;
    return node;
}

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

const char* op_to_string(OpType op) {
    switch (op) {
        case OP_PLUS: return "+";
        case OP_MINUS: return "-";
        case OP_TIMES: return "*";
        case OP_DIVIDE: return "/";
        case OP_EQUALS: return "==";
        case OP_GREATER: return ">";
        case OP_LESS: return "<";
        case OP_AND: return "&&";
        case OP_OR: return "||";
        case OP_NOT: return "!";
        default: return "?";
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            print_ast(node->data.list.head, indent + 1);
            break;
            
        case AST_NUMBER:
            printf("Number: %d\n", node->data.number.value);
            break;
            
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->data.identifier.name);
            break;
            
        case AST_BINARY_OP:
            printf("BinaryOp: %s\n", op_to_string(node->data.binary_op.op));
            print_ast(node->data.binary_op.left, indent + 1);
            print_ast(node->data.binary_op.right, indent + 1);
            break;
            
        case AST_UNARY_OP:
            printf("UnaryOp: %s\n", op_to_string(node->data.unary_op.op));
            print_ast(node->data.unary_op.operand, indent + 1);
            break;
            
        case AST_ASSIGNMENT:
            printf("Assignment: %s\n", node->data.assignment.name);
            print_ast(node->data.assignment.value, indent + 1);
            break;
            
        case AST_VAR_DEC:
            printf("VarDeclaration: %s\n", node->data.var_dec.name);
            if (node->data.var_dec.value) {
                print_ast(node->data.var_dec.value, indent + 1);
            }
            break;
            
        case AST_BLOCK:
            printf("Block\n");
            print_ast(node->data.block.statements, indent + 1);
            break;
            
        case AST_IF:
            printf("If\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->data.if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("Then:\n");
            print_ast(node->data.if_stmt.then_stmt, indent + 2);
            if (node->data.if_stmt.else_stmt) {
                print_indent(indent + 1);
                printf("Else:\n");
                print_ast(node->data.if_stmt.else_stmt, indent + 2);
            }
            break;
            
        case AST_FUNC_DEC:
            printf("FunctionDeclaration: %s\n", node->data.func_dec.name);
            if (node->data.func_dec.params) {
                print_indent(indent + 1);
                printf("Parameters:\n");
                print_ast(node->data.func_dec.params, indent + 2);
            }
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->data.func_dec.body, indent + 2);
            break;
            
        case AST_FUNC_CALL:
            printf("FunctionCall: %s\n", node->data.func_call.name);
            if (node->data.func_call.args) {
                print_ast(node->data.func_call.args, indent + 1);
            }
            break;
            
        case AST_STENCIL:
            printf("Stencil: %s\n", node->data.stencil.name);
            print_ast(node->data.stencil.body, indent + 1);
            break;
            
        case AST_APPLY:
            printf("Apply: %s\n", node->data.apply.name);
            if (node->data.apply.directives) {
                print_ast(node->data.apply.directives, indent + 1);
            }
            break;
            
        case AST_PAINT:
            printf("Paint\n");
            print_ast(node->data.paint.value, indent + 1);
            break;
            
        case AST_RETURN:
            printf("Return\n");
            print_ast(node->data.return_stmt.value, indent + 1);
            break;
            
        case AST_COORDINATE:
            printf("Coordinate\n");
            print_ast(node->data.coordinate.x, indent + 1);
            print_ast(node->data.coordinate.y, indent + 1);
            break;
            
        case AST_STATEMENT_LIST:
        case AST_EXPRESSION_LIST:
        case AST_PARAMETER_LIST:
        case AST_DIRECTIVE_LIST:
            if (node->data.list.head) {
                print_ast(node->data.list.head, indent);
            }
            if (node->data.list.tail) {
                print_ast(node->data.list.tail, indent);
            }
            break;
            
        case AST_LOCATION_DIRECTIVE:
            printf("LocationDirective\n");
            print_ast(node->data.location_directive.coordinate, indent + 1);
            break;
            
        case AST_SIZE_DIRECTIVE:
            printf("SizeDirective\n");
            print_ast(node->data.size_directive.size, indent + 1);
            break;
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case AST_BINARY_OP:
            free_ast(node->data.binary_op.left);
            free_ast(node->data.binary_op.right);
            break;
            
        case AST_UNARY_OP:
            free_ast(node->data.unary_op.operand);
            break;
            
        case AST_ASSIGNMENT:
            free(node->data.assignment.name);
            free_ast(node->data.assignment.value);
            break;
            
        case AST_VAR_DEC:
            free(node->data.var_dec.name);
            free_ast(node->data.var_dec.value);
            break;
            
        case AST_BLOCK:
            free_ast(node->data.block.statements);
            break;
            
        case AST_IF:
            free_ast(node->data.if_stmt.condition);
            free_ast(node->data.if_stmt.then_stmt);
            free_ast(node->data.if_stmt.else_stmt);
            break;
            
        case AST_FUNC_DEC:
            free(node->data.func_dec.name);
            free_ast(node->data.func_dec.params);
            free_ast(node->data.func_dec.body);
            break;
            
        case AST_FUNC_CALL:
            free(node->data.func_call.name);
            free_ast(node->data.func_call.args);
            break;
            
        case AST_STENCIL:
            free(node->data.stencil.name);
            free_ast(node->data.stencil.body);
            break;
            
        case AST_APPLY:
            free(node->data.apply.name);
            free_ast(node->data.apply.directives);
            break;
            
        case AST_PAINT:
            free_ast(node->data.paint.value);
            break;
            
        case AST_RETURN:
            free_ast(node->data.return_stmt.value);
            break;
            
        case AST_COORDINATE:
            free_ast(node->data.coordinate.x);
            free_ast(node->data.coordinate.y);
            break;
            
        case AST_STATEMENT_LIST:
        case AST_EXPRESSION_LIST:
        case AST_PARAMETER_LIST:
        case AST_DIRECTIVE_LIST:
            free_ast(node->data.list.head);
            free_ast(node->data.list.tail);
            break;
            
        case AST_LOCATION_DIRECTIVE:
            free_ast(node->data.location_directive.coordinate);
            break;
            
        case AST_SIZE_DIRECTIVE:
            free_ast(node->data.size_directive.size);
            break;
            
        default:
            break;
    }
    
    free(node);
}