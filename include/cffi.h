/*
 * Brisk Language - C Foreign Function Interface
 */

#ifndef BRISK_CFFI_H
#define BRISK_CFFI_H

#include <stdbool.h>
#include <stdint.h>
#include <ffi.h>
#include "value.h"

/* C Type enumeration */
typedef enum {
    CTYPE_VOID,
    CTYPE_CHAR,
    CTYPE_SCHAR,
    CTYPE_UCHAR,
    CTYPE_SHORT,
    CTYPE_USHORT,
    CTYPE_INT,
    CTYPE_UINT,
    CTYPE_LONG,
    CTYPE_ULONG,
    CTYPE_LONGLONG,
    CTYPE_ULONGLONG,
    CTYPE_FLOAT,
    CTYPE_DOUBLE,
    CTYPE_POINTER,
    CTYPE_STRING,   /* char* (null-terminated) */
    CTYPE_STRUCT,
    CTYPE_BOOL,
    CTYPE_SIZE_T,
    CTYPE_INT8,
    CTYPE_INT16,
    CTYPE_INT32,
    CTYPE_INT64,
    CTYPE_UINT8,
    CTYPE_UINT16,
    CTYPE_UINT32,
    CTYPE_UINT64
} CType;

/* C struct field descriptor */
typedef struct {
    char* name;
    CType type;
    int offset;
    int size;
    struct CStructDesc* struct_type;  /* For nested structs */
} CFieldDesc;

/* C struct descriptor */
struct CStructDesc {
    char* name;
    CFieldDesc* fields;
    int field_count;
    int size;
    int alignment;
    ffi_type* ffi_type_ptr;  /* For FFI */
};

/* C function descriptor */
typedef struct {
    char* name;
    CType return_type;
    CType* param_types;
    int param_count;
    bool is_variadic;
    void* func_ptr;
    ffi_cif cif;
    bool cif_prepared;
} CFunctionDesc;

/* C function object (for interpreter) */
struct ObjCFunction {
    Object obj;
    CFunctionDesc* desc;
};

/* Get ffi_type for a CType */
ffi_type* ctype_to_ffi(CType type);

/* Get size of a CType */
int ctype_size(CType type);

/* Parse C type from string */
CType ctype_from_string(const char* str);

/* Get C type name as string */
const char* ctype_name(CType type);

/* Create a C function descriptor */
CFunctionDesc* cfunc_create(const char* name, CType return_type, 
                             CType* param_types, int param_count,
                             bool is_variadic, void* func_ptr);

/* Free a C function descriptor */
void cfunc_free(CFunctionDesc* desc);

/* Prepare FFI call interface */
bool cfunc_prepare(CFunctionDesc* desc);

/* Marshal Brisk value to C type */
bool marshal_to_c(Value value, CType type, void* out);

/* Marshal C value to Brisk */
Value marshal_from_c(void* in, CType type);

/* Call a C function */
Value cffi_call(CFunctionDesc* desc, int arg_count, Value* args);

/* Create ObjCFunction from descriptor */
ObjCFunction* cfunction_create(CFunctionDesc* desc);

/* Create a C struct descriptor */
CStructDesc* cstruct_desc_create(const char* name, int field_count);

/* Add field to struct descriptor */
void cstruct_desc_add_field(CStructDesc* desc, int index, 
                             const char* name, CType type,
                             int offset, int size);

/* Finalize struct descriptor (calculate size, create ffi_type) */
void cstruct_desc_finalize(CStructDesc* desc);

/* Free struct descriptor */
void cstruct_desc_free(CStructDesc* desc);

/* Create a C struct instance */
ObjCStruct* cstruct_create(CStructDesc* desc);

/* Get field from C struct */
Value cstruct_get_field(ObjCStruct* obj, const char* name);

/* Set field in C struct */
bool cstruct_set_field(ObjCStruct* obj, const char* name, Value value);

/* Get raw pointer to struct data */
void* cstruct_data_ptr(ObjCStruct* obj);

#endif /* BRISK_CFFI_H */
