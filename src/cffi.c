/*
 * Brisk Language - C Foreign Function Interface Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ffi.h>
#include "cffi.h"
#include "memory.h"

/* Get ffi_type for a CType */
ffi_type* ctype_to_ffi(CType type) {
    switch (type) {
        case CTYPE_VOID: return &ffi_type_void;
        case CTYPE_CHAR:
        case CTYPE_SCHAR: return &ffi_type_schar;
        case CTYPE_UCHAR: return &ffi_type_uchar;
        case CTYPE_SHORT: return &ffi_type_sshort;
        case CTYPE_USHORT: return &ffi_type_ushort;
        case CTYPE_INT:
        case CTYPE_BOOL: return &ffi_type_sint;
        case CTYPE_UINT: return &ffi_type_uint;
        case CTYPE_LONG: return &ffi_type_slong;
        case CTYPE_ULONG: return &ffi_type_ulong;
        case CTYPE_LONGLONG: return &ffi_type_sint64;
        case CTYPE_ULONGLONG: return &ffi_type_uint64;
        case CTYPE_FLOAT: return &ffi_type_float;
        case CTYPE_DOUBLE: return &ffi_type_double;
        case CTYPE_POINTER:
        case CTYPE_STRING: return &ffi_type_pointer;
        case CTYPE_SIZE_T: return &ffi_type_ulong;  /* Platform-dependent */
        case CTYPE_INT8: return &ffi_type_sint8;
        case CTYPE_INT16: return &ffi_type_sint16;
        case CTYPE_INT32: return &ffi_type_sint32;
        case CTYPE_INT64: return &ffi_type_sint64;
        case CTYPE_UINT8: return &ffi_type_uint8;
        case CTYPE_UINT16: return &ffi_type_uint16;
        case CTYPE_UINT32: return &ffi_type_uint32;
        case CTYPE_UINT64: return &ffi_type_uint64;
        case CTYPE_STRUCT:
            return &ffi_type_pointer;  /* Pass by pointer */
        default:
            return &ffi_type_void;
    }
}

/* Get size of a CType */
int ctype_size(CType type) {
    switch (type) {
        case CTYPE_VOID: return 0;
        case CTYPE_CHAR:
        case CTYPE_SCHAR:
        case CTYPE_UCHAR:
        case CTYPE_INT8:
        case CTYPE_UINT8: return 1;
        case CTYPE_SHORT:
        case CTYPE_USHORT:
        case CTYPE_INT16:
        case CTYPE_UINT16: return 2;
        case CTYPE_INT:
        case CTYPE_UINT:
        case CTYPE_BOOL:
        case CTYPE_INT32:
        case CTYPE_UINT32:
        case CTYPE_FLOAT: return 4;
        case CTYPE_LONG:
        case CTYPE_ULONG:
        case CTYPE_LONGLONG:
        case CTYPE_ULONGLONG:
        case CTYPE_POINTER:
        case CTYPE_STRING:
        case CTYPE_SIZE_T:
        case CTYPE_INT64:
        case CTYPE_UINT64:
        case CTYPE_DOUBLE: return 8;
        default: return 0;
    }
}

/* Parse C type from string */
CType ctype_from_string(const char* str) {
    if (strcmp(str, "void") == 0) return CTYPE_VOID;
    if (strcmp(str, "char") == 0) return CTYPE_CHAR;
    if (strcmp(str, "signed char") == 0) return CTYPE_SCHAR;
    if (strcmp(str, "unsigned char") == 0) return CTYPE_UCHAR;
    if (strcmp(str, "short") == 0) return CTYPE_SHORT;
    if (strcmp(str, "unsigned short") == 0) return CTYPE_USHORT;
    if (strcmp(str, "int") == 0) return CTYPE_INT;
    if (strcmp(str, "unsigned int") == 0) return CTYPE_UINT;
    if (strcmp(str, "unsigned") == 0) return CTYPE_UINT;
    if (strcmp(str, "long") == 0) return CTYPE_LONG;
    if (strcmp(str, "unsigned long") == 0) return CTYPE_ULONG;
    if (strcmp(str, "long long") == 0) return CTYPE_LONGLONG;
    if (strcmp(str, "unsigned long long") == 0) return CTYPE_ULONGLONG;
    if (strcmp(str, "float") == 0) return CTYPE_FLOAT;
    if (strcmp(str, "double") == 0) return CTYPE_DOUBLE;
    if (strcmp(str, "bool") == 0 || strcmp(str, "_Bool") == 0) return CTYPE_BOOL;
    if (strcmp(str, "size_t") == 0) return CTYPE_SIZE_T;
    if (strcmp(str, "int8_t") == 0) return CTYPE_INT8;
    if (strcmp(str, "int16_t") == 0) return CTYPE_INT16;
    if (strcmp(str, "int32_t") == 0) return CTYPE_INT32;
    if (strcmp(str, "int64_t") == 0) return CTYPE_INT64;
    if (strcmp(str, "uint8_t") == 0) return CTYPE_UINT8;
    if (strcmp(str, "uint16_t") == 0) return CTYPE_UINT16;
    if (strcmp(str, "uint32_t") == 0) return CTYPE_UINT32;
    if (strcmp(str, "uint64_t") == 0) return CTYPE_UINT64;
    if (strstr(str, "*") != NULL) return CTYPE_POINTER;
    if (strcmp(str, "char*") == 0 || strcmp(str, "const char*") == 0) return CTYPE_STRING;
    
    return CTYPE_INT;  /* Default */
}

/* Get C type name as string */
const char* ctype_name(CType type) {
    switch (type) {
        case CTYPE_VOID: return "void";
        case CTYPE_CHAR: return "char";
        case CTYPE_SCHAR: return "signed char";
        case CTYPE_UCHAR: return "unsigned char";
        case CTYPE_SHORT: return "short";
        case CTYPE_USHORT: return "unsigned short";
        case CTYPE_INT: return "int";
        case CTYPE_UINT: return "unsigned int";
        case CTYPE_LONG: return "long";
        case CTYPE_ULONG: return "unsigned long";
        case CTYPE_LONGLONG: return "long long";
        case CTYPE_ULONGLONG: return "unsigned long long";
        case CTYPE_FLOAT: return "float";
        case CTYPE_DOUBLE: return "double";
        case CTYPE_POINTER: return "void*";
        case CTYPE_STRING: return "char*";
        case CTYPE_STRUCT: return "struct";
        case CTYPE_BOOL: return "bool";
        case CTYPE_SIZE_T: return "size_t";
        case CTYPE_INT8: return "int8_t";
        case CTYPE_INT16: return "int16_t";
        case CTYPE_INT32: return "int32_t";
        case CTYPE_INT64: return "int64_t";
        case CTYPE_UINT8: return "uint8_t";
        case CTYPE_UINT16: return "uint16_t";
        case CTYPE_UINT32: return "uint32_t";
        case CTYPE_UINT64: return "uint64_t";
        default: return "unknown";
    }
}

/* Create a C function descriptor */
CFunctionDesc* cfunc_create(const char* name, CType return_type,
                             CType* param_types, int param_count,
                             bool is_variadic, void* func_ptr) {
    CFunctionDesc* desc = mem_alloc(sizeof(CFunctionDesc));
    
    desc->name = mem_alloc(strlen(name) + 1);
    strcpy(desc->name, name);
    
    desc->return_type = return_type;
    desc->param_count = param_count;
    desc->is_variadic = is_variadic;
    desc->func_ptr = func_ptr;
    desc->cif_prepared = false;
    
    if (param_count > 0) {
        desc->param_types = mem_alloc(sizeof(CType) * param_count);
        memcpy(desc->param_types, param_types, sizeof(CType) * param_count);
    } else {
        desc->param_types = NULL;
    }
    
    return desc;
}

/* Free a C function descriptor */
void cfunc_free(CFunctionDesc* desc) {
    if (desc == NULL) return;
    
    if (desc->name) mem_free(desc->name, strlen(desc->name) + 1);
    if (desc->param_types) mem_free(desc->param_types, sizeof(CType) * desc->param_count);
    mem_free(desc, sizeof(CFunctionDesc));
}

/* Prepare FFI call interface */
bool cfunc_prepare(CFunctionDesc* desc) {
    if (desc->cif_prepared) return true;
    
    ffi_type* ret_type = ctype_to_ffi(desc->return_type);
    
    ffi_type** arg_types = NULL;
    if (desc->param_count > 0) {
        arg_types = mem_alloc(sizeof(ffi_type*) * desc->param_count);
        for (int i = 0; i < desc->param_count; i++) {
            arg_types[i] = ctype_to_ffi(desc->param_types[i]);
        }
    }
    
    ffi_status status;
    if (desc->is_variadic) {
        status = ffi_prep_cif_var(&desc->cif, FFI_DEFAULT_ABI,
                                   desc->param_count, desc->param_count,
                                   ret_type, arg_types);
    } else {
        status = ffi_prep_cif(&desc->cif, FFI_DEFAULT_ABI,
                               desc->param_count, ret_type, arg_types);
    }
    
    if (status != FFI_OK) {
        if (arg_types) mem_free(arg_types, sizeof(ffi_type*) * desc->param_count);
        return false;
    }
    
    desc->cif_prepared = true;
    return true;
}

/* Marshal Brisk value to C type */
bool marshal_to_c(Value value, CType type, void* out) {
    switch (type) {
        case CTYPE_VOID:
            return true;
            
        case CTYPE_CHAR:
        case CTYPE_SCHAR:
            if (IS_INT(value)) {
                *(char*)out = (char)AS_INT(value);
                return true;
            }
            if (IS_STRING(value) && AS_STRING(value)->length > 0) {
                *(char*)out = AS_STRING(value)->chars[0];
                return true;
            }
            break;
            
        case CTYPE_UCHAR:
        case CTYPE_UINT8:
            if (IS_INT(value)) {
                *(unsigned char*)out = (unsigned char)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_SHORT:
        case CTYPE_INT16:
            if (IS_INT(value)) {
                *(short*)out = (short)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_USHORT:
        case CTYPE_UINT16:
            if (IS_INT(value)) {
                *(unsigned short*)out = (unsigned short)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_INT:
        case CTYPE_INT32:
        case CTYPE_BOOL:
            if (IS_INT(value)) {
                *(int*)out = (int)AS_INT(value);
                return true;
            }
            if (IS_BOOL(value)) {
                *(int*)out = AS_BOOL(value) ? 1 : 0;
                return true;
            }
            break;
            
        case CTYPE_UINT:
        case CTYPE_UINT32:
            if (IS_INT(value)) {
                *(unsigned int*)out = (unsigned int)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_LONG:
        case CTYPE_INT64:
            if (IS_INT(value)) {
                *(long*)out = (long)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_ULONG:
        case CTYPE_UINT64:
        case CTYPE_SIZE_T:
            if (IS_INT(value)) {
                *(unsigned long*)out = (unsigned long)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_LONGLONG:
            if (IS_INT(value)) {
                *(long long*)out = (long long)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_ULONGLONG:
            if (IS_INT(value)) {
                *(unsigned long long*)out = (unsigned long long)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_FLOAT:
            if (IS_INT(value)) {
                *(float*)out = (float)AS_INT(value);
                return true;
            }
            if (IS_FLOAT(value)) {
                *(float*)out = (float)AS_FLOAT(value);
                return true;
            }
            break;
            
        case CTYPE_DOUBLE:
            if (IS_INT(value)) {
                *(double*)out = (double)AS_INT(value);
                return true;
            }
            if (IS_FLOAT(value)) {
                *(double*)out = AS_FLOAT(value);
                return true;
            }
            break;
            
        case CTYPE_STRING:
            if (IS_NIL(value)) {
                *(char**)out = NULL;
                return true;
            }
            if (IS_STRING(value)) {
                *(char**)out = AS_STRING(value)->chars;
                return true;
            }
            break;
            
        case CTYPE_POINTER:
            if (IS_NIL(value)) {
                *(void**)out = NULL;
                return true;
            }
            if (IS_POINTER(value)) {
                *(void**)out = AS_POINTER(value)->ptr;
                return true;
            }
            if (IS_CSTRUCT(value)) {
                *(void**)out = AS_CSTRUCT(value)->data;
                return true;
            }
            if (IS_INT(value)) {
                *(void**)out = (void*)(intptr_t)AS_INT(value);
                return true;
            }
            break;
            
        case CTYPE_STRUCT:
            if (IS_CSTRUCT(value)) {
                *(void**)out = AS_CSTRUCT(value)->data;
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

/* Marshal C value to Brisk */
Value marshal_from_c(void* in, CType type) {
    switch (type) {
        case CTYPE_VOID:
            return NIL_VAL;
            
        case CTYPE_CHAR:
        case CTYPE_SCHAR:
            return INT_VAL(*(char*)in);
            
        case CTYPE_UCHAR:
        case CTYPE_UINT8:
            return INT_VAL(*(unsigned char*)in);
            
        case CTYPE_SHORT:
        case CTYPE_INT16:
            return INT_VAL(*(short*)in);
            
        case CTYPE_USHORT:
        case CTYPE_UINT16:
            return INT_VAL(*(unsigned short*)in);
            
        case CTYPE_INT:
        case CTYPE_INT32:
            return INT_VAL(*(int*)in);
            
        case CTYPE_UINT:
        case CTYPE_UINT32:
            return INT_VAL(*(unsigned int*)in);
            
        case CTYPE_LONG:
        case CTYPE_INT64:
            return INT_VAL(*(long*)in);
            
        case CTYPE_ULONG:
        case CTYPE_UINT64:
        case CTYPE_SIZE_T:
            return INT_VAL(*(unsigned long*)in);
            
        case CTYPE_LONGLONG:
            return INT_VAL(*(long long*)in);
            
        case CTYPE_ULONGLONG:
            return INT_VAL((int64_t)*(unsigned long long*)in);
            
        case CTYPE_FLOAT:
            return FLOAT_VAL(*(float*)in);
            
        case CTYPE_DOUBLE:
            return FLOAT_VAL(*(double*)in);
            
        case CTYPE_BOOL:
            return BOOL_VAL(*(int*)in != 0);
            
        case CTYPE_STRING: {
            char* str = *(char**)in;
            if (str == NULL) return NIL_VAL;
            return OBJ_VAL(string_create(str, strlen(str)));
        }
            
        case CTYPE_POINTER: {
            void* ptr = *(void**)in;
            if (ptr == NULL) return NIL_VAL;
            return OBJ_VAL(pointer_create(ptr, "void*"));
        }
            
        default:
            return NIL_VAL;
    }
}

/* Call a C function */
Value cffi_call(CFunctionDesc* desc, int arg_count, Value* args) {
    if (!desc->cif_prepared) {
        if (!cfunc_prepare(desc)) {
            fprintf(stderr, "FFI Error: Failed to prepare call to %s\n", desc->name);
            return NIL_VAL;
        }
    }
    
    /* Validate argument count */
    if (!desc->is_variadic && arg_count != desc->param_count) {
        fprintf(stderr, "FFI Error: %s expects %d arguments, got %d\n",
                desc->name, desc->param_count, arg_count);
        return NIL_VAL;
    }
    if (desc->is_variadic && arg_count < desc->param_count) {
        fprintf(stderr, "FFI Error: %s expects at least %d arguments, got %d\n",
                desc->name, desc->param_count, arg_count);
        return NIL_VAL;
    }
    
    /* Allocate argument storage */
    void** arg_values = NULL;
    void* arg_storage = NULL;
    
    if (arg_count > 0) {
        arg_values = mem_alloc(sizeof(void*) * arg_count);
        arg_storage = mem_alloc(16 * arg_count);  /* 16 bytes per arg is enough */
        
        for (int i = 0; i < arg_count; i++) {
            CType param_type;
            if (i < desc->param_count) {
                param_type = desc->param_types[i];
            } else {
                /* Variadic args - infer type */
                if (IS_INT(args[i])) param_type = CTYPE_INT;
                else if (IS_FLOAT(args[i])) param_type = CTYPE_DOUBLE;
                else if (IS_STRING(args[i])) param_type = CTYPE_STRING;
                else if (IS_POINTER(args[i])) param_type = CTYPE_POINTER;
                else param_type = CTYPE_INT;
            }
            
            void* storage = (char*)arg_storage + (16 * i);
            arg_values[i] = storage;
            
            if (!marshal_to_c(args[i], param_type, storage)) {
                fprintf(stderr, "FFI Error: Failed to marshal argument %d to %s\n",
                        i, ctype_name(param_type));
                mem_free(arg_values, sizeof(void*) * arg_count);
                mem_free(arg_storage, 16 * arg_count);
                return NIL_VAL;
            }
        }
    }
    
    /* Allocate return value storage */
    long long ret_storage[2] = {0, 0};  /* 16 bytes is enough for any return type */
    
    /* Make the call */
    ffi_call(&desc->cif, FFI_FN(desc->func_ptr), &ret_storage, arg_values);
    
    /* Marshal return value */
    Value result = marshal_from_c(&ret_storage, desc->return_type);
    
    /* Cleanup */
    if (arg_values) mem_free(arg_values, sizeof(void*) * arg_count);
    if (arg_storage) mem_free(arg_storage, 16 * arg_count);
    
    return result;
}

/* Create ObjCFunction from descriptor */
ObjCFunction* cfunction_create(CFunctionDesc* desc) {
    ObjCFunction* cfn = (ObjCFunction*)allocate_object(sizeof(ObjCFunction), OBJ_CFUNCTION);
    cfn->desc = desc;
    return cfn;
}

/* ============ Struct Support ============ */

/* Create a C struct descriptor */
CStructDesc* cstruct_desc_create(const char* name, int field_count) {
    CStructDesc* desc = mem_alloc(sizeof(CStructDesc));
    
    desc->name = mem_alloc(strlen(name) + 1);
    strcpy(desc->name, name);
    
    desc->fields = mem_alloc(sizeof(CFieldDesc) * field_count);
    desc->field_count = field_count;
    desc->size = 0;
    desc->alignment = 0;
    desc->ffi_type_ptr = NULL;
    
    return desc;
}

/* Add field to struct descriptor */
void cstruct_desc_add_field(CStructDesc* desc, int index,
                             const char* name, CType type,
                             int offset, int size) {
    CFieldDesc* field = &desc->fields[index];
    
    field->name = mem_alloc(strlen(name) + 1);
    strcpy(field->name, name);
    
    field->type = type;
    field->offset = offset;
    field->size = size;
    field->struct_type = NULL;
}

/* Finalize struct descriptor */
void cstruct_desc_finalize(CStructDesc* desc) {
    /* Calculate size and alignment */
    int max_align = 1;
    int offset = 0;
    
    for (int i = 0; i < desc->field_count; i++) {
        CFieldDesc* field = &desc->fields[i];
        int field_size = ctype_size(field->type);
        int field_align = field_size > 0 ? field_size : 1;
        
        if (field_align > 8) field_align = 8;  /* Max alignment */
        
        /* Align offset */
        offset = (offset + field_align - 1) & ~(field_align - 1);
        field->offset = offset;
        field->size = field_size;
        
        offset += field_size;
        if (field_align > max_align) max_align = field_align;
    }
    
    /* Align total size */
    desc->size = (offset + max_align - 1) & ~(max_align - 1);
    desc->alignment = max_align;
}

/* Free struct descriptor */
void cstruct_desc_free(CStructDesc* desc) {
    if (desc == NULL) return;
    
    if (desc->name) mem_free(desc->name, strlen(desc->name) + 1);
    
    for (int i = 0; i < desc->field_count; i++) {
        if (desc->fields[i].name) {
            mem_free(desc->fields[i].name, strlen(desc->fields[i].name) + 1);
        }
    }
    
    if (desc->fields) mem_free(desc->fields, sizeof(CFieldDesc) * desc->field_count);
    mem_free(desc, sizeof(CStructDesc));
}

/* Create a C struct instance */
ObjCStruct* cstruct_create(CStructDesc* desc) {
    ObjCStruct* obj = (ObjCStruct*)allocate_object(sizeof(ObjCStruct), OBJ_CSTRUCT);
    obj->desc = desc;
    obj->data = calloc(1, desc->size);  /* Zero-initialize */
    return obj;
}

/* Get field from C struct */
Value cstruct_get_field(ObjCStruct* obj, const char* name) {
    CStructDesc* desc = obj->desc;
    
    for (int i = 0; i < desc->field_count; i++) {
        if (strcmp(desc->fields[i].name, name) == 0) {
            void* field_ptr = (char*)obj->data + desc->fields[i].offset;
            return marshal_from_c(field_ptr, desc->fields[i].type);
        }
    }
    
    return NIL_VAL;
}

/* Set field in C struct */
bool cstruct_set_field(ObjCStruct* obj, const char* name, Value value) {
    CStructDesc* desc = obj->desc;
    
    for (int i = 0; i < desc->field_count; i++) {
        if (strcmp(desc->fields[i].name, name) == 0) {
            void* field_ptr = (char*)obj->data + desc->fields[i].offset;
            return marshal_to_c(value, desc->fields[i].type, field_ptr);
        }
    }
    
    return false;
}

/* Get raw pointer to struct data */
void* cstruct_data_ptr(ObjCStruct* obj) {
    return obj->data;
}
