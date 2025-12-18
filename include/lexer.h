/*
 * Brisk Language - Lexer
 */

#ifndef BRISK_LEXER_H
#define BRISK_LEXER_H

#include "token.h"

/* Lexer structure */
typedef struct {
    const char* source;     /* Source code */
    const char* start;      /* Start of current token */
    const char* current;    /* Current character */
    int line;               /* Current line number */
    int column;             /* Current column number */
    int start_column;       /* Column at start of token */
} Lexer;

/* Initialize lexer with source code */
void lexer_init(Lexer* lexer, const char* source);

/* Get next token */
Token lexer_next_token(Lexer* lexer);

/* Peek at current token without consuming */
Token lexer_peek_token(Lexer* lexer);

#endif /* BRISK_LEXER_H */
