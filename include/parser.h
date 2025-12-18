/*
 * Brisk Language - Parser
 */

#ifndef BRISK_PARSER_H
#define BRISK_PARSER_H

#include "lexer.h"
#include "ast.h"
#include <stdbool.h>

/* Parser structure */
typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

/* Initialize parser */
void parser_init(Parser* parser, Lexer* lexer);

/* Parse a complete program */
AstNode* parse_program(Parser* parser);

/* Main entry point - parse source code to AST */
AstNode* parse(const char* source);

#endif /* BRISK_PARSER_H */
