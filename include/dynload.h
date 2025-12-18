/*
 * Brisk Language - Dynamic Library Loading (Linux/WSL)
 */

#ifndef BRISK_DYNLOAD_H
#define BRISK_DYNLOAD_H

#include <stdbool.h>

/* Library handle */
typedef void* LibHandle;

/* Open a dynamic library */
LibHandle lib_open(const char* path);

/* Close a dynamic library */
void lib_close(LibHandle handle);

/* Get symbol from library */
void* lib_symbol(LibHandle handle, const char* name);

/* Get last error message */
const char* lib_error(void);

/* Check if library is valid */
bool lib_is_valid(LibHandle handle);

#endif /* BRISK_DYNLOAD_H */
