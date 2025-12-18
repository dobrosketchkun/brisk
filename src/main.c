/*
 * Brisk Language - Main Entry Point
 * A minimal interpreted language with native C interop
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"
#include "interp.h"
#include "memory.h"

#define BRISK_VERSION "0.1.0"
#define BRISK_NAME "Brisk"

/* Forward declarations */
static void print_help(const char* program_name);
static void print_version(void);
static void run_file(const char* path);
static void run_repl(void);

int main(int argc, char* argv[]) {
    /* No arguments - run REPL */
    if (argc == 1) {
        run_repl();
        return 0;
    }
    
    /* Parse command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            print_version();
            return 0;
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
            return 1;
        }
        else {
            /* Treat as file path */
            run_file(argv[i]);
            return 0;
        }
    }
    
    return 0;
}

static void print_help(const char* program_name) {
    printf("Usage: %s [options] [file]\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message and exit\n");
    printf("  -v, --version  Show version information and exit\n");
    printf("\n");
    printf("If no file is given, starts an interactive REPL.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s                    # Start REPL\n", program_name);
    printf("  %s script.brisk       # Run a Brisk script\n", program_name);
    printf("  %s --version          # Show version\n", program_name);
}

static void print_version(void) {
    printf("%s version %s\n", BRISK_NAME, BRISK_VERSION);
    printf("A minimal interpreted language with native C interop\n");
}

static void run_file(const char* path) {
    int result = interpret_file(path);
    if (result != 0) {
        exit(result);
    }
}

/* Check if input is incomplete (unclosed braces/parens) */
static bool is_incomplete(const char* input) {
    int braces = 0;
    int parens = 0;
    int brackets = 0;
    bool in_string = false;
    bool escaped = false;
    
    for (const char* p = input; *p; p++) {
        if (escaped) {
            escaped = false;
            continue;
        }
        
        if (*p == '\\') {
            escaped = true;
            continue;
        }
        
        if (*p == '"') {
            in_string = !in_string;
            continue;
        }
        
        if (in_string) continue;
        
        switch (*p) {
            case '{': braces++; break;
            case '}': braces--; break;
            case '(': parens++; break;
            case ')': parens--; break;
            case '[': brackets++; break;
            case ']': brackets--; break;
        }
    }
    
    return braces > 0 || parens > 0 || brackets > 0 || in_string;
}

static void run_repl(void) {
    printf("%s %s - Interactive Mode\n", BRISK_NAME, BRISK_VERSION);
    printf("Type ':help' for commands, ':quit' to exit\n");
    printf("\n");
    
    Interpreter interp;
    interp_init(&interp);
    
    char line[1024];
    char buffer[8192];
    buffer[0] = '\0';
    bool continuing = false;
    
    while (true) {
        if (continuing) {
            printf("... ");
        } else {
            printf("> ");
        }
        fflush(stdout);
        
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break; /* EOF */
        }
        
        /* Remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }
        
        /* Skip empty input when not continuing */
        if (len == 0 && !continuing) {
            continue;
        }
        
        /* Handle REPL commands */
        if (!continuing && line[0] == ':') {
            if (strcmp(line, ":quit") == 0 || strcmp(line, ":exit") == 0) {
                printf("Goodbye!\n");
                break;
            }
            else if (strcmp(line, ":help") == 0) {
                printf("REPL Commands:\n");
                printf("  :help   - Show this help\n");
                printf("  :quit   - Exit the REPL\n");
                printf("  :exit   - Exit the REPL\n");
                printf("  :clear  - Clear the screen\n");
                printf("  :mem    - Show memory usage\n");
                continue;
            }
            else if (strcmp(line, ":clear") == 0) {
                printf("\033[2J\033[H"); /* ANSI escape to clear screen */
                continue;
            }
            else if (strcmp(line, ":mem") == 0) {
                mem_print_stats();
                continue;
            }
            else {
                printf("Unknown command: %s\n", line);
                continue;
            }
        }
        
        /* Accumulate input */
        if (strlen(buffer) + len + 2 < sizeof(buffer)) {
            if (continuing) {
                strcat(buffer, "\n");
            }
            strcat(buffer, line);
        }
        
        /* Check if input is complete */
        if (is_incomplete(buffer)) {
            continuing = true;
            continue;
        }
        
        continuing = false;
        
        /* Parse and execute */
        AstNode* ast = parse(buffer);
        buffer[0] = '\0';
        
        if (ast == NULL) {
            /* Parse error already printed */
            continue;
        }
        
        /* Reset error state */
        interp.had_error = false;
        
        /* Execute */
        exec_program(&interp, ast);
        
        /* If it was an expression statement, print the result */
        if (!interp.had_error && 
            ast->as.program.statement_count == 1 &&
            ast->as.program.statements[0]->type == NODE_EXPR_STMT) {
            
            /* Re-evaluate to get the value (simple approach) */
            AstNode* expr = ast->as.program.statements[0]->as.unary.operand;
            Value result = eval(&interp, expr);
            
            if (!IS_NIL(result)) {
                printf("=> ");
                value_print(result);
                printf("\n");
            }
        }
        
        ast_free_tree(ast);
    }
    
    interp_destroy(&interp);
}
