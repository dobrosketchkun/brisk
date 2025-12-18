/*
 * Brisk Language - AST Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Node type names */
static const char* node_names[] = {
    [NODE_LITERAL_INT] = "INT_LITERAL",
    [NODE_LITERAL_FLOAT] = "FLOAT_LITERAL",
    [NODE_LITERAL_STRING] = "STRING_LITERAL",
    [NODE_LITERAL_BOOL] = "BOOL_LITERAL",
    [NODE_LITERAL_NIL] = "NIL_LITERAL",
    [NODE_IDENTIFIER] = "IDENTIFIER",
    [NODE_BINARY] = "BINARY",
    [NODE_UNARY] = "UNARY",
    [NODE_CALL] = "CALL",
    [NODE_INDEX] = "INDEX",
    [NODE_FIELD] = "FIELD",
    [NODE_ARRAY] = "ARRAY",
    [NODE_TABLE] = "TABLE",
    [NODE_LAMBDA] = "LAMBDA",
    [NODE_RANGE] = "RANGE",
    [NODE_ADDRESS_OF] = "ADDRESS_OF",
    [NODE_VAR_DECL] = "VAR_DECL",
    [NODE_CONST_DECL] = "CONST_DECL",
    [NODE_ASSIGNMENT] = "ASSIGNMENT",
    [NODE_EXPR_STMT] = "EXPR_STMT",
    [NODE_BLOCK] = "BLOCK",
    [NODE_IF] = "IF",
    [NODE_WHILE] = "WHILE",
    [NODE_FOR] = "FOR",
    [NODE_RETURN] = "RETURN",
    [NODE_BREAK] = "BREAK",
    [NODE_CONTINUE] = "CONTINUE",
    [NODE_FN_DECL] = "FN_DECL",
    [NODE_MATCH] = "MATCH",
    [NODE_DEFER] = "DEFER",
    [NODE_PROGRAM] = "PROGRAM",
    [NODE_IMPORT] = "IMPORT",
    [NODE_C_BLOCK] = "C_BLOCK",
    [NODE_CTYPE] = "CTYPE",
};

const char* node_type_name(NodeType type) {
    if (type >= 0 && type < NODE_TYPE_COUNT) {
        return node_names[type];
    }
    return "UNKNOWN";
}

/* Helper to duplicate a string */
static char* str_dup(const char* src, int length) {
    char* dst = malloc(length + 1);
    if (dst) {
        memcpy(dst, src, length);
        dst[length] = '\0';
    }
    return dst;
}

/* Create a basic node */
AstNode* ast_create_node(NodeType type, int line, int column) {
    AstNode* node = malloc(sizeof(AstNode));
    if (node) {
        memset(node, 0, sizeof(AstNode));
        node->type = type;
        node->line = line;
        node->column = column;
    }
    return node;
}

/* Node constructors */
AstNode* ast_int_literal(int64_t value, int line, int column) {
    AstNode* node = ast_create_node(NODE_LITERAL_INT, line, column);
    if (node) {
        node->as.int_literal.value = value;
    }
    return node;
}

AstNode* ast_float_literal(double value, int line, int column) {
    AstNode* node = ast_create_node(NODE_LITERAL_FLOAT, line, column);
    if (node) {
        node->as.float_literal.value = value;
    }
    return node;
}

AstNode* ast_string_literal(const char* value, int length, int line, int column) {
    AstNode* node = ast_create_node(NODE_LITERAL_STRING, line, column);
    if (node) {
        node->as.string_literal.value = str_dup(value, length);
        node->as.string_literal.length = length;
    }
    return node;
}

AstNode* ast_bool_literal(bool value, int line, int column) {
    AstNode* node = ast_create_node(NODE_LITERAL_BOOL, line, column);
    if (node) {
        node->as.bool_literal.value = value;
    }
    return node;
}

AstNode* ast_nil_literal(int line, int column) {
    return ast_create_node(NODE_LITERAL_NIL, line, column);
}

AstNode* ast_identifier(const char* name, int length, int line, int column) {
    AstNode* node = ast_create_node(NODE_IDENTIFIER, line, column);
    if (node) {
        node->as.identifier.name = str_dup(name, length);
        node->as.identifier.name_length = length;
    }
    return node;
}

AstNode* ast_binary(TokenType op, AstNode* left, AstNode* right, int line, int column) {
    AstNode* node = ast_create_node(NODE_BINARY, line, column);
    if (node) {
        node->as.binary.operator = op;
        node->as.binary.left = left;
        node->as.binary.right = right;
    }
    return node;
}

AstNode* ast_unary(TokenType op, AstNode* operand, int line, int column) {
    AstNode* node = ast_create_node(NODE_UNARY, line, column);
    if (node) {
        node->as.unary.operator = op;
        node->as.unary.operand = operand;
    }
    return node;
}

AstNode* ast_call(AstNode* callee, AstNode** args, int arg_count, int line, int column) {
    AstNode* node = ast_create_node(NODE_CALL, line, column);
    if (node) {
        node->as.call.callee = callee;
        node->as.call.arguments = args;
        node->as.call.arg_count = arg_count;
    }
    return node;
}

AstNode* ast_index(AstNode* object, AstNode* index, int line, int column) {
    AstNode* node = ast_create_node(NODE_INDEX, line, column);
    if (node) {
        node->as.index.object = object;
        node->as.index.index = index;
    }
    return node;
}

AstNode* ast_field(AstNode* object, const char* field, int length, int line, int column) {
    AstNode* node = ast_create_node(NODE_FIELD, line, column);
    if (node) {
        node->as.field.object = object;
        node->as.field.field_name = str_dup(field, length);
        node->as.field.field_name_length = length;
    }
    return node;
}

AstNode* ast_array(AstNode** elements, int count, int line, int column) {
    AstNode* node = ast_create_node(NODE_ARRAY, line, column);
    if (node) {
        node->as.array.elements = elements;
        node->as.array.element_count = count;
    }
    return node;
}

AstNode* ast_table(char** keys, int* key_lengths, AstNode** values, int count, int line, int column) {
    AstNode* node = ast_create_node(NODE_TABLE, line, column);
    if (node) {
        node->as.table.keys = keys;
        node->as.table.key_lengths = key_lengths;
        node->as.table.values = values;
        node->as.table.count = count;
    }
    return node;
}

AstNode* ast_range(AstNode* start, AstNode* end, int line, int column) {
    AstNode* node = ast_create_node(NODE_RANGE, line, column);
    if (node) {
        node->as.range.start = start;
        node->as.range.end = end;
    }
    return node;
}

AstNode* ast_var_decl(const char* name, int length, AstNode* init, bool is_const, int line, int column) {
    AstNode* node = ast_create_node(is_const ? NODE_CONST_DECL : NODE_VAR_DECL, line, column);
    if (node) {
        node->as.var_decl.name = str_dup(name, length);
        node->as.var_decl.name_length = length;
        node->as.var_decl.initializer = init;
        node->as.var_decl.is_const = is_const;
    }
    return node;
}

AstNode* ast_assignment(AstNode* target, AstNode* value, int line, int column) {
    AstNode* node = ast_create_node(NODE_ASSIGNMENT, line, column);
    if (node) {
        node->as.assignment.target = target;
        node->as.assignment.value = value;
    }
    return node;
}

AstNode* ast_block(AstNode** stmts, int count, int line, int column) {
    AstNode* node = ast_create_node(NODE_BLOCK, line, column);
    if (node) {
        node->as.block.statements = stmts;
        node->as.block.statement_count = count;
    }
    return node;
}

AstNode* ast_if(AstNode* cond, AstNode* then_b, AstNode* else_b, int line, int column) {
    AstNode* node = ast_create_node(NODE_IF, line, column);
    if (node) {
        node->as.if_stmt.condition = cond;
        node->as.if_stmt.then_branch = then_b;
        node->as.if_stmt.else_branch = else_b;
    }
    return node;
}

AstNode* ast_while(AstNode* cond, AstNode* body, int line, int column) {
    AstNode* node = ast_create_node(NODE_WHILE, line, column);
    if (node) {
        node->as.while_stmt.condition = cond;
        node->as.while_stmt.body = body;
    }
    return node;
}

AstNode* ast_for(const char* iter, int iter_len, AstNode* iterable, AstNode* body, int line, int column) {
    AstNode* node = ast_create_node(NODE_FOR, line, column);
    if (node) {
        node->as.for_stmt.iterator_name = str_dup(iter, iter_len);
        node->as.for_stmt.iterator_name_length = iter_len;
        node->as.for_stmt.iterable = iterable;
        node->as.for_stmt.body = body;
    }
    return node;
}

AstNode* ast_fn_decl(const char* name, int name_len, char** params, int* param_lens, int param_count, AstNode* body, int line, int column) {
    AstNode* node = ast_create_node(NODE_FN_DECL, line, column);
    if (node) {
        node->as.fn_decl.name = str_dup(name, name_len);
        node->as.fn_decl.name_length = name_len;
        node->as.fn_decl.parameters = params;
        node->as.fn_decl.param_lengths = param_lens;
        node->as.fn_decl.param_count = param_count;
        node->as.fn_decl.body = body;
    }
    return node;
}

AstNode* ast_lambda(char** params, int* param_lens, int param_count, AstNode* body, int line, int column) {
    AstNode* node = ast_create_node(NODE_LAMBDA, line, column);
    if (node) {
        node->as.lambda.parameters = params;
        node->as.lambda.param_lengths = param_lens;
        node->as.lambda.param_count = param_count;
        node->as.lambda.body = body;
    }
    return node;
}

AstNode* ast_return(AstNode* value, int line, int column) {
    AstNode* node = ast_create_node(NODE_RETURN, line, column);
    if (node) {
        node->as.return_stmt.value = value;
    }
    return node;
}

AstNode* ast_break(int line, int column) {
    return ast_create_node(NODE_BREAK, line, column);
}

AstNode* ast_continue(int line, int column) {
    return ast_create_node(NODE_CONTINUE, line, column);
}

AstNode* ast_match(AstNode* value, AstNode** patterns, AstNode** bodies, int arm_count, int line, int column) {
    AstNode* node = ast_create_node(NODE_MATCH, line, column);
    if (node) {
        node->as.match_stmt.value = value;
        node->as.match_stmt.patterns = patterns;
        node->as.match_stmt.bodies = bodies;
        node->as.match_stmt.arm_count = arm_count;
    }
    return node;
}

AstNode* ast_defer(AstNode* stmt, int line, int column) {
    AstNode* node = ast_create_node(NODE_DEFER, line, column);
    if (node) {
        node->as.defer_stmt.statement = stmt;
    }
    return node;
}

AstNode* ast_import(const char* path, int length, int line, int column) {
    AstNode* node = ast_create_node(NODE_IMPORT, line, column);
    if (node) {
        node->as.import.path = str_dup(path, length);
        node->as.import.path_length = length;
    }
    return node;
}

AstNode* ast_c_block(const char* code, int length, int line, int column) {
    AstNode* node = ast_create_node(NODE_C_BLOCK, line, column);
    if (node) {
        node->as.c_block.code = str_dup(code, length);
        node->as.c_block.code_length = length;
    }
    return node;
}

AstNode* ast_program(AstNode** stmts, int count) {
    AstNode* node = ast_create_node(NODE_PROGRAM, 1, 1);
    if (node) {
        node->as.program.statements = stmts;
        node->as.program.statement_count = count;
    }
    return node;
}

AstNode* ast_address_of(AstNode* operand, int line, int column) {
    AstNode* node = ast_create_node(NODE_ADDRESS_OF, line, column);
    if (node) {
        node->as.address_of.operand = operand;
    }
    return node;
}

AstNode* ast_expr_stmt(AstNode* expr, int line, int column) {
    AstNode* node = ast_create_node(NODE_EXPR_STMT, line, column);
    if (node) {
        /* Reuse unary for single expression */
        node->as.unary.operand = expr;
    }
    return node;
}

/* Free a single node's data (not recursively) */
static void free_node_data(AstNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_LITERAL_STRING:
            free(node->as.string_literal.value);
            break;
        case NODE_IDENTIFIER:
            free(node->as.identifier.name);
            break;
        case NODE_FIELD:
            free(node->as.field.field_name);
            break;
        case NODE_VAR_DECL:
        case NODE_CONST_DECL:
            free(node->as.var_decl.name);
            break;
        case NODE_FOR:
            free(node->as.for_stmt.iterator_name);
            break;
        case NODE_FN_DECL:
            free(node->as.fn_decl.name);
            for (int i = 0; i < node->as.fn_decl.param_count; i++) {
                free(node->as.fn_decl.parameters[i]);
            }
            free(node->as.fn_decl.parameters);
            free(node->as.fn_decl.param_lengths);
            break;
        case NODE_LAMBDA:
            for (int i = 0; i < node->as.lambda.param_count; i++) {
                free(node->as.lambda.parameters[i]);
            }
            free(node->as.lambda.parameters);
            free(node->as.lambda.param_lengths);
            break;
        case NODE_TABLE:
            for (int i = 0; i < node->as.table.count; i++) {
                free(node->as.table.keys[i]);
            }
            free(node->as.table.keys);
            free(node->as.table.key_lengths);
            break;
        case NODE_IMPORT:
            free(node->as.import.path);
            break;
        case NODE_C_BLOCK:
            free(node->as.c_block.code);
            break;
        default:
            break;
    }
}

/* Free a node and all its children */
void ast_free_tree(AstNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_BINARY:
            ast_free_tree(node->as.binary.left);
            ast_free_tree(node->as.binary.right);
            break;
        case NODE_UNARY:
        case NODE_EXPR_STMT:
            ast_free_tree(node->as.unary.operand);
            break;
        case NODE_CALL:
            ast_free_tree(node->as.call.callee);
            for (int i = 0; i < node->as.call.arg_count; i++) {
                ast_free_tree(node->as.call.arguments[i]);
            }
            free(node->as.call.arguments);
            break;
        case NODE_INDEX:
            ast_free_tree(node->as.index.object);
            ast_free_tree(node->as.index.index);
            break;
        case NODE_FIELD:
            ast_free_tree(node->as.field.object);
            break;
        case NODE_ARRAY:
            for (int i = 0; i < node->as.array.element_count; i++) {
                ast_free_tree(node->as.array.elements[i]);
            }
            free(node->as.array.elements);
            break;
        case NODE_TABLE:
            for (int i = 0; i < node->as.table.count; i++) {
                ast_free_tree(node->as.table.values[i]);
            }
            free(node->as.table.values);
            break;
        case NODE_RANGE:
            ast_free_tree(node->as.range.start);
            ast_free_tree(node->as.range.end);
            break;
        case NODE_VAR_DECL:
        case NODE_CONST_DECL:
            ast_free_tree(node->as.var_decl.initializer);
            break;
        case NODE_ASSIGNMENT:
            ast_free_tree(node->as.assignment.target);
            ast_free_tree(node->as.assignment.value);
            break;
        case NODE_BLOCK:
            for (int i = 0; i < node->as.block.statement_count; i++) {
                ast_free_tree(node->as.block.statements[i]);
            }
            free(node->as.block.statements);
            break;
        case NODE_IF:
            ast_free_tree(node->as.if_stmt.condition);
            ast_free_tree(node->as.if_stmt.then_branch);
            ast_free_tree(node->as.if_stmt.else_branch);
            break;
        case NODE_WHILE:
            ast_free_tree(node->as.while_stmt.condition);
            ast_free_tree(node->as.while_stmt.body);
            break;
        case NODE_FOR:
            ast_free_tree(node->as.for_stmt.iterable);
            ast_free_tree(node->as.for_stmt.body);
            break;
        case NODE_FN_DECL:
            ast_free_tree(node->as.fn_decl.body);
            break;
        case NODE_LAMBDA:
            ast_free_tree(node->as.lambda.body);
            break;
        case NODE_RETURN:
            ast_free_tree(node->as.return_stmt.value);
            break;
        case NODE_MATCH:
            ast_free_tree(node->as.match_stmt.value);
            for (int i = 0; i < node->as.match_stmt.arm_count; i++) {
                ast_free_tree(node->as.match_stmt.patterns[i]);
                ast_free_tree(node->as.match_stmt.bodies[i]);
            }
            free(node->as.match_stmt.patterns);
            free(node->as.match_stmt.bodies);
            break;
        case NODE_DEFER:
            ast_free_tree(node->as.defer_stmt.statement);
            break;
        case NODE_PROGRAM:
            for (int i = 0; i < node->as.program.statement_count; i++) {
                ast_free_tree(node->as.program.statements[i]);
            }
            free(node->as.program.statements);
            break;
        case NODE_ADDRESS_OF:
            ast_free_tree(node->as.address_of.operand);
            break;
        default:
            break;
    }
    
    free_node_data(node);
    free(node);
}

void ast_free_node(AstNode* node) {
    if (!node) return;
    free_node_data(node);
    free(node);
}

/* Print indentation */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

/* Debug print AST */
void ast_print(AstNode* node, int indent) {
    if (!node) {
        print_indent(indent);
        printf("(null)\n");
        return;
    }
    
    print_indent(indent);
    printf("%s", node_type_name(node->type));
    
    switch (node->type) {
        case NODE_LITERAL_INT:
            printf(" %ld\n", (long)node->as.int_literal.value);
            break;
        case NODE_LITERAL_FLOAT:
            printf(" %g\n", node->as.float_literal.value);
            break;
        case NODE_LITERAL_STRING:
            printf(" \"%.*s\"\n", node->as.string_literal.length, node->as.string_literal.value);
            break;
        case NODE_LITERAL_BOOL:
            printf(" %s\n", node->as.bool_literal.value ? "true" : "false");
            break;
        case NODE_LITERAL_NIL:
            printf("\n");
            break;
        case NODE_IDENTIFIER:
            printf(" %.*s\n", node->as.identifier.name_length, node->as.identifier.name);
            break;
        case NODE_BINARY:
            printf(" op=%s\n", token_type_name(node->as.binary.operator));
            ast_print(node->as.binary.left, indent + 1);
            ast_print(node->as.binary.right, indent + 1);
            break;
        case NODE_UNARY:
            printf(" op=%s\n", token_type_name(node->as.unary.operator));
            ast_print(node->as.unary.operand, indent + 1);
            break;
        case NODE_CALL:
            printf("\n");
            print_indent(indent + 1);
            printf("callee:\n");
            ast_print(node->as.call.callee, indent + 2);
            print_indent(indent + 1);
            printf("arguments: %d\n", node->as.call.arg_count);
            for (int i = 0; i < node->as.call.arg_count; i++) {
                ast_print(node->as.call.arguments[i], indent + 2);
            }
            break;
        case NODE_VAR_DECL:
        case NODE_CONST_DECL:
            printf(" %.*s%s\n", 
                   node->as.var_decl.name_length, 
                   node->as.var_decl.name,
                   node->as.var_decl.is_const ? " (const)" : "");
            ast_print(node->as.var_decl.initializer, indent + 1);
            break;
        case NODE_FN_DECL:
            printf(" %.*s(", node->as.fn_decl.name_length, node->as.fn_decl.name);
            for (int i = 0; i < node->as.fn_decl.param_count; i++) {
                if (i > 0) printf(", ");
                printf("%.*s", node->as.fn_decl.param_lengths[i], node->as.fn_decl.parameters[i]);
            }
            printf(")\n");
            ast_print(node->as.fn_decl.body, indent + 1);
            break;
        case NODE_BLOCK:
            printf(" (%d statements)\n", node->as.block.statement_count);
            for (int i = 0; i < node->as.block.statement_count; i++) {
                ast_print(node->as.block.statements[i], indent + 1);
            }
            break;
        case NODE_IF:
            printf("\n");
            print_indent(indent + 1);
            printf("condition:\n");
            ast_print(node->as.if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("then:\n");
            ast_print(node->as.if_stmt.then_branch, indent + 2);
            if (node->as.if_stmt.else_branch) {
                print_indent(indent + 1);
                printf("else:\n");
                ast_print(node->as.if_stmt.else_branch, indent + 2);
            }
            break;
        case NODE_WHILE:
            printf("\n");
            print_indent(indent + 1);
            printf("condition:\n");
            ast_print(node->as.while_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("body:\n");
            ast_print(node->as.while_stmt.body, indent + 2);
            break;
        case NODE_FOR:
            printf(" %.*s in\n", 
                   node->as.for_stmt.iterator_name_length, 
                   node->as.for_stmt.iterator_name);
            print_indent(indent + 1);
            printf("iterable:\n");
            ast_print(node->as.for_stmt.iterable, indent + 2);
            print_indent(indent + 1);
            printf("body:\n");
            ast_print(node->as.for_stmt.body, indent + 2);
            break;
        case NODE_RETURN:
            printf("\n");
            if (node->as.return_stmt.value) {
                ast_print(node->as.return_stmt.value, indent + 1);
            }
            break;
        case NODE_PROGRAM:
            printf(" (%d statements)\n", node->as.program.statement_count);
            for (int i = 0; i < node->as.program.statement_count; i++) {
                ast_print(node->as.program.statements[i], indent + 1);
            }
            break;
        case NODE_EXPR_STMT:
            printf("\n");
            ast_print(node->as.unary.operand, indent + 1);
            break;
        default:
            printf("\n");
            break;
    }
}
