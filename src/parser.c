/*
 * Brisk Language - Parser Implementation
 * Pratt parser (recursive descent with precedence climbing)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"

/* Precedence levels */
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,    /* = */
    PREC_OR,            /* or */
    PREC_AND,           /* and */
    PREC_EQUALITY,      /* == != */
    PREC_COMPARISON,    /* < > <= >= */
    PREC_RANGE,         /* .. */
    PREC_TERM,          /* + - */
    PREC_FACTOR,        /* * / % */
    PREC_UNARY,         /* - not & */
    PREC_CALL,          /* . () [] */
    PREC_PRIMARY
} Precedence;

/* Forward declarations */
typedef AstNode* (*ParsePrefixFn)(Parser* parser);
typedef AstNode* (*ParseInfixFn)(Parser* parser, AstNode* left);

typedef struct {
    ParsePrefixFn prefix;
    ParseInfixFn infix;
    Precedence precedence;
} ParseRule;

static AstNode* parse_expression(Parser* parser);
static AstNode* parse_precedence(Parser* parser, Precedence precedence);
static AstNode* parse_statement(Parser* parser);
static AstNode* parse_block(Parser* parser);
static ParseRule* get_rule(TokenType type);

/* Forward declarations for parse functions */
static AstNode* parse_number(Parser* parser);
static AstNode* parse_string(Parser* parser);
static AstNode* parse_literal(Parser* parser);
static AstNode* parse_identifier(Parser* parser);
static AstNode* parse_grouping(Parser* parser);
static AstNode* parse_unary(Parser* parser);
static AstNode* parse_binary(Parser* parser, AstNode* left);
static AstNode* parse_range(Parser* parser, AstNode* left);
static AstNode* parse_call(Parser* parser, AstNode* left);
static AstNode* parse_index(Parser* parser, AstNode* left);
static AstNode* parse_field(Parser* parser, AstNode* left);
static AstNode* parse_array(Parser* parser);
static AstNode* parse_table(Parser* parser);
static AstNode* parse_fn_expr(Parser* parser);
static AstNode* parse_address_of(Parser* parser);

/* Error handling */
static void error_at(Parser* parser, Token* token, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;
    
    fprintf(stderr, "[line %d, col %d] Error", token->line, token->column);
    
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type != TOKEN_ERROR) {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    
    fprintf(stderr, ": %s\n", message);
}

static void error(Parser* parser, const char* message) {
    error_at(parser, &parser->previous, message);
}

static void error_at_current(Parser* parser, const char* message) {
    error_at(parser, &parser->current, message);
}

/* Token handling */
static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    for (;;) {
        parser->current = lexer_next_token(parser->lexer);
        
        /* Skip newlines in most contexts */
        if (parser->current.type == TOKEN_NEWLINE) {
            continue;
        }
        
        if (parser->current.type != TOKEN_ERROR) break;
        
        error_at_current(parser, parser->current.start);
    }
}

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    
    error_at_current(parser, message);
}

/* Skip newlines (currently handled in advance()) */
#if 0
static void skip_newlines(Parser* parser) {
    while (parser->current.type == TOKEN_NEWLINE) {
        parser->current = lexer_next_token(parser->lexer);
    }
}
#endif

/* Error recovery */
static void synchronize(Parser* parser) {
    parser->panic_mode = false;
    
    while (parser->current.type != TOKEN_EOF) {
        if (parser->previous.type == TOKEN_NEWLINE) return;
        
        switch (parser->current.type) {
            case TOKEN_FN:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_FOR:
            case TOKEN_RETURN:
            case TOKEN_MATCH:
            case TOKEN_DEFER:
                return;
            default:
                break;
        }
        
        advance(parser);
    }
}

/* Parse rules table */
static ParseRule rules[TOKEN_COUNT];

static void init_rules(void) {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    
    /* Initialize all to NULL/NONE */
    for (int i = 0; i < TOKEN_COUNT; i++) {
        rules[i] = (ParseRule){NULL, NULL, PREC_NONE};
    }
    
    /* Literals */
    rules[TOKEN_INT] = (ParseRule){parse_number, NULL, PREC_NONE};
    rules[TOKEN_FLOAT] = (ParseRule){parse_number, NULL, PREC_NONE};
    rules[TOKEN_STRING] = (ParseRule){parse_string, NULL, PREC_NONE};
    rules[TOKEN_TRUE] = (ParseRule){parse_literal, NULL, PREC_NONE};
    rules[TOKEN_FALSE] = (ParseRule){parse_literal, NULL, PREC_NONE};
    rules[TOKEN_NIL] = (ParseRule){parse_literal, NULL, PREC_NONE};
    
    /* Identifier */
    rules[TOKEN_IDENTIFIER] = (ParseRule){parse_identifier, NULL, PREC_NONE};
    
    /* Grouping and collections */
    rules[TOKEN_LPAREN] = (ParseRule){parse_grouping, parse_call, PREC_CALL};
    rules[TOKEN_LBRACKET] = (ParseRule){parse_array, parse_index, PREC_CALL};
    rules[TOKEN_LBRACE] = (ParseRule){parse_table, NULL, PREC_NONE};
    
    /* Unary */
    rules[TOKEN_MINUS] = (ParseRule){parse_unary, parse_binary, PREC_TERM};
    rules[TOKEN_NOT] = (ParseRule){parse_unary, NULL, PREC_NONE};
    rules[TOKEN_BANG] = (ParseRule){parse_unary, NULL, PREC_NONE};
    rules[TOKEN_AMPERSAND] = (ParseRule){parse_address_of, NULL, PREC_NONE};
    
    /* Binary operators */
    rules[TOKEN_PLUS] = (ParseRule){NULL, parse_binary, PREC_TERM};
    rules[TOKEN_STAR] = (ParseRule){NULL, parse_binary, PREC_FACTOR};
    rules[TOKEN_SLASH] = (ParseRule){NULL, parse_binary, PREC_FACTOR};
    rules[TOKEN_PERCENT] = (ParseRule){NULL, parse_binary, PREC_FACTOR};
    
    /* Comparison */
    rules[TOKEN_EQEQ] = (ParseRule){NULL, parse_binary, PREC_EQUALITY};
    rules[TOKEN_NEQ] = (ParseRule){NULL, parse_binary, PREC_EQUALITY};
    rules[TOKEN_LT] = (ParseRule){NULL, parse_binary, PREC_COMPARISON};
    rules[TOKEN_GT] = (ParseRule){NULL, parse_binary, PREC_COMPARISON};
    rules[TOKEN_LTE] = (ParseRule){NULL, parse_binary, PREC_COMPARISON};
    rules[TOKEN_GTE] = (ParseRule){NULL, parse_binary, PREC_COMPARISON};
    
    /* Logical */
    rules[TOKEN_AND] = (ParseRule){NULL, parse_binary, PREC_AND};
    rules[TOKEN_OR] = (ParseRule){NULL, parse_binary, PREC_OR};
    
    /* Range */
    rules[TOKEN_DOTDOT] = (ParseRule){NULL, parse_range, PREC_RANGE};
    
    /* Field access */
    rules[TOKEN_DOT] = (ParseRule){NULL, parse_field, PREC_CALL};
    
    /* Function expression */
    rules[TOKEN_FN] = (ParseRule){parse_fn_expr, NULL, PREC_NONE};
}

static ParseRule* get_rule(TokenType type) {
    init_rules();
    return &rules[type];
}

/* Parser initialization */
void parser_init(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->had_error = false;
    parser->panic_mode = false;
    
    init_rules();
    advance(parser);
}

/* Parse number literal */
static AstNode* parse_number(Parser* parser) {
    Token token = parser->previous;
    
    if (token.type == TOKEN_INT) {
        /* Parse integer */
        char* buf = malloc(token.length + 1);
        int j = 0;
        for (int i = 0; i < token.length; i++) {
            if (token.start[i] != '_') {
                buf[j++] = token.start[i];
            }
        }
        buf[j] = '\0';
        
        int64_t value = strtoll(buf, NULL, 0); /* 0 = auto-detect hex/octal/decimal */
        free(buf);
        return ast_int_literal(value, token.line, token.column);
    } else {
        /* Parse float */
        char* buf = malloc(token.length + 1);
        int j = 0;
        for (int i = 0; i < token.length; i++) {
            if (token.start[i] != '_') {
                buf[j++] = token.start[i];
            }
        }
        buf[j] = '\0';
        
        double value = strtod(buf, NULL);
        free(buf);
        return ast_float_literal(value, token.line, token.column);
    }
}

/* Parse string literal */
static AstNode* parse_string(Parser* parser) {
    Token token = parser->previous;
    
    /* Skip opening quote and closing quote */
    const char* start = token.start + 1;
    int length = token.length - 2;
    
    /* Process escape sequences */
    char* buf = malloc(length + 1);
    int j = 0;
    for (int i = 0; i < length; i++) {
        if (start[i] == '\\' && i + 1 < length) {
            i++;
            switch (start[i]) {
                case 'n': buf[j++] = '\n'; break;
                case 't': buf[j++] = '\t'; break;
                case 'r': buf[j++] = '\r'; break;
                case '\\': buf[j++] = '\\'; break;
                case '"': buf[j++] = '"'; break;
                case '0': buf[j++] = '\0'; break;
                default: buf[j++] = start[i]; break;
            }
        } else {
            buf[j++] = start[i];
        }
    }
    buf[j] = '\0';
    
    AstNode* node = ast_string_literal(buf, j, token.line, token.column);
    free(buf);
    return node;
}

/* Parse true/false/nil */
static AstNode* parse_literal(Parser* parser) {
    Token token = parser->previous;
    
    switch (token.type) {
        case TOKEN_TRUE:
            return ast_bool_literal(true, token.line, token.column);
        case TOKEN_FALSE:
            return ast_bool_literal(false, token.line, token.column);
        case TOKEN_NIL:
            return ast_nil_literal(token.line, token.column);
        default:
            return NULL;
    }
}

/* Parse identifier */
static AstNode* parse_identifier(Parser* parser) {
    Token token = parser->previous;
    return ast_identifier(token.start, token.length, token.line, token.column);
}

/* Parse grouping (parentheses) */
static AstNode* parse_grouping(Parser* parser) {
    AstNode* expr = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
    return expr;
}

/* Parse unary expression */
static AstNode* parse_unary(Parser* parser) {
    Token op = parser->previous;
    
    AstNode* operand = parse_precedence(parser, PREC_UNARY);
    return ast_unary(op.type, operand, op.line, op.column);
}

/* Parse address-of operator */
static AstNode* parse_address_of(Parser* parser) {
    Token op = parser->previous;
    
    AstNode* operand = parse_precedence(parser, PREC_UNARY);
    return ast_address_of(operand, op.line, op.column);
}

/* Parse binary expression */
static AstNode* parse_binary(Parser* parser, AstNode* left) {
    Token op = parser->previous;
    
    ParseRule* rule = get_rule(op.type);
    AstNode* right = parse_precedence(parser, (Precedence)(rule->precedence + 1));
    
    return ast_binary(op.type, left, right, op.line, op.column);
}

/* Parse range expression */
static AstNode* parse_range(Parser* parser, AstNode* left) {
    Token op = parser->previous;
    
    AstNode* right = parse_precedence(parser, PREC_RANGE + 1);
    return ast_range(left, right, op.line, op.column);
}

/* Parse function call */
static AstNode* parse_call(Parser* parser, AstNode* left) {
    Token token = parser->previous;
    
    /* Parse arguments */
    AstNode** args = NULL;
    int arg_count = 0;
    int arg_capacity = 0;
    
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            if (arg_count >= arg_capacity) {
                arg_capacity = arg_capacity < 8 ? 8 : arg_capacity * 2;
                args = realloc(args, sizeof(AstNode*) * arg_capacity);
            }
            args[arg_count++] = parse_expression(parser);
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RPAREN, "Expected ')' after arguments");
    
    return ast_call(left, args, arg_count, token.line, token.column);
}

/* Parse index expression */
static AstNode* parse_index(Parser* parser, AstNode* left) {
    Token token = parser->previous;
    
    AstNode* index = parse_expression(parser);
    consume(parser, TOKEN_RBRACKET, "Expected ']' after index");
    
    return ast_index(left, index, token.line, token.column);
}

/* Parse field access */
static AstNode* parse_field(Parser* parser, AstNode* left) {
    Token token = parser->previous;
    
    consume(parser, TOKEN_IDENTIFIER, "Expected field name after '.'");
    Token field = parser->previous;
    
    return ast_field(left, field.start, field.length, token.line, token.column);
}

/* Parse array literal */
static AstNode* parse_array(Parser* parser) {
    Token token = parser->previous;
    
    AstNode** elements = NULL;
    int count = 0;
    int capacity = 0;
    
    if (!check(parser, TOKEN_RBRACKET)) {
        do {
            /* Allow trailing comma */
            if (check(parser, TOKEN_RBRACKET)) break;
            
            if (count >= capacity) {
                capacity = capacity < 8 ? 8 : capacity * 2;
                elements = realloc(elements, sizeof(AstNode*) * capacity);
            }
            elements[count++] = parse_expression(parser);
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RBRACKET, "Expected ']' after array elements");
    
    return ast_array(elements, count, token.line, token.column);
}

/* Parse table literal */
static AstNode* parse_table(Parser* parser) {
    Token token = parser->previous;
    
    char** keys = NULL;
    int* key_lengths = NULL;
    AstNode** values = NULL;
    int count = 0;
    int capacity = 0;
    
    if (!check(parser, TOKEN_RBRACE)) {
        do {
            /* Allow trailing comma */
            if (check(parser, TOKEN_RBRACE)) break;
            
            if (count >= capacity) {
                capacity = capacity < 8 ? 8 : capacity * 2;
                keys = realloc(keys, sizeof(char*) * capacity);
                key_lengths = realloc(key_lengths, sizeof(int) * capacity);
                values = realloc(values, sizeof(AstNode*) * capacity);
            }
            
            /* Parse key */
            consume(parser, TOKEN_IDENTIFIER, "Expected key name");
            Token key = parser->previous;
            keys[count] = malloc(key.length + 1);
            memcpy(keys[count], key.start, key.length);
            keys[count][key.length] = '\0';
            key_lengths[count] = key.length;
            
            consume(parser, TOKEN_COLON, "Expected ':' after key");
            
            /* Parse value */
            values[count] = parse_expression(parser);
            count++;
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RBRACE, "Expected '}' after table entries");
    
    return ast_table(keys, key_lengths, values, count, token.line, token.column);
}

/* Parse function expression (lambda or named) */
static AstNode* parse_fn_expr(Parser* parser) {
    Token token = parser->previous;
    
    /* Check for function name */
    char* name = NULL;
    int name_len = 0;
    if (check(parser, TOKEN_IDENTIFIER)) {
        advance(parser);
        name = malloc(parser->previous.length + 1);
        memcpy(name, parser->previous.start, parser->previous.length);
        name[parser->previous.length] = '\0';
        name_len = parser->previous.length;
    }
    
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'fn'");
    
    /* Parse parameters */
    char** params = NULL;
    int* param_lens = NULL;
    int param_count = 0;
    int param_capacity = 0;
    
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            if (param_count >= param_capacity) {
                param_capacity = param_capacity < 8 ? 8 : param_capacity * 2;
                params = realloc(params, sizeof(char*) * param_capacity);
                param_lens = realloc(param_lens, sizeof(int) * param_capacity);
            }
            
            consume(parser, TOKEN_IDENTIFIER, "Expected parameter name");
            Token param = parser->previous;
            params[param_count] = malloc(param.length + 1);
            memcpy(params[param_count], param.start, param.length);
            params[param_count][param.length] = '\0';
            param_lens[param_count] = param.length;
            param_count++;
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");
    
    /* Optional return type annotation (skip for now) */
    if (match(parser, TOKEN_RARROW)) {
        /* Skip type annotation */
        advance(parser);
    }
    
    /* Parse body */
    consume(parser, TOKEN_LBRACE, "Expected '{' before function body");
    AstNode* body = parse_block(parser);
    
    if (name) {
        AstNode* node = ast_fn_decl(name, name_len, params, param_lens, param_count, body, token.line, token.column);
        free(name);
        return node;
    } else {
        return ast_lambda(params, param_lens, param_count, body, token.line, token.column);
    }
}

/* Parse with precedence */
static AstNode* parse_precedence(Parser* parser, Precedence precedence) {
    advance(parser);
    
    ParsePrefixFn prefix_rule = get_rule(parser->previous.type)->prefix;
    if (prefix_rule == NULL) {
        error(parser, "Expected expression");
        return NULL;
    }
    
    AstNode* left = prefix_rule(parser);
    
    while (precedence <= get_rule(parser->current.type)->precedence) {
        advance(parser);
        ParseInfixFn infix_rule = get_rule(parser->previous.type)->infix;
        left = infix_rule(parser, left);
    }
    
    return left;
}

/* Parse expression */
static AstNode* parse_expression(Parser* parser) {
    return parse_precedence(parser, PREC_ASSIGNMENT);
}

/* Parse block */
static AstNode* parse_block(Parser* parser) {
    Token token = parser->previous;
    
    AstNode** stmts = NULL;
    int count = 0;
    int capacity = 0;
    
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        if (count >= capacity) {
            capacity = capacity < 8 ? 8 : capacity * 2;
            stmts = realloc(stmts, sizeof(AstNode*) * capacity);
        }
        
        AstNode* stmt = parse_statement(parser);
        if (stmt) {
            stmts[count++] = stmt;
        }
        
        if (parser->panic_mode) synchronize(parser);
    }
    
    consume(parser, TOKEN_RBRACE, "Expected '}' after block");
    
    return ast_block(stmts, count, token.line, token.column);
}

/* Parse if statement */
static AstNode* parse_if(Parser* parser) {
    Token token = parser->previous;
    
    AstNode* condition = parse_expression(parser);
    consume(parser, TOKEN_LBRACE, "Expected '{' after if condition");
    AstNode* then_branch = parse_block(parser);
    
    AstNode* else_branch = NULL;
    if (match(parser, TOKEN_ELIF)) {
        /* elif is just nested if */
        else_branch = parse_if(parser);
    } else if (match(parser, TOKEN_ELSE)) {
        consume(parser, TOKEN_LBRACE, "Expected '{' after else");
        else_branch = parse_block(parser);
    }
    
    return ast_if(condition, then_branch, else_branch, token.line, token.column);
}

/* Parse while statement */
static AstNode* parse_while(Parser* parser) {
    Token token = parser->previous;
    
    AstNode* condition = parse_expression(parser);
    consume(parser, TOKEN_LBRACE, "Expected '{' after while condition");
    AstNode* body = parse_block(parser);
    
    return ast_while(condition, body, token.line, token.column);
}

/* Parse for statement */
static AstNode* parse_for(Parser* parser) {
    Token token = parser->previous;
    
    consume(parser, TOKEN_IDENTIFIER, "Expected iterator name");
    Token iter = parser->previous;
    
    consume(parser, TOKEN_IN, "Expected 'in' after iterator");
    
    AstNode* iterable = parse_expression(parser);
    
    consume(parser, TOKEN_LBRACE, "Expected '{' after for iterable");
    AstNode* body = parse_block(parser);
    
    return ast_for(iter.start, iter.length, iterable, body, token.line, token.column);
}

/* Parse return statement */
static AstNode* parse_return(Parser* parser) {
    Token token = parser->previous;
    
    AstNode* value = NULL;
    if (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_NEWLINE) && !check(parser, TOKEN_EOF)) {
        value = parse_expression(parser);
    }
    
    return ast_return(value, token.line, token.column);
}

/* Parse match statement */
static AstNode* parse_match(Parser* parser) {
    Token token = parser->previous;
    
    AstNode* value = parse_expression(parser);
    consume(parser, TOKEN_LBRACE, "Expected '{' after match value");
    
    AstNode** patterns = NULL;
    AstNode** bodies = NULL;
    int arm_count = 0;
    int capacity = 0;
    
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        if (arm_count >= capacity) {
            capacity = capacity < 8 ? 8 : capacity * 2;
            patterns = realloc(patterns, sizeof(AstNode*) * capacity);
            bodies = realloc(bodies, sizeof(AstNode*) * capacity);
        }
        
        /* Parse pattern */
        if (match(parser, TOKEN_UNDERSCORE)) {
            patterns[arm_count] = ast_identifier("_", 1, parser->previous.line, parser->previous.column);
        } else {
            patterns[arm_count] = parse_expression(parser);
        }
        
        consume(parser, TOKEN_ARROW, "Expected '=>' after pattern");
        
        /* Parse body */
        if (check(parser, TOKEN_LBRACE)) {
            advance(parser);
            bodies[arm_count] = parse_block(parser);
        } else {
            bodies[arm_count] = parse_expression(parser);
        }
        
        arm_count++;
        
        /* Optional comma */
        match(parser, TOKEN_COMMA);
    }
    
    consume(parser, TOKEN_RBRACE, "Expected '}' after match arms");
    
    return ast_match(value, patterns, bodies, arm_count, token.line, token.column);
}

/* Parse defer statement */
static AstNode* parse_defer(Parser* parser) {
    Token token = parser->previous;
    
    AstNode* stmt = parse_statement(parser);
    return ast_defer(stmt, token.line, token.column);
}

/* Parse import directive */
static AstNode* parse_import(Parser* parser) {
    Token token = parser->previous;
    
    consume(parser, TOKEN_STRING, "Expected import path string");
    Token path = parser->previous;
    
    /* Skip quotes */
    return ast_import(path.start + 1, path.length - 2, token.line, token.column);
}

/* Parse @c block */
static AstNode* parse_c_block(Parser* parser) {
    Token token = parser->previous;
    
    consume(parser, TOKEN_LBRACE, "Expected '{' after @c");
    
    /* Capture everything until matching brace */
    const char* start = parser->lexer->current;
    int brace_count = 1;
    int length = 0;
    
    while (brace_count > 0 && !check(parser, TOKEN_EOF)) {
        char c = *parser->lexer->current;
        if (c == '{') brace_count++;
        if (c == '}') brace_count--;
        if (brace_count > 0) {
            parser->lexer->current++;
            length++;
        }
    }
    
    /* Advance past closing brace */
    advance(parser);
    
    return ast_c_block(start, length, token.line, token.column);
}

/* Parse statement */
static AstNode* parse_statement(Parser* parser) {
    /* Skip any leading newlines */
    while (parser->current.type == TOKEN_NEWLINE) {
        parser->current = lexer_next_token(parser->lexer);
    }
    
    if (check(parser, TOKEN_EOF)) return NULL;
    
    /* Variable/constant declaration - peek ahead to check */
    if (check(parser, TOKEN_IDENTIFIER)) {
        /* Save position to potentially backtrack */
        Token name = parser->current;
        
        /* Peek at next token to decide */
        Token peeked = lexer_peek_token(parser->lexer);
        
        if (peeked.type == TOKEN_COLONEQ) {
            /* Variable declaration: name := expr */
            advance(parser);  /* consume identifier */
            advance(parser);  /* consume := */
            AstNode* init = parse_expression(parser);
            return ast_var_decl(name.start, name.length, init, false, name.line, name.column);
        } else if (peeked.type == TOKEN_COLONCOLON) {
            /* Constant declaration: name :: expr */
            advance(parser);  /* consume identifier */
            advance(parser);  /* consume :: */
            AstNode* init = parse_expression(parser);
            return ast_var_decl(name.start, name.length, init, true, name.line, name.column);
        }
        /* Otherwise fall through to expression parsing */
    }
    
    /* Control flow */
    if (match(parser, TOKEN_IF)) {
        return parse_if(parser);
    }
    
    if (match(parser, TOKEN_WHILE)) {
        return parse_while(parser);
    }
    
    if (match(parser, TOKEN_FOR)) {
        return parse_for(parser);
    }
    
    if (match(parser, TOKEN_RETURN)) {
        return parse_return(parser);
    }
    
    if (match(parser, TOKEN_BREAK)) {
        return ast_break(parser->previous.line, parser->previous.column);
    }
    
    if (match(parser, TOKEN_CONTINUE)) {
        return ast_continue(parser->previous.line, parser->previous.column);
    }
    
    if (match(parser, TOKEN_MATCH)) {
        return parse_match(parser);
    }
    
    if (match(parser, TOKEN_DEFER)) {
        return parse_defer(parser);
    }
    
    /* Function declaration or anonymous function */
    if (match(parser, TOKEN_FN)) {
        AstNode* fn = parse_fn_expr(parser);
        /* If it's a named function, it's a declaration statement */
        /* If it's anonymous, wrap in expression statement */
        if (fn && fn->type == NODE_LAMBDA) {
            return ast_expr_stmt(fn, fn->line, fn->column);
        }
        return fn;
    }
    
    /* Directives */
    if (match(parser, TOKEN_AT)) {
        if (check(parser, TOKEN_IDENTIFIER)) {
            Token directive = parser->current;
            if (strncmp(directive.start, "import", 6) == 0 && directive.length == 6) {
                advance(parser);
                return parse_import(parser);
            } else if (strncmp(directive.start, "c", 1) == 0 && directive.length == 1) {
                advance(parser);
                return parse_c_block(parser);
            }
        }
        error(parser, "Unknown directive");
        return NULL;
    }
    
    /* Expression statement (could be assignment) */
    Token start = parser->current;
    AstNode* expr = parse_expression(parser);
    
    /* Check for assignment */
    if (match(parser, TOKEN_EQ)) {
        AstNode* value = parse_expression(parser);
        return ast_assignment(expr, value, start.line, start.column);
    }
    
    return ast_expr_stmt(expr, start.line, start.column);
}

/* Parse program */
AstNode* parse_program(Parser* parser) {
    AstNode** stmts = NULL;
    int count = 0;
    int capacity = 0;
    
    while (!check(parser, TOKEN_EOF)) {
        if (count >= capacity) {
            capacity = capacity < 8 ? 8 : capacity * 2;
            stmts = realloc(stmts, sizeof(AstNode*) * capacity);
        }
        
        AstNode* stmt = parse_statement(parser);
        if (stmt) {
            stmts[count++] = stmt;
        }
        
        if (parser->panic_mode) synchronize(parser);
    }
    
    return ast_program(stmts, count);
}

/* Main entry point */
AstNode* parse(const char* source) {
    Lexer lexer;
    lexer_init(&lexer, source);
    
    Parser parser;
    parser_init(&parser, &lexer);
    
    AstNode* ast = parse_program(&parser);
    
    if (parser.had_error) {
        ast_free_tree(ast);
        return NULL;
    }
    
    return ast;
}
