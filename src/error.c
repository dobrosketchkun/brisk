/*
 * Brisk Language - Error Handling Implementation
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "error.h"

BriskError last_error = {0};
bool had_error = false;

void error_set(ErrorType type, const char* file, int line, int column, const char* fmt, ...) {
    had_error = true;
    last_error.type = type;
    last_error.line = line;
    last_error.column = column;
    
    if (file) {
        strncpy(last_error.file, file, sizeof(last_error.file) - 1);
        last_error.file[sizeof(last_error.file) - 1] = '\0';
    } else {
        last_error.file[0] = '\0';
    }
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(last_error.message, sizeof(last_error.message), fmt, args);
    va_end(args);
}

void error_clear(void) {
    had_error = false;
    last_error.type = ERR_NONE;
    last_error.message[0] = '\0';
    last_error.file[0] = '\0';
    last_error.line = 0;
    last_error.column = 0;
}

void error_print(void) {
    if (!had_error) return;
    
    fprintf(stderr, "%s Error", error_type_name(last_error.type));
    
    if (last_error.file[0]) {
        fprintf(stderr, " in %s", last_error.file);
    }
    
    if (last_error.line > 0) {
        fprintf(stderr, " at line %d", last_error.line);
        if (last_error.column > 0) {
            fprintf(stderr, ", column %d", last_error.column);
        }
    }
    
    fprintf(stderr, ": %s\n", last_error.message);
}

const char* error_type_name(ErrorType type) {
    switch (type) {
        case ERR_NONE: return "No";
        case ERR_SYNTAX: return "Syntax";
        case ERR_RUNTIME: return "Runtime";
        case ERR_TYPE: return "Type";
        case ERR_NAME: return "Name";
        case ERR_INDEX: return "Index";
        case ERR_IO: return "IO";
        case ERR_FFI: return "FFI";
        default: return "Unknown";
    }
}
