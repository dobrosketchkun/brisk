/*
 * Brisk Language - Lexer Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

/* Token type names for debugging */
static const char* token_names[] = {
    [TOKEN_EOF] = "EOF",
    [TOKEN_ERROR] = "ERROR",
    [TOKEN_NEWLINE] = "NEWLINE",
    [TOKEN_INT] = "INT",
    [TOKEN_FLOAT] = "FLOAT",
    [TOKEN_STRING] = "STRING",
    [TOKEN_TRUE] = "TRUE",
    [TOKEN_FALSE] = "FALSE",
    [TOKEN_NIL] = "NIL",
    [TOKEN_IDENTIFIER] = "IDENTIFIER",
    [TOKEN_FN] = "FN",
    [TOKEN_IF] = "IF",
    [TOKEN_ELIF] = "ELIF",
    [TOKEN_ELSE] = "ELSE",
    [TOKEN_WHILE] = "WHILE",
    [TOKEN_FOR] = "FOR",
    [TOKEN_IN] = "IN",
    [TOKEN_RETURN] = "RETURN",
    [TOKEN_BREAK] = "BREAK",
    [TOKEN_CONTINUE] = "CONTINUE",
    [TOKEN_MATCH] = "MATCH",
    [TOKEN_DEFER] = "DEFER",
    [TOKEN_AND] = "AND",
    [TOKEN_OR] = "OR",
    [TOKEN_NOT] = "NOT",
    [TOKEN_PLUS] = "PLUS",
    [TOKEN_MINUS] = "MINUS",
    [TOKEN_STAR] = "STAR",
    [TOKEN_SLASH] = "SLASH",
    [TOKEN_PERCENT] = "PERCENT",
    [TOKEN_EQ] = "EQ",
    [TOKEN_EQEQ] = "EQEQ",
    [TOKEN_NEQ] = "NEQ",
    [TOKEN_LT] = "LT",
    [TOKEN_GT] = "GT",
    [TOKEN_LTE] = "LTE",
    [TOKEN_GTE] = "GTE",
    [TOKEN_AMPERSAND] = "AMPERSAND",
    [TOKEN_DOTDOT] = "DOTDOT",
    [TOKEN_ARROW] = "ARROW",
    [TOKEN_RARROW] = "RARROW",
    [TOKEN_LPAREN] = "LPAREN",
    [TOKEN_RPAREN] = "RPAREN",
    [TOKEN_LBRACE] = "LBRACE",
    [TOKEN_RBRACE] = "RBRACE",
    [TOKEN_LBRACKET] = "LBRACKET",
    [TOKEN_RBRACKET] = "RBRACKET",
    [TOKEN_COMMA] = "COMMA",
    [TOKEN_DOT] = "DOT",
    [TOKEN_COLON] = "COLON",
    [TOKEN_COLONCOLON] = "COLONCOLON",
    [TOKEN_COLONEQ] = "COLONEQ",
    [TOKEN_SEMICOLON] = "SEMICOLON",
    [TOKEN_HASH] = "HASH",
    [TOKEN_AT] = "AT",
    [TOKEN_UNDERSCORE] = "UNDERSCORE",
    [TOKEN_BANG] = "BANG",
};

const char* token_type_name(TokenType type) {
    if (type >= 0 && type < TOKEN_COUNT) {
        return token_names[type];
    }
    return "UNKNOWN";
}

/* Helper functions */
static bool is_at_end(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->current[1];
}

static char advance(Lexer* lexer) {
    lexer->column++;
    return *lexer->current++;
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (*lexer->current != expected) return false;
    lexer->current++;
    lexer->column++;
    return true;
}

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->start_column;
    return token;
}

static Token error_token(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance(lexer);
                break;
            case '#':
                /* Comment - skip until end of line */
                while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                    advance(lexer);
                }
                break;
            default:
                return;
        }
    }
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static bool is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

static bool is_hex_digit(char c) {
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static Token scan_number(Lexer* lexer) {
    /* Check for hex number: 0x... */
    if (lexer->current - lexer->start == 1 && 
        lexer->start[0] == '0' && 
        (peek(lexer) == 'x' || peek(lexer) == 'X')) {
        advance(lexer); /* Consume 'x' */
        while (is_hex_digit(peek(lexer)) || peek(lexer) == '_') {
            if (peek(lexer) == '_') {
                advance(lexer);
                continue;
            }
            advance(lexer);
        }
        return make_token(lexer, TOKEN_INT);
    }
    
    /* Consume leading digits */
    while (is_digit(peek(lexer)) || peek(lexer) == '_') {
        if (peek(lexer) == '_') {
            advance(lexer); /* Skip digit separator */
            continue;
        }
        advance(lexer);
    }
    
    /* Check for float */
    if (peek(lexer) == '.' && is_digit(peek_next(lexer))) {
        advance(lexer); /* Consume '.' */
        while (is_digit(peek(lexer)) || peek(lexer) == '_') {
            if (peek(lexer) == '_') {
                advance(lexer);
                continue;
            }
            advance(lexer);
        }
        return make_token(lexer, TOKEN_FLOAT);
    }
    
    return make_token(lexer, TOKEN_INT);
}

static Token scan_string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 0;
        }
        if (peek(lexer) == '\\') {
            advance(lexer); /* Skip backslash */
            if (!is_at_end(lexer)) {
                advance(lexer); /* Skip escaped character */
            }
        } else {
            advance(lexer);
        }
    }
    
    if (is_at_end(lexer)) {
        return error_token(lexer, "Unterminated string");
    }
    
    advance(lexer); /* Closing quote */
    return make_token(lexer, TOKEN_STRING);
}

/* Keyword lookup using a trie-like approach (simplified) */
static TokenType check_keyword(const char* start, int length, 
                               int offset, int rest_len, 
                               const char* rest, TokenType type) {
    if (length == offset + rest_len &&
        memcmp(start + offset, rest, rest_len) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Lexer* lexer) {
    const char* start = lexer->start;
    int length = (int)(lexer->current - lexer->start);
    
    switch (start[0]) {
        case 'a':
            if (length > 1) {
                if (start[1] == 'n') return check_keyword(start, length, 2, 1, "d", TOKEN_AND);
            }
            break;
        case 'b':
            if (length > 1) {
                if (start[1] == 'r') return check_keyword(start, length, 2, 3, "eak", TOKEN_BREAK);
            }
            break;
        case 'c':
            if (length > 1) {
                if (start[1] == 'o') return check_keyword(start, length, 2, 6, "ntinue", TOKEN_CONTINUE);
            }
            break;
        case 'd':
            if (length > 1) {
                if (start[1] == 'e') return check_keyword(start, length, 2, 3, "fer", TOKEN_DEFER);
            }
            break;
        case 'e':
            if (length > 1) {
                if (start[1] == 'l') {
                    if (length > 2 && start[2] == 's') return check_keyword(start, length, 3, 1, "e", TOKEN_ELSE);
                    if (length > 2 && start[2] == 'i') return check_keyword(start, length, 3, 1, "f", TOKEN_ELIF);
                }
            }
            break;
        case 'f':
            if (length > 1) {
                if (start[1] == 'n') {
                    if (length == 2) return TOKEN_FN;
                }
                if (start[1] == 'o') return check_keyword(start, length, 2, 1, "r", TOKEN_FOR);
                if (start[1] == 'a') return check_keyword(start, length, 2, 3, "lse", TOKEN_FALSE);
            }
            break;
        case 'i':
            if (length > 1) {
                if (start[1] == 'f') {
                    if (length == 2) return TOKEN_IF;
                }
                if (start[1] == 'n') {
                    if (length == 2) return TOKEN_IN;
                }
            }
            break;
        case 'm':
            if (length > 1) {
                if (start[1] == 'a') return check_keyword(start, length, 2, 3, "tch", TOKEN_MATCH);
            }
            break;
        case 'n':
            if (length > 1) {
                if (start[1] == 'i') return check_keyword(start, length, 2, 1, "l", TOKEN_NIL);
                if (start[1] == 'o') return check_keyword(start, length, 2, 1, "t", TOKEN_NOT);
            }
            break;
        case 'o':
            if (length > 1) {
                if (start[1] == 'r') {
                    if (length == 2) return TOKEN_OR;
                }
            }
            break;
        case 'r':
            if (length > 1) {
                if (start[1] == 'e') return check_keyword(start, length, 2, 4, "turn", TOKEN_RETURN);
            }
            break;
        case 't':
            if (length > 1) {
                if (start[1] == 'r') return check_keyword(start, length, 2, 2, "ue", TOKEN_TRUE);
            }
            break;
        case 'w':
            if (length > 1) {
                if (start[1] == 'h') return check_keyword(start, length, 2, 3, "ile", TOKEN_WHILE);
            }
            break;
        case '_':
            if (length == 1) return TOKEN_UNDERSCORE;
            break;
    }
    
    return TOKEN_IDENTIFIER;
}

static Token scan_identifier(Lexer* lexer) {
    while (is_alnum(peek(lexer))) {
        advance(lexer);
    }
    return make_token(lexer, identifier_type(lexer));
}

/* Public functions */
void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start_column = 1;
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    
    lexer->start = lexer->current;
    lexer->start_column = lexer->column;
    
    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF);
    }
    
    char c = advance(lexer);
    
    /* Identifiers and keywords */
    if (is_alpha(c)) {
        return scan_identifier(lexer);
    }
    
    /* Numbers */
    if (is_digit(c)) {
        return scan_number(lexer);
    }
    
    /* Other tokens */
    switch (c) {
        case '\n':
            lexer->line++;
            lexer->column = 1;
            return make_token(lexer, TOKEN_NEWLINE);
            
        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case '{': return make_token(lexer, TOKEN_LBRACE);
        case '}': return make_token(lexer, TOKEN_RBRACE);
        case '[': return make_token(lexer, TOKEN_LBRACKET);
        case ']': return make_token(lexer, TOKEN_RBRACKET);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '*': return make_token(lexer, TOKEN_STAR);
        case '/': return make_token(lexer, TOKEN_SLASH);
        case '%': return make_token(lexer, TOKEN_PERCENT);
        case '&': return make_token(lexer, TOKEN_AMPERSAND);
        case '@': return make_token(lexer, TOKEN_AT);
        
        case '-':
            if (match(lexer, '>')) return make_token(lexer, TOKEN_RARROW);
            return make_token(lexer, TOKEN_MINUS);
            
        case '.':
            if (match(lexer, '.')) return make_token(lexer, TOKEN_DOTDOT);
            return make_token(lexer, TOKEN_DOT);
            
        case ':':
            if (match(lexer, ':')) return make_token(lexer, TOKEN_COLONCOLON);
            if (match(lexer, '=')) return make_token(lexer, TOKEN_COLONEQ);
            return make_token(lexer, TOKEN_COLON);
            
        case '=':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_EQEQ);
            if (match(lexer, '>')) return make_token(lexer, TOKEN_ARROW);
            return make_token(lexer, TOKEN_EQ);
            
        case '!':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_NEQ);
            return make_token(lexer, TOKEN_BANG);
            
        case '<':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_LTE);
            return make_token(lexer, TOKEN_LT);
            
        case '>':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_GTE);
            return make_token(lexer, TOKEN_GT);
            
        case '"':
            return scan_string(lexer);
    }
    
    return error_token(lexer, "Unexpected character");
}

Token lexer_peek_token(Lexer* lexer) {
    /* Save state */
    const char* start = lexer->start;
    const char* current = lexer->current;
    int line = lexer->line;
    int column = lexer->column;
    int start_column = lexer->start_column;
    
    /* Get token */
    Token token = lexer_next_token(lexer);
    
    /* Restore state */
    lexer->start = start;
    lexer->current = current;
    lexer->line = line;
    lexer->column = column;
    lexer->start_column = start_column;
    
    return token;
}
