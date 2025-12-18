/*
 * Brisk Language - Parser Tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/parser.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Running test_%s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("FAILED: %s\n", msg); \
        tests_failed++; \
        return; \
    } \
} while(0)

/* Test literal expressions */
TEST(literal_int) {
    AstNode* ast = parse("42");
    ASSERT(ast != NULL, "AST should not be NULL");
    ASSERT(ast->type == NODE_PROGRAM, "Should be program node");
    ASSERT(ast->as.program.statement_count == 1, "Should have 1 statement");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_EXPR_STMT, "Should be expression statement");
    
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_LITERAL_INT, "Should be int literal");
    ASSERT(expr->as.int_literal.value == 42, "Value should be 42");
    
    ast_free_tree(ast);
}

TEST(literal_float) {
    AstNode* ast = parse("3.14");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_LITERAL_FLOAT, "Should be float literal");
    ASSERT(expr->as.float_literal.value == 3.14, "Value should be 3.14");
    
    ast_free_tree(ast);
}

TEST(literal_string) {
    AstNode* ast = parse("\"hello\"");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_LITERAL_STRING, "Should be string literal");
    ASSERT(strcmp(expr->as.string_literal.value, "hello") == 0, "Value should be 'hello'");
    
    ast_free_tree(ast);
}

TEST(literal_bool) {
    AstNode* ast = parse("true");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_LITERAL_BOOL, "Should be bool literal");
    ASSERT(expr->as.bool_literal.value == true, "Value should be true");
    
    ast_free_tree(ast);
}

TEST(literal_nil) {
    AstNode* ast = parse("nil");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_LITERAL_NIL, "Should be nil literal");
    
    ast_free_tree(ast);
}

/* Test binary expressions */
TEST(binary_add) {
    AstNode* ast = parse("1 + 2");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_BINARY, "Should be binary expression");
    ASSERT(expr->as.binary.operator == TOKEN_PLUS, "Operator should be +");
    ASSERT(expr->as.binary.left->type == NODE_LITERAL_INT, "Left should be int");
    ASSERT(expr->as.binary.right->type == NODE_LITERAL_INT, "Right should be int");
    
    ast_free_tree(ast);
}

/* Test operator precedence */
TEST(precedence) {
    AstNode* ast = parse("1 + 2 * 3");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    
    /* Should be: (1 + (2 * 3)) */
    ASSERT(expr->type == NODE_BINARY, "Should be binary expression");
    ASSERT(expr->as.binary.operator == TOKEN_PLUS, "Root should be +");
    ASSERT(expr->as.binary.right->type == NODE_BINARY, "Right should be binary");
    ASSERT(expr->as.binary.right->as.binary.operator == TOKEN_STAR, "Right should be *");
    
    ast_free_tree(ast);
}

/* Test unary expressions */
TEST(unary_negate) {
    AstNode* ast = parse("-42");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_UNARY, "Should be unary expression");
    ASSERT(expr->as.unary.operator == TOKEN_MINUS, "Operator should be -");
    ASSERT(expr->as.unary.operand->type == NODE_LITERAL_INT, "Operand should be int");
    
    ast_free_tree(ast);
}

TEST(unary_not) {
    AstNode* ast = parse("not true");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_UNARY, "Should be unary expression");
    ASSERT(expr->as.unary.operator == TOKEN_NOT, "Operator should be not");
    
    ast_free_tree(ast);
}

/* Test function calls */
TEST(function_call) {
    AstNode* ast = parse("print(42)");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_CALL, "Should be call expression");
    ASSERT(expr->as.call.arg_count == 1, "Should have 1 argument");
    ASSERT(expr->as.call.callee->type == NODE_IDENTIFIER, "Callee should be identifier");
    
    ast_free_tree(ast);
}

TEST(function_call_multiple_args) {
    AstNode* ast = parse("add(1, 2, 3)");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_CALL, "Should be call expression");
    ASSERT(expr->as.call.arg_count == 3, "Should have 3 arguments");
    
    ast_free_tree(ast);
}

/* Test variable declarations */
TEST(var_decl) {
    AstNode* ast = parse("x := 42");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_VAR_DECL, "Should be var declaration");
    ASSERT(strcmp(stmt->as.var_decl.name, "x") == 0, "Name should be x");
    ASSERT(stmt->as.var_decl.is_const == false, "Should not be const");
    
    ast_free_tree(ast);
}

TEST(const_decl) {
    AstNode* ast = parse("PI :: 3.14");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_CONST_DECL, "Should be const declaration");
    ASSERT(strcmp(stmt->as.var_decl.name, "PI") == 0, "Name should be PI");
    ASSERT(stmt->as.var_decl.is_const == true, "Should be const");
    
    ast_free_tree(ast);
}

/* Test if statements */
TEST(if_statement) {
    AstNode* ast = parse("if x { y }");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_IF, "Should be if statement");
    ASSERT(stmt->as.if_stmt.condition != NULL, "Should have condition");
    ASSERT(stmt->as.if_stmt.then_branch != NULL, "Should have then branch");
    ASSERT(stmt->as.if_stmt.else_branch == NULL, "Should not have else branch");
    
    ast_free_tree(ast);
}

TEST(if_else_statement) {
    AstNode* ast = parse("if x { y } else { z }");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_IF, "Should be if statement");
    ASSERT(stmt->as.if_stmt.else_branch != NULL, "Should have else branch");
    
    ast_free_tree(ast);
}

/* Test while loops */
TEST(while_loop) {
    AstNode* ast = parse("while x { y }");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_WHILE, "Should be while statement");
    ASSERT(stmt->as.while_stmt.condition != NULL, "Should have condition");
    ASSERT(stmt->as.while_stmt.body != NULL, "Should have body");
    
    ast_free_tree(ast);
}

/* Test for loops */
TEST(for_loop) {
    AstNode* ast = parse("for i in 0..10 { x }");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_FOR, "Should be for statement");
    ASSERT(strcmp(stmt->as.for_stmt.iterator_name, "i") == 0, "Iterator should be i");
    ASSERT(stmt->as.for_stmt.iterable->type == NODE_RANGE, "Iterable should be range");
    
    ast_free_tree(ast);
}

/* Test function declarations */
TEST(function_decl) {
    AstNode* ast = parse("fn add(a, b) { a + b }");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_FN_DECL, "Should be function declaration");
    ASSERT(strcmp(stmt->as.fn_decl.name, "add") == 0, "Name should be add");
    ASSERT(stmt->as.fn_decl.param_count == 2, "Should have 2 params");
    
    ast_free_tree(ast);
}

/* Test array literals */
TEST(array_literal) {
    AstNode* ast = parse("[1, 2, 3]");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_ARRAY, "Should be array literal");
    ASSERT(expr->as.array.element_count == 3, "Should have 3 elements");
    
    ast_free_tree(ast);
}

/* Test table literals */
TEST(table_literal) {
    AstNode* ast = parse("{ name: \"test\", value: 42 }");
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    AstNode* expr = stmt->as.unary.operand;
    ASSERT(expr->type == NODE_TABLE, "Should be table literal");
    ASSERT(expr->as.table.count == 2, "Should have 2 entries");
    
    ast_free_tree(ast);
}

/* Test complex nested structures */
TEST(complex_nested) {
    AstNode* ast = parse(
        "fn factorial(n) {\n"
        "    if n <= 1 {\n"
        "        return 1\n"
        "    }\n"
        "    return n * factorial(n - 1)\n"
        "}"
    );
    ASSERT(ast != NULL, "AST should not be NULL");
    
    AstNode* stmt = ast->as.program.statements[0];
    ASSERT(stmt->type == NODE_FN_DECL, "Should be function declaration");
    ASSERT(strcmp(stmt->as.fn_decl.name, "factorial") == 0, "Name should be factorial");
    
    ast_free_tree(ast);
}

/* Test error handling */
TEST(error_recovery) {
    AstNode* ast = parse("x := ");  /* Incomplete */
    ASSERT(ast == NULL, "Should fail on incomplete expression");
}

/* Main test runner */
int main(void) {
    printf("\n=== Brisk Parser Tests ===\n\n");
    
    RUN_TEST(literal_int);
    RUN_TEST(literal_float);
    RUN_TEST(literal_string);
    RUN_TEST(literal_bool);
    RUN_TEST(literal_nil);
    RUN_TEST(binary_add);
    RUN_TEST(precedence);
    RUN_TEST(unary_negate);
    RUN_TEST(unary_not);
    RUN_TEST(function_call);
    RUN_TEST(function_call_multiple_args);
    RUN_TEST(var_decl);
    RUN_TEST(const_decl);
    RUN_TEST(if_statement);
    RUN_TEST(if_else_statement);
    RUN_TEST(while_loop);
    RUN_TEST(for_loop);
    RUN_TEST(function_decl);
    RUN_TEST(array_literal);
    RUN_TEST(table_literal);
    RUN_TEST(complex_nested);
    RUN_TEST(error_recovery);
    
    printf("\n=== Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}
