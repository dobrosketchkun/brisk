/*
 * Brisk Language - C Header Parser Implementation
 * Simplified parser for common C header patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cheader.h"
#include "dynload.h"
#include "memory.h"

/* Helper macros */
#define IS_SPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r')
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_ALNUM(c) (IS_ALPHA(c) || IS_DIGIT(c))

/* Skip whitespace and comments */
static void skip_space(CHeaderParser* p) {
    while (*p->current) {
        if (IS_SPACE(*p->current)) {
            p->current++;
        } else if (*p->current == '\n') {
            p->current++;
            p->line++;
        } else if (p->current[0] == '/' && p->current[1] == '/') {
            /* Line comment */
            while (*p->current && *p->current != '\n') p->current++;
        } else if (p->current[0] == '/' && p->current[1] == '*') {
            /* Block comment */
            p->current += 2;
            while (*p->current && !(p->current[0] == '*' && p->current[1] == '/')) {
                if (*p->current == '\n') p->line++;
                p->current++;
            }
            if (*p->current) p->current += 2;
        } else {
            break;
        }
    }
}

/* Check if at keyword */
static bool match_keyword(CHeaderParser* p, const char* kw) {
    int len = strlen(kw);
    if (strncmp(p->current, kw, len) == 0 && !IS_ALNUM(p->current[len])) {
        p->current += len;
        return true;
    }
    return false;
}

/* Read identifier */
static char* read_ident(CHeaderParser* p) {
    skip_space(p);
    if (!IS_ALPHA(*p->current)) return NULL;
    
    const char* start = p->current;
    while (IS_ALNUM(*p->current)) p->current++;
    
    int len = p->current - start;
    char* ident = mem_alloc(len + 1);
    memcpy(ident, start, len);
    ident[len] = '\0';
    return ident;
}

/* Forward declarations */
static void skip_parens(CHeaderParser* p);
static void skip_gnu_extension(CHeaderParser* p);

/* Read C type (simplified) */
static CType read_type(CHeaderParser* p, char** type_str) {
    skip_space(p);
    skip_gnu_extension(p);
    
    char buffer[256] = {0};
    int buf_len = 0;
    (void)buf_len;  /* Suppress unused warning */
    
    /* Handle qualifiers */
    while (true) {
        skip_space(p);
        skip_gnu_extension(p);
        if (match_keyword(p, "const")) {
            strcat(buffer, "const "); buf_len += 6;
        } else if (match_keyword(p, "volatile")) {
            /* Skip */
        } else if (match_keyword(p, "static")) {
            /* Skip */
        } else if (match_keyword(p, "extern")) {
            /* Skip */
        } else if (match_keyword(p, "inline")) {
            /* Skip */
        } else if (match_keyword(p, "register")) {
            /* Skip */
        } else {
            break;
        }
    }
    
    skip_gnu_extension(p);
    
    /* Handle unsigned/signed */
    bool is_unsigned = false;
    if (match_keyword(p, "unsigned")) {
        is_unsigned = true;
        strcat(buffer, "unsigned "); buf_len += 9;
    } else if (match_keyword(p, "signed")) {
        strcat(buffer, "signed "); buf_len += 7;
    }
    
    skip_space(p);
    
    /* Read type name */
    CType result = CTYPE_INT;  /* Default */
    
    if (match_keyword(p, "void")) {
        result = CTYPE_VOID;
        strcat(buffer, "void");
    } else if (match_keyword(p, "char")) {
        result = is_unsigned ? CTYPE_UCHAR : CTYPE_CHAR;
        strcat(buffer, "char");
    } else if (match_keyword(p, "short")) {
        result = is_unsigned ? CTYPE_USHORT : CTYPE_SHORT;
        strcat(buffer, "short");
    } else if (match_keyword(p, "int")) {
        result = is_unsigned ? CTYPE_UINT : CTYPE_INT;
        strcat(buffer, "int");
    } else if (match_keyword(p, "long")) {
        skip_space(p);
        if (match_keyword(p, "long")) {
            result = is_unsigned ? CTYPE_ULONGLONG : CTYPE_LONGLONG;
            strcat(buffer, "long long");
        } else if (match_keyword(p, "double")) {
            result = CTYPE_DOUBLE;
            strcat(buffer, "long double");
        } else {
            result = is_unsigned ? CTYPE_ULONG : CTYPE_LONG;
            strcat(buffer, "long");
        }
    } else if (match_keyword(p, "float")) {
        result = CTYPE_FLOAT;
        strcat(buffer, "float");
    } else if (match_keyword(p, "double")) {
        result = CTYPE_DOUBLE;
        strcat(buffer, "double");
    } else if (match_keyword(p, "size_t")) {
        result = CTYPE_SIZE_T;
        strcat(buffer, "size_t");
    } else if (match_keyword(p, "_Bool") || match_keyword(p, "bool")) {
        result = CTYPE_BOOL;
        strcat(buffer, "bool");
    } else if (match_keyword(p, "int8_t")) {
        result = CTYPE_INT8;
        strcat(buffer, "int8_t");
    } else if (match_keyword(p, "int16_t")) {
        result = CTYPE_INT16;
        strcat(buffer, "int16_t");
    } else if (match_keyword(p, "int32_t")) {
        result = CTYPE_INT32;
        strcat(buffer, "int32_t");
    } else if (match_keyword(p, "int64_t")) {
        result = CTYPE_INT64;
        strcat(buffer, "int64_t");
    } else if (match_keyword(p, "uint8_t")) {
        result = CTYPE_UINT8;
        strcat(buffer, "uint8_t");
    } else if (match_keyword(p, "uint16_t")) {
        result = CTYPE_UINT16;
        strcat(buffer, "uint16_t");
    } else if (match_keyword(p, "uint32_t")) {
        result = CTYPE_UINT32;
        strcat(buffer, "uint32_t");
    } else if (match_keyword(p, "uint64_t")) {
        result = CTYPE_UINT64;
        strcat(buffer, "uint64_t");
    } else if (match_keyword(p, "struct")) {
        result = CTYPE_STRUCT;
        strcat(buffer, "struct ");
        char* name = read_ident(p);
        if (name) {
            strcat(buffer, name);
            mem_free(name, strlen(name) + 1);
        }
    } else {
        /* Unknown type - could be typedef */
        char* name = read_ident(p);
        if (name) {
            strcat(buffer, name);
            result = CTYPE_INT;  /* Assume int-like */
            mem_free(name, strlen(name) + 1);
        }
    }
    
    /* Handle pointers */
    skip_space(p);
    while (*p->current == '*') {
        /* char* and const char* should be CTYPE_STRING */
        if (result == CTYPE_CHAR || result == CTYPE_UCHAR || result == CTYPE_SCHAR) {
            result = CTYPE_STRING;
        } else {
            result = CTYPE_POINTER;
        }
        strcat(buffer, "*");
        p->current++;
        skip_space(p);
    }
    
    if (type_str) {
        *type_str = mem_alloc(strlen(buffer) + 1);
        strcpy(*type_str, buffer);
    }
    
    return result;
}

/* Skip to character */
static void skip_to(CHeaderParser* p, char c) {
    while (*p->current && *p->current != c) {
        if (*p->current == '\n') p->line++;
        p->current++;
    }
}

/* Skip balanced braces */
static void skip_braces(CHeaderParser* p) {
    int depth = 1;
    p->current++;  /* Skip opening brace */
    while (*p->current && depth > 0) {
        if (*p->current == '{') depth++;
        else if (*p->current == '}') depth--;
        else if (*p->current == '\n') p->line++;
        p->current++;
    }
}

/* Skip balanced parentheses */
static void skip_parens(CHeaderParser* p) {
    int depth = 1;
    p->current++;  /* Skip opening paren */
    while (*p->current && depth > 0) {
        if (*p->current == '(') depth++;
        else if (*p->current == ')') depth--;
        else if (*p->current == '\n') p->line++;
        p->current++;
    }
}

/* Skip __attribute__, __asm__, etc. */
static void skip_gnu_extension(CHeaderParser* p) {
    skip_space(p);
    while (*p->current == '_' && p->current[1] == '_') {
        /* Skip __attribute__, __asm__, __extension__, etc. */
        if (strncmp(p->current, "__attribute__", 13) == 0 ||
            strncmp(p->current, "__asm__", 7) == 0 ||
            strncmp(p->current, "__asm", 5) == 0 ||
            strncmp(p->current, "__extension__", 13) == 0 ||
            strncmp(p->current, "__inline__", 10) == 0 ||
            strncmp(p->current, "__inline", 8) == 0 ||
            strncmp(p->current, "__restrict", 10) == 0 ||
            strncmp(p->current, "__const", 7) == 0 ||
            strncmp(p->current, "__volatile", 10) == 0 ||
            strncmp(p->current, "__nonnull", 9) == 0 ||
            strncmp(p->current, "__wur", 5) == 0 ||
            strncmp(p->current, "__THROW", 7) == 0 ||
            strncmp(p->current, "__nothrow", 9) == 0) {
            
            /* Skip the keyword */
            while (IS_ALNUM(*p->current) || *p->current == '_') p->current++;
            skip_space(p);
            
            /* Skip any parenthesized arguments */
            while (*p->current == '(') {
                skip_parens(p);
                skip_space(p);
            }
            skip_space(p);
        } else {
            break;
        }
    }
}

/* Parse function declaration */
static bool parse_function(CHeaderParser* p) {
    const char* start = p->current;
    
    /* Read return type */
    char* ret_type_str = NULL;
    CType ret_type = read_type(p, &ret_type_str);
    
    skip_space(p);
    
    /* Read function name */
    char* name = read_ident(p);
    if (!name) {
        if (ret_type_str) mem_free(ret_type_str, strlen(ret_type_str) + 1);
        return false;
    }
    
    skip_space(p);
    
    /* Check for '(' */
    if (*p->current != '(') {
        mem_free(name, strlen(name) + 1);
        if (ret_type_str) mem_free(ret_type_str, strlen(ret_type_str) + 1);
        return false;
    }
    p->current++;
    
    /* Parse parameters */
    CType param_types[32];
    char* param_names[32];
    int param_count = 0;
    bool is_variadic = false;
    
    skip_space(p);
    
    /* Check for void parameter */
    if (match_keyword(p, "void")) {
        skip_space(p);
        if (*p->current == ')') {
            /* No parameters */
        } else {
            /* void* or void something */
            p->current = start;
            skip_to(p, ';');
            if (*p->current) p->current++;
            mem_free(name, strlen(name) + 1);
            if (ret_type_str) mem_free(ret_type_str, strlen(ret_type_str) + 1);
            return false;
        }
    }
    
    while (*p->current && *p->current != ')') {
        const char* loop_start = p->current;
        skip_space(p);
        
        /* Check for variadic */
        if (p->current[0] == '.' && p->current[1] == '.' && p->current[2] == '.') {
            is_variadic = true;
            p->current += 3;
            skip_space(p);
            break;
        }
        
        /* Skip nested parentheses (macros like __REDIRECT) */
        if (*p->current == '(') {
            skip_parens(p);
            skip_space(p);
            if (*p->current == ',') p->current++;
            continue;
        }
        
        /* If not a valid type start, skip to comma or closing paren */
        if (!IS_ALPHA(*p->current) && *p->current != '_') {
            while (*p->current && *p->current != ',' && *p->current != ')') {
                p->current++;
            }
            if (*p->current == ',') p->current++;
            continue;
        }
        
        /* Parse parameter type */
        char* ptype_str = NULL;
        CType ptype = read_type(p, &ptype_str);
        if (ptype_str) mem_free(ptype_str, strlen(ptype_str) + 1);
        
        skip_space(p);
        
        /* Parameter name (optional) */
        char* pname = NULL;
        if (IS_ALPHA(*p->current) || *p->current == '_') {
            pname = read_ident(p);
        }
        
        /* Skip array brackets */
        skip_space(p);
        while (*p->current == '[') {
            skip_to(p, ']');
            if (*p->current) p->current++;
            ptype = CTYPE_POINTER;
        }
        
        /* Skip any trailing attributes */
        skip_gnu_extension(p);
        
        if (param_count < 32) {
            param_types[param_count] = ptype;
            param_names[param_count] = pname;
            param_count++;
        } else if (pname) {
            mem_free(pname, strlen(pname) + 1);
        }
        
        skip_space(p);
        if (*p->current == ',') p->current++;
        
        /* Safety: ensure we made progress */
        if (p->current == loop_start) {
            p->current++;
        }
    }
    
    /* Skip ')' */
    if (*p->current == ')') p->current++;
    
    /* Skip to ';' or '{' */
    skip_space(p);
    if (*p->current == '{') {
        skip_braces(p);
    } else {
        skip_to(p, ';');
        if (*p->current) p->current++;
    }
    
    /* Add to functions list */
    if (p->function_count >= p->function_capacity) {
        p->function_capacity = p->function_capacity < 16 ? 16 : p->function_capacity * 2;
        p->functions = realloc(p->functions, sizeof(ParsedFunction) * p->function_capacity);
    }
    
    ParsedFunction* fn = &p->functions[p->function_count++];
    fn->name = name;
    fn->return_type = ret_type;
    fn->return_type_str = ret_type_str;
    fn->param_count = param_count;
    fn->is_variadic = is_variadic;
    
    if (param_count > 0) {
        fn->param_types = mem_alloc(sizeof(CType) * param_count);
        fn->param_names = mem_alloc(sizeof(char*) * param_count);
        memcpy(fn->param_types, param_types, sizeof(CType) * param_count);
        memcpy(fn->param_names, param_names, sizeof(char*) * param_count);
    } else {
        fn->param_types = NULL;
        fn->param_names = NULL;
    }
    
    return true;
}

/* Parse #define */
static bool parse_define(CHeaderParser* p) {
    skip_space(p);
    
    char* name = read_ident(p);
    if (!name) return false;
    
    skip_space(p);
    
    /* Skip function-like macros */
    if (*p->current == '(') {
        skip_to(p, '\n');
        mem_free(name, strlen(name) + 1);
        return false;
    }
    
    /* Read value */
    const char* value_start = p->current;
    while (*p->current && *p->current != '\n') p->current++;
    int value_len = p->current - value_start;
    
    /* Trim trailing whitespace */
    while (value_len > 0 && IS_SPACE(value_start[value_len - 1])) value_len--;
    
    if (value_len == 0) {
        mem_free(name, strlen(name) + 1);
        return false;
    }
    
    /* Try to parse as integer */
    char value_buf[256];
    if (value_len >= (int)sizeof(value_buf)) {
        mem_free(name, strlen(name) + 1);
        return false;
    }
    memcpy(value_buf, value_start, value_len);
    value_buf[value_len] = '\0';
    
    /* Add to macros */
    if (p->macro_count >= p->macro_capacity) {
        p->macro_capacity = p->macro_capacity < 16 ? 16 : p->macro_capacity * 2;
        p->macros = realloc(p->macros, sizeof(ParsedMacro) * p->macro_capacity);
    }
    
    ParsedMacro* macro = &p->macros[p->macro_count++];
    macro->name = name;
    macro->string_value = NULL;
    
    /* Try parsing as number */
    char* end;
    long long val = strtoll(value_buf, &end, 0);
    if (*end == '\0' || IS_SPACE(*end)) {
        macro->is_int = true;
        macro->int_value = val;
    } else {
        /* Try as float */
        double fval = strtod(value_buf, &end);
        if (*end == '\0' || IS_SPACE(*end)) {
            macro->is_int = false;
            macro->float_value = fval;
        } else {
            /* Keep as string */
            macro->is_int = true;
            macro->int_value = 0;
            macro->string_value = mem_alloc(value_len + 1);
            memcpy(macro->string_value, value_buf, value_len);
            macro->string_value[value_len] = '\0';
        }
    }
    
    return true;
}

/* Parse enum */
static bool parse_enum(CHeaderParser* p) {
    skip_space(p);
    
    /* Optional enum name */
    char* name = NULL;
    if (IS_ALPHA(*p->current)) {
        name = read_ident(p);
    }
    
    skip_space(p);
    
    if (*p->current != '{') {
        if (name) mem_free(name, strlen(name) + 1);
        return false;
    }
    p->current++;
    
    /* Parse enum values */
    char* value_names[256];
    int values[256];
    int count = 0;
    int current_value = 0;
    
    while (*p->current && *p->current != '}' && count < 256) {
        skip_space(p);
        
        if (*p->current == '}') break;
        
        char* val_name = read_ident(p);
        if (!val_name) break;
        
        skip_space(p);
        
        if (*p->current == '=') {
            p->current++;
            skip_space(p);
            
            /* Parse value */
            const char* val_start = p->current;
            while (*p->current && *p->current != ',' && *p->current != '}') {
                p->current++;
            }
            
            char val_buf[64];
            int val_len = p->current - val_start;
            if (val_len >= (int)sizeof(val_buf)) val_len = (int)sizeof(val_buf) - 1;
            memcpy(val_buf, val_start, val_len);
            val_buf[val_len] = '\0';
            
            current_value = (int)strtol(val_buf, NULL, 0);
        }
        
        value_names[count] = val_name;
        values[count] = current_value++;
        count++;
        
        skip_space(p);
        if (*p->current == ',') p->current++;
    }
    
    if (*p->current == '}') p->current++;
    
    /* Skip to semicolon */
    skip_to(p, ';');
    if (*p->current) p->current++;
    
    /* Add to enums */
    if (p->enum_count >= p->enum_capacity) {
        p->enum_capacity = p->enum_capacity < 16 ? 16 : p->enum_capacity * 2;
        p->enums = realloc(p->enums, sizeof(ParsedEnum) * p->enum_capacity);
    }
    
    ParsedEnum* e = &p->enums[p->enum_count++];
    e->name = name;
    e->count = count;
    
    if (count > 0) {
        e->value_names = mem_alloc(sizeof(char*) * count);
        e->values = mem_alloc(sizeof(int) * count);
        memcpy(e->value_names, value_names, sizeof(char*) * count);
        memcpy(e->values, values, sizeof(int) * count);
    } else {
        e->value_names = NULL;
        e->values = NULL;
    }
    
    return true;
}

/* Initialize header parser */
void cheader_init(CHeaderParser* parser) {
    memset(parser, 0, sizeof(CHeaderParser));
}

/* Free header parser */
void cheader_free(CHeaderParser* parser) {
    for (int i = 0; i < parser->function_count; i++) {
        ParsedFunction* fn = &parser->functions[i];
        if (fn->name) mem_free(fn->name, strlen(fn->name) + 1);
        if (fn->return_type_str) mem_free(fn->return_type_str, strlen(fn->return_type_str) + 1);
        if (fn->param_types) mem_free(fn->param_types, sizeof(CType) * fn->param_count);
        if (fn->param_names) {
            for (int j = 0; j < fn->param_count; j++) {
                if (fn->param_names[j]) {
                    mem_free(fn->param_names[j], strlen(fn->param_names[j]) + 1);
                }
            }
            mem_free(fn->param_names, sizeof(char*) * fn->param_count);
        }
    }
    free(parser->functions);
    
    for (int i = 0; i < parser->enum_count; i++) {
        ParsedEnum* e = &parser->enums[i];
        if (e->name) mem_free(e->name, strlen(e->name) + 1);
        if (e->value_names) {
            for (int j = 0; j < e->count; j++) {
                if (e->value_names[j]) {
                    mem_free(e->value_names[j], strlen(e->value_names[j]) + 1);
                }
            }
            mem_free(e->value_names, sizeof(char*) * e->count);
        }
        if (e->values) mem_free(e->values, sizeof(int) * e->count);
    }
    free(parser->enums);
    
    for (int i = 0; i < parser->macro_count; i++) {
        ParsedMacro* m = &parser->macros[i];
        if (m->name) mem_free(m->name, strlen(m->name) + 1);
        if (m->string_value) mem_free(m->string_value, strlen(m->string_value) + 1);
    }
    free(parser->macros);
}

/* Parse a C header file */
bool cheader_parse(CHeaderParser* parser, const char* source) {
    parser->source = source;
    parser->current = source;
    parser->line = 1;
    
    /* Limit iterations to prevent hangs on complex headers */
    int max_iterations = 100000;
    int iteration = 0;
    
    while (*parser->current && iteration++ < max_iterations) {
        const char* start_pos = parser->current;
        
        skip_space(parser);
        skip_gnu_extension(parser);
        if (!*parser->current) break;
        
        /* Preprocessor directive */
        if (*parser->current == '#') {
            parser->current++;
            skip_space(parser);
            
            if (match_keyword(parser, "define")) {
                parse_define(parser);
            } else {
                /* Skip other preprocessor directives */
                while (*parser->current && *parser->current != '\n') {
                    /* Handle line continuation */
                    if (parser->current[0] == '\\' && parser->current[1] == '\n') {
                        parser->current += 2;
                        parser->line++;
                    } else {
                        parser->current++;
                    }
                }
            }
            continue;
        }
        
        /* Skip __asm blocks */
        if (strncmp(parser->current, "__asm", 5) == 0 || 
            strncmp(parser->current, "asm", 3) == 0) {
            while (IS_ALNUM(*parser->current) || *parser->current == '_') parser->current++;
            skip_space(parser);
            if (*parser->current == '(') skip_parens(parser);
            if (*parser->current == '{') skip_braces(parser);
            skip_to(parser, ';');
            if (*parser->current) parser->current++;
            continue;
        }
        
        /* Enum */
        if (match_keyword(parser, "enum")) {
            parse_enum(parser);
            continue;
        }
        
        /* Typedef */
        if (match_keyword(parser, "typedef")) {
            /* Skip typedef - handle nested braces */
            skip_space(parser);
            while (*parser->current && *parser->current != ';') {
                if (*parser->current == '{') {
                    skip_braces(parser);
                } else if (*parser->current == '(') {
                    skip_parens(parser);
                } else if (*parser->current == '\n') {
                    parser->line++;
                    parser->current++;
                } else {
                    parser->current++;
                }
            }
            if (*parser->current) parser->current++;
            continue;
        }
        
        /* Struct/union definition (skip) */
        if (match_keyword(parser, "struct") || match_keyword(parser, "union")) {
            skip_space(parser);
            skip_gnu_extension(parser);
            /* Skip name if present */
            if (IS_ALPHA(*parser->current)) {
                char* name = read_ident(parser);
                if (name) mem_free(name, strlen(name) + 1);
            }
            skip_space(parser);
            skip_gnu_extension(parser);
            if (*parser->current == '{') {
                skip_braces(parser);
            }
            skip_to(parser, ';');
            if (*parser->current) parser->current++;
            continue;
        }
        
        /* Skip extern "C" blocks */
        if (match_keyword(parser, "extern")) {
            skip_space(parser);
            if (*parser->current == '"') {
                /* extern "C" */
                skip_to(parser, '"');
                if (*parser->current) parser->current++;
                skip_to(parser, '"');
                if (*parser->current) parser->current++;
                skip_space(parser);
                if (*parser->current == '{') {
                    /* Don't skip the whole block, just the brace */
                    parser->current++;
                }
            }
            continue;
        }
        
        /* Try to parse as function declaration */
        const char* save = parser->current;
        if (!parse_function(parser)) {
            /* Skip to next statement */
            parser->current = save;
            skip_to(parser, ';');
            if (*parser->current) parser->current++;
        }
        
        /* Ensure we always make progress to avoid infinite loops */
        if (parser->current == start_pos && *parser->current) {
            parser->current++;
        }
    }
    
    return true;
}

/* Load and parse a header file */
bool cheader_load(CHeaderParser* parser, const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) return false;
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    
    char* source = mem_alloc(size + 1);
    size_t read = fread(source, 1, size, file);
    source[read] = '\0';
    fclose(file);
    
    bool result = cheader_parse(parser, source);
    
    mem_free(source, size + 1);
    return result;
}

/* Register parsed declarations into environment */
bool cheader_register(CHeaderParser* parser, Environment* env, void* lib_handle) {
    /* Register functions */
    for (int i = 0; i < parser->function_count; i++) {
        ParsedFunction* fn = &parser->functions[i];
        
        void* func_ptr = lib_symbol(lib_handle, fn->name);
        if (!func_ptr) continue;
        
        CFunctionDesc* desc = cfunc_create(
            fn->name, fn->return_type,
            fn->param_types, fn->param_count,
            fn->is_variadic, func_ptr
        );
        
        if (!cfunc_prepare(desc)) {
            cfunc_free(desc);
            continue;
        }
        
        ObjCFunction* cfn = cfunction_create(desc);
        env_define(env, fn->name, strlen(fn->name), OBJ_VAL(cfn), true);
    }
    
    /* Register enum values */
    for (int i = 0; i < parser->enum_count; i++) {
        ParsedEnum* e = &parser->enums[i];
        for (int j = 0; j < e->count; j++) {
            env_define(env, e->value_names[j], strlen(e->value_names[j]),
                      INT_VAL(e->values[j]), true);
        }
    }
    
    /* Register macros */
    for (int i = 0; i < parser->macro_count; i++) {
        ParsedMacro* m = &parser->macros[i];
        Value val;
        if (m->string_value) {
            val = OBJ_VAL(string_create(m->string_value, strlen(m->string_value)));
        } else if (m->is_int) {
            val = INT_VAL(m->int_value);
        } else {
            val = FLOAT_VAL(m->float_value);
        }
        env_define(env, m->name, strlen(m->name), val, true);
    }
    
    return true;
}

/* Find include file in standard paths */
char* cheader_find_include(const char* name, bool is_system) {
    (void)is_system;  /* Suppress unused warning for now */
    
    static const char* system_paths[] = {
        "/usr/include",
        "/usr/local/include",
        "/usr/include/x86_64-linux-gnu",
        NULL
    };
    
    char path[512];
    
    /* Try as-is first */
    FILE* f = fopen(name, "r");
    if (f) {
        fclose(f);
        char* result = mem_alloc(strlen(name) + 1);
        strcpy(result, name);
        return result;
    }
    
    /* Try system paths */
    for (int i = 0; system_paths[i]; i++) {
        snprintf(path, sizeof(path), "%s/%s", system_paths[i], name);
        f = fopen(path, "r");
        if (f) {
            fclose(f);
            char* result = mem_alloc(strlen(path) + 1);
            strcpy(result, path);
            return result;
        }
    }
    
    return NULL;
}
