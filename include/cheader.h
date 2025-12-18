/*
 * Brisk Language - C Header Parser
 * Simplified parser for C function declarations, structs, enums
 */

#ifndef BRISK_CHEADER_H
#define BRISK_CHEADER_H

#include <stdbool.h>
#include "cffi.h"
#include "env.h"

/* Parsed function declaration */
typedef struct {
    char* name;
    CType return_type;
    char* return_type_str;
    CType* param_types;
    char** param_names;
    int param_count;
    bool is_variadic;
} ParsedFunction;

/* Parsed struct definition */
typedef struct {
    char* name;
    CFieldDesc* fields;
    int field_count;
} ParsedStruct;

/* Parsed enum definition */
typedef struct {
    char* name;
    char** value_names;
    int* values;
    int count;
} ParsedEnum;

/* Parsed macro constant */
typedef struct {
    char* name;
    bool is_int;
    int64_t int_value;
    double float_value;
    char* string_value;
} ParsedMacro;

/* Header parser context */
typedef struct {
    const char* source;
    const char* current;
    int line;
    
    /* Parsed declarations */
    ParsedFunction* functions;
    int function_count;
    int function_capacity;
    
    ParsedStruct* structs;
    int struct_count;
    int struct_capacity;
    
    ParsedEnum* enums;
    int enum_count;
    int enum_capacity;
    
    ParsedMacro* macros;
    int macro_count;
    int macro_capacity;
    
    /* Typedef mappings */
    char** typedef_names;
    char** typedef_types;
    int typedef_count;
    int typedef_capacity;
} CHeaderParser;

/* Initialize header parser */
void cheader_init(CHeaderParser* parser);

/* Free header parser */
void cheader_free(CHeaderParser* parser);

/* Parse a C header file */
bool cheader_parse(CHeaderParser* parser, const char* source);

/* Load and parse a header file */
bool cheader_load(CHeaderParser* parser, const char* path);

/* Register parsed declarations into environment */
bool cheader_register(CHeaderParser* parser, Environment* env, void* lib_handle);

/* Find include file in standard paths */
char* cheader_find_include(const char* name, bool is_system);

#endif /* BRISK_CHEADER_H */
