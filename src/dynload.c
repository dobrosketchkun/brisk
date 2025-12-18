/*
 * Brisk Language - Dynamic Library Loading Implementation (Linux/WSL)
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "dynload.h"

LibHandle lib_open(const char* path) {
    /* If path is NULL, get handle to current process (for libc functions) */
    if (path == NULL) {
        return dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
    }
    
    /* Try to open the library */
    void* handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (handle != NULL) {
        return handle;
    }
    
    /* Try with common suffixes */
    char fullpath[512];
    
    /* Try .so suffix */
    snprintf(fullpath, sizeof(fullpath), "%s.so", path);
    handle = dlopen(fullpath, RTLD_NOW | RTLD_GLOBAL);
    if (handle != NULL) return handle;
    
    /* Try lib prefix and .so suffix */
    snprintf(fullpath, sizeof(fullpath), "lib%s.so", path);
    handle = dlopen(fullpath, RTLD_NOW | RTLD_GLOBAL);
    if (handle != NULL) return handle;
    
    /* Try common library paths */
    const char* lib_paths[] = {
        "/usr/lib",
        "/usr/lib/x86_64-linux-gnu",
        "/usr/local/lib",
        "/lib",
        "/lib/x86_64-linux-gnu",
        NULL
    };
    
    for (int i = 0; lib_paths[i] != NULL; i++) {
        snprintf(fullpath, sizeof(fullpath), "%s/lib%s.so", lib_paths[i], path);
        handle = dlopen(fullpath, RTLD_NOW | RTLD_GLOBAL);
        if (handle != NULL) return handle;
    }
    
    return NULL;
}

void lib_close(LibHandle handle) {
    if (handle != NULL) {
        dlclose(handle);
    }
}

void* lib_symbol(LibHandle handle, const char* name) {
    /* Clear any existing error */
    dlerror();
    
    void* symbol = dlsym(handle, name);
    
    /* Check for error (symbol could legitimately be NULL) */
    char* error = dlerror();
    if (error != NULL) {
        return NULL;
    }
    
    return symbol;
}

const char* lib_error(void) {
    return dlerror();
}

bool lib_is_valid(LibHandle handle) {
    return handle != NULL;
}
