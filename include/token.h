/*
 * Brisk Language - Token Definitions
 */

#ifndef BRISK_TOKEN_H
#define BRISK_TOKEN_H

/* Token types */
typedef enum {
    /* Special tokens */
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_NEWLINE,
    
    /* Literals */
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NIL,
    
    /* Identifier */
    TOKEN_IDENTIFIER,
    
    /* Keywords */
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_MATCH,
    TOKEN_DEFER,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    
    /* Operators */
    TOKEN_PLUS,         /* + */
    TOKEN_MINUS,        /* - */
    TOKEN_STAR,         /* * */
    TOKEN_SLASH,        /* / */
    TOKEN_PERCENT,      /* % */
    TOKEN_EQ,           /* = */
    TOKEN_EQEQ,         /* == */
    TOKEN_NEQ,          /* != */
    TOKEN_LT,           /* < */
    TOKEN_GT,           /* > */
    TOKEN_LTE,          /* <= */
    TOKEN_GTE,          /* >= */
    TOKEN_AMPERSAND,    /* & (address-of) */
    TOKEN_DOTDOT,       /* .. (range) */
    TOKEN_ARROW,        /* => (match arm) */
    TOKEN_RARROW,       /* -> (return type) */
    
    /* Delimiters */
    TOKEN_LPAREN,       /* ( */
    TOKEN_RPAREN,       /* ) */
    TOKEN_LBRACE,       /* { */
    TOKEN_RBRACE,       /* } */
    TOKEN_LBRACKET,     /* [ */
    TOKEN_RBRACKET,     /* ] */
    TOKEN_COMMA,        /* , */
    TOKEN_DOT,          /* . */
    TOKEN_COLON,        /* : */
    TOKEN_COLONCOLON,   /* :: (const) */
    TOKEN_COLONEQ,      /* := (var decl) */
    TOKEN_SEMICOLON,    /* ; (optional) */
    TOKEN_HASH,         /* # (comment start) */
    
    /* Special */
    TOKEN_AT,           /* @ (directives) */
    TOKEN_UNDERSCORE,   /* _ (wildcard) */
    TOKEN_BANG,         /* ! (for !=) */
    
    TOKEN_COUNT         /* Number of token types */
} TokenType;

/* Token structure */
typedef struct {
    TokenType type;
    const char* start;  /* Pointer to start in source */
    int length;
    int line;
    int column;
} Token;

/* Get token type name for debugging */
const char* token_type_name(TokenType type);

#endif /* BRISK_TOKEN_H */
