/*
 * Brisk Language - Tree-Walking Interpreter
 */

#ifndef BRISK_INTERP_H
#define BRISK_INTERP_H

#include "ast.h"
#include "value.h"
#include "env.h"
#include <stdbool.h>

/* Defer stack entry */
typedef struct DeferEntry {
    AstNode* statement;
    struct DeferEntry* next;
} DeferEntry;

/* Interpreter structure */
typedef struct {
    Environment* global;
    Environment* current;
    Value return_value;
    Value last_value;       /* Last expression value (for implicit return) */
    bool returning;
    bool breaking;
    bool continuing;
    bool had_error;
    char error_message[256];
    int error_line;
    DeferEntry* defer_stack;
} Interpreter;

/* Initialize interpreter */
void interp_init(Interpreter* interp);

/* Destroy interpreter */
void interp_destroy(Interpreter* interp);

/* Evaluate an expression */
Value eval(Interpreter* interp, AstNode* node);

/* Execute a statement */
void exec(Interpreter* interp, AstNode* node);

/* Execute a program */
void exec_program(Interpreter* interp, AstNode* program);

/* Main entry point */
int interpret(const char* source);

/* Run from file */
int interpret_file(const char* path);

/* Runtime error */
void runtime_error(Interpreter* interp, int line, const char* format, ...);

#endif /* BRISK_INTERP_H */
