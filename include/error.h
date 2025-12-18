/*
 * Brisk Language - Error Handling
 */

#ifndef BRISK_ERROR_H
#define BRISK_ERROR_H

#include <stdbool.h>

/* Error types */
typedef enum {
    ERR_NONE,
    ERR_SYNTAX,
    ERR_RUNTIME,
    ERR_TYPE,
    ERR_NAME,
    ERR_INDEX,
    ERR_IO,
    ERR_FFI
} ErrorType;

/* Error structure */
typedef struct {
    ErrorType type;
    char message[512];
    char file[256];
    int line;
    int column;
} BriskError;

/* Global error state */
extern BriskError last_error;
extern bool had_error;

/* Set error */
void error_set(ErrorType type, const char* file, int line, int column, const char* fmt, ...);

/* Clear error */
void error_clear(void);

/* Print error */
void error_print(void);

/* Get error type name */
const char* error_type_name(ErrorType type);

#endif /* BRISK_ERROR_H */
