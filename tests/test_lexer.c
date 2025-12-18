/*
 * Brisk Language - Lexer Tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/lexer.h"

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

#define ASSERT_TOKEN(lexer, expected_type) do { \
    Token t = lexer_next_token(&lexer); \
    if (t.type != expected_type) { \
        printf("FAILED: Expected %s, got %s\n", \
               token_type_name(expected_type), token_type_name(t.type)); \
        tests_failed++; \
        return; \
    } \
} while(0)

/* Test empty input */
TEST(empty_input) {
    Lexer lexer;
    lexer_init(&lexer, "");
    Token token = lexer_next_token(&lexer);
    ASSERT(token.type == TOKEN_EOF, "Empty input should give EOF");
}

/* Test single tokens */
TEST(single_tokens) {
    Lexer lexer;
    
    lexer_init(&lexer, "(");
    ASSERT_TOKEN(lexer, TOKEN_LPAREN);
    
    lexer_init(&lexer, ")");
    ASSERT_TOKEN(lexer, TOKEN_RPAREN);
    
    lexer_init(&lexer, "{");
    ASSERT_TOKEN(lexer, TOKEN_LBRACE);
    
    lexer_init(&lexer, "}");
    ASSERT_TOKEN(lexer, TOKEN_RBRACE);
    
    lexer_init(&lexer, "[");
    ASSERT_TOKEN(lexer, TOKEN_LBRACKET);
    
    lexer_init(&lexer, "]");
    ASSERT_TOKEN(lexer, TOKEN_RBRACKET);
    
    lexer_init(&lexer, ",");
    ASSERT_TOKEN(lexer, TOKEN_COMMA);
    
    lexer_init(&lexer, ".");
    ASSERT_TOKEN(lexer, TOKEN_DOT);
    
    lexer_init(&lexer, "+");
    ASSERT_TOKEN(lexer, TOKEN_PLUS);
    
    lexer_init(&lexer, "-");
    ASSERT_TOKEN(lexer, TOKEN_MINUS);
    
    lexer_init(&lexer, "*");
    ASSERT_TOKEN(lexer, TOKEN_STAR);
    
    lexer_init(&lexer, "/");
    ASSERT_TOKEN(lexer, TOKEN_SLASH);
    
    lexer_init(&lexer, "%");
    ASSERT_TOKEN(lexer, TOKEN_PERCENT);
    
    lexer_init(&lexer, "&");
    ASSERT_TOKEN(lexer, TOKEN_AMPERSAND);
    
    lexer_init(&lexer, "@");
    ASSERT_TOKEN(lexer, TOKEN_AT);
}

/* Test multi-char operators */
TEST(operators) {
    Lexer lexer;
    
    lexer_init(&lexer, "==");
    ASSERT_TOKEN(lexer, TOKEN_EQEQ);
    
    lexer_init(&lexer, "!=");
    ASSERT_TOKEN(lexer, TOKEN_NEQ);
    
    lexer_init(&lexer, "<=");
    ASSERT_TOKEN(lexer, TOKEN_LTE);
    
    lexer_init(&lexer, ">=");
    ASSERT_TOKEN(lexer, TOKEN_GTE);
    
    lexer_init(&lexer, "::");
    ASSERT_TOKEN(lexer, TOKEN_COLONCOLON);
    
    lexer_init(&lexer, ":=");
    ASSERT_TOKEN(lexer, TOKEN_COLONEQ);
    
    lexer_init(&lexer, "..");
    ASSERT_TOKEN(lexer, TOKEN_DOTDOT);
    
    lexer_init(&lexer, "=>");
    ASSERT_TOKEN(lexer, TOKEN_ARROW);
    
    lexer_init(&lexer, "->");
    ASSERT_TOKEN(lexer, TOKEN_RARROW);
}

/* Test all keywords */
TEST(keywords) {
    Lexer lexer;
    
    lexer_init(&lexer, "fn");
    ASSERT_TOKEN(lexer, TOKEN_FN);
    
    lexer_init(&lexer, "if");
    ASSERT_TOKEN(lexer, TOKEN_IF);
    
    lexer_init(&lexer, "elif");
    ASSERT_TOKEN(lexer, TOKEN_ELIF);
    
    lexer_init(&lexer, "else");
    ASSERT_TOKEN(lexer, TOKEN_ELSE);
    
    lexer_init(&lexer, "while");
    ASSERT_TOKEN(lexer, TOKEN_WHILE);
    
    lexer_init(&lexer, "for");
    ASSERT_TOKEN(lexer, TOKEN_FOR);
    
    lexer_init(&lexer, "in");
    ASSERT_TOKEN(lexer, TOKEN_IN);
    
    lexer_init(&lexer, "return");
    ASSERT_TOKEN(lexer, TOKEN_RETURN);
    
    lexer_init(&lexer, "break");
    ASSERT_TOKEN(lexer, TOKEN_BREAK);
    
    lexer_init(&lexer, "continue");
    ASSERT_TOKEN(lexer, TOKEN_CONTINUE);
    
    lexer_init(&lexer, "match");
    ASSERT_TOKEN(lexer, TOKEN_MATCH);
    
    lexer_init(&lexer, "defer");
    ASSERT_TOKEN(lexer, TOKEN_DEFER);
    
    lexer_init(&lexer, "and");
    ASSERT_TOKEN(lexer, TOKEN_AND);
    
    lexer_init(&lexer, "or");
    ASSERT_TOKEN(lexer, TOKEN_OR);
    
    lexer_init(&lexer, "not");
    ASSERT_TOKEN(lexer, TOKEN_NOT);
    
    lexer_init(&lexer, "true");
    ASSERT_TOKEN(lexer, TOKEN_TRUE);
    
    lexer_init(&lexer, "false");
    ASSERT_TOKEN(lexer, TOKEN_FALSE);
    
    lexer_init(&lexer, "nil");
    ASSERT_TOKEN(lexer, TOKEN_NIL);
}

/* Test integers */
TEST(integers) {
    Lexer lexer;
    
    lexer_init(&lexer, "0");
    Token t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_INT, "0 should be INT");
    ASSERT(t.length == 1, "Length should be 1");
    
    lexer_init(&lexer, "123");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_INT, "123 should be INT");
    ASSERT(t.length == 3, "Length should be 3");
    
    lexer_init(&lexer, "1_000_000");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_INT, "1_000_000 should be INT");
}

/* Test floats */
TEST(floats) {
    Lexer lexer;
    
    lexer_init(&lexer, "3.14");
    Token t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_FLOAT, "3.14 should be FLOAT");
    
    lexer_init(&lexer, "0.5");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_FLOAT, "0.5 should be FLOAT");
    
    lexer_init(&lexer, "123.456");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_FLOAT, "123.456 should be FLOAT");
}

/* Test strings */
TEST(strings) {
    Lexer lexer;
    
    lexer_init(&lexer, "\"hello\"");
    Token t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_STRING, "\"hello\" should be STRING");
    ASSERT(t.length == 7, "String with quotes should be length 7");
    
    lexer_init(&lexer, "\"hello\\nworld\"");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_STRING, "String with escape should be STRING");
    
    lexer_init(&lexer, "\"test\\\"quote\"");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_STRING, "String with escaped quote should be STRING");
}

/* Test comments */
TEST(comments) {
    Lexer lexer;
    
    lexer_init(&lexer, "# this is a comment\n42");
    Token t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_NEWLINE, "After comment should be NEWLINE");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_INT, "After newline should be INT");
    
    lexer_init(&lexer, "42 # comment");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_INT, "Before comment should be INT");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_EOF, "After comment at end should be EOF");
}

/* Test identifiers */
TEST(identifiers) {
    Lexer lexer;
    
    lexer_init(&lexer, "foo");
    Token t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_IDENTIFIER, "foo should be IDENTIFIER");
    ASSERT(t.length == 3, "foo length should be 3");
    
    lexer_init(&lexer, "_private");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_IDENTIFIER, "_private should be IDENTIFIER");
    
    lexer_init(&lexer, "camelCase");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_IDENTIFIER, "camelCase should be IDENTIFIER");
    
    lexer_init(&lexer, "snake_case");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_IDENTIFIER, "snake_case should be IDENTIFIER");
    
    lexer_init(&lexer, "var123");
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_IDENTIFIER, "var123 should be IDENTIFIER");
}

/* Test multiline input */
TEST(multiline) {
    Lexer lexer;
    
    lexer_init(&lexer, "x := 42\ny := 10");
    
    Token t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_IDENTIFIER, "x should be IDENTIFIER");
    ASSERT(t.line == 1, "x should be on line 1");
    
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_COLONEQ, ":= expected");
    
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_INT, "42 should be INT");
    
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_NEWLINE, "Newline expected");
    
    t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_IDENTIFIER, "y should be IDENTIFIER");
    ASSERT(t.line == 2, "y should be on line 2");
}

/* Test complex expression */
TEST(complex_expression) {
    Lexer lexer;
    lexer_init(&lexer, "fn add(a, b) { a + b }");
    
    ASSERT_TOKEN(lexer, TOKEN_FN);
    ASSERT_TOKEN(lexer, TOKEN_IDENTIFIER);  /* add */
    ASSERT_TOKEN(lexer, TOKEN_LPAREN);
    ASSERT_TOKEN(lexer, TOKEN_IDENTIFIER);  /* a */
    ASSERT_TOKEN(lexer, TOKEN_COMMA);
    ASSERT_TOKEN(lexer, TOKEN_IDENTIFIER);  /* b */
    ASSERT_TOKEN(lexer, TOKEN_RPAREN);
    ASSERT_TOKEN(lexer, TOKEN_LBRACE);
    ASSERT_TOKEN(lexer, TOKEN_IDENTIFIER);  /* a */
    ASSERT_TOKEN(lexer, TOKEN_PLUS);
    ASSERT_TOKEN(lexer, TOKEN_IDENTIFIER);  /* b */
    ASSERT_TOKEN(lexer, TOKEN_RBRACE);
    ASSERT_TOKEN(lexer, TOKEN_EOF);
}

/* Test error cases */
TEST(errors) {
    Lexer lexer;
    
    lexer_init(&lexer, "\"unterminated");
    Token t = lexer_next_token(&lexer);
    ASSERT(t.type == TOKEN_ERROR, "Unterminated string should be ERROR");
}

/* Main test runner */
int main(void) {
    printf("\n=== Brisk Lexer Tests ===\n\n");
    
    RUN_TEST(empty_input);
    RUN_TEST(single_tokens);
    RUN_TEST(operators);
    RUN_TEST(keywords);
    RUN_TEST(integers);
    RUN_TEST(floats);
    RUN_TEST(strings);
    RUN_TEST(comments);
    RUN_TEST(identifiers);
    RUN_TEST(multiline);
    RUN_TEST(complex_expression);
    RUN_TEST(errors);
    
    printf("\n=== Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}
