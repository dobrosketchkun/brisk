/*
 * Brisk Language - Value System
 */

#ifndef BRISK_VALUE_H
#define BRISK_VALUE_H

#include <stdint.h>
#include <stdbool.h>
#include "ast.h"

/* Forward declarations */
typedef struct Object Object;
typedef struct ObjString ObjString;
typedef struct ObjArray ObjArray;
typedef struct ObjTable ObjTable;
typedef struct ObjFunction ObjFunction;
typedef struct ObjNative ObjNative;
typedef struct ObjPointer ObjPointer;
typedef struct ObjCStruct ObjCStruct;
typedef struct ObjCFunction ObjCFunction;
typedef struct Environment Environment;

/* Value types */
typedef enum {
    VAL_NIL,
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
    VAL_OBJ
} ValueType;

/* Object types (for heap-allocated values) */
typedef enum {
    OBJ_STRING,
    OBJ_ARRAY,
    OBJ_TABLE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_POINTER,
    OBJ_CSTRUCT,
    OBJ_CFUNCTION
} ObjectType;

/* Value structure */
typedef struct {
    ValueType type;
    union {
        bool boolean;
        int64_t integer;
        double floating;
        Object* object;
    } as;
} Value;

/* Object base (header for all heap objects) */
struct Object {
    ObjectType type;
    int ref_count;
    Object* next;  /* For GC list */
    bool marked;   /* For cycle detection */
};

/* String object */
struct ObjString {
    Object obj;
    int length;
    uint32_t hash;
    char chars[];  /* Flexible array member */
};

/* Array object */
struct ObjArray {
    Object obj;
    Value* elements;
    int count;
    int capacity;
};

/* Table entry */
typedef struct {
    ObjString* key;
    Value value;
    bool is_const;  /* For constant entries */
} TableEntry;

/* Table object */
struct ObjTable {
    Object obj;
    TableEntry* entries;
    int count;
    int capacity;
};

/* Native function type */
typedef Value (*NativeFn)(int arg_count, Value* args);

/* Function object */
struct ObjFunction {
    Object obj;
    char* name;
    int arity;
    AstNode* body;           /* AST node for body */
    char** params;           /* Parameter names */
    int* param_lengths;
    Environment* closure;    /* Captured environment */
};

/* Native function object */
struct ObjNative {
    Object obj;
    NativeFn function;
    int arity;  /* -1 for variadic */
    const char* name;
};

/* Pointer object (for C interop) */
struct ObjPointer {
    Object obj;
    void* ptr;
    char* type_name;  /* C type name for debugging */
};

/* C struct descriptor (forward declaration, defined in cffi.h) */
typedef struct CStructDesc CStructDesc;

/* C struct object */
struct ObjCStruct {
    Object obj;
    CStructDesc* desc;
    void* data;  /* Raw C memory */
};

/* Value creation macros */
#define NIL_VAL           ((Value){VAL_NIL, {.integer = 0}})
#define BOOL_VAL(b)       ((Value){VAL_BOOL, {.boolean = (b)}})
#define INT_VAL(n)        ((Value){VAL_INT, {.integer = (n)}})
#define FLOAT_VAL(n)      ((Value){VAL_FLOAT, {.floating = (n)}})
#define OBJ_VAL(o)        ((Value){VAL_OBJ, {.object = (Object*)(o)}})

/* Value type check macros */
#define IS_NIL(v)         ((v).type == VAL_NIL)
#define IS_BOOL(v)        ((v).type == VAL_BOOL)
#define IS_INT(v)         ((v).type == VAL_INT)
#define IS_FLOAT(v)       ((v).type == VAL_FLOAT)
#define IS_NUMBER(v)      ((v).type == VAL_INT || (v).type == VAL_FLOAT)
#define IS_OBJ(v)         ((v).type == VAL_OBJ)

/* Object type check macros */
#define OBJ_TYPE(v)       ((v).as.object->type)
#define IS_STRING(v)      (IS_OBJ(v) && OBJ_TYPE(v) == OBJ_STRING)
#define IS_ARRAY(v)       (IS_OBJ(v) && OBJ_TYPE(v) == OBJ_ARRAY)
#define IS_TABLE(v)       (IS_OBJ(v) && OBJ_TYPE(v) == OBJ_TABLE)
#define IS_FUNCTION(v)    (IS_OBJ(v) && OBJ_TYPE(v) == OBJ_FUNCTION)
#define IS_NATIVE(v)      (IS_OBJ(v) && OBJ_TYPE(v) == OBJ_NATIVE)
#define IS_POINTER(v)     (IS_OBJ(v) && OBJ_TYPE(v) == OBJ_POINTER)
#define IS_CSTRUCT(v)     (IS_OBJ(v) && OBJ_TYPE(v) == OBJ_CSTRUCT)
#define IS_CFUNCTION(v)   (IS_OBJ(v) && OBJ_TYPE(v) == OBJ_CFUNCTION)

/* Value extraction macros */
#define AS_BOOL(v)        ((v).as.boolean)
#define AS_INT(v)         ((v).as.integer)
#define AS_FLOAT(v)       ((v).as.floating)
#define AS_OBJ(v)         ((v).as.object)
#define AS_STRING(v)      ((ObjString*)AS_OBJ(v))
#define AS_CSTRING(v)     (((ObjString*)AS_OBJ(v))->chars)
#define AS_ARRAY(v)       ((ObjArray*)AS_OBJ(v))
#define AS_TABLE(v)       ((ObjTable*)AS_OBJ(v))
#define AS_FUNCTION(v)    ((ObjFunction*)AS_OBJ(v))
#define AS_NATIVE(v)      ((ObjNative*)AS_OBJ(v))
#define AS_POINTER(v)     ((ObjPointer*)AS_OBJ(v))
#define AS_CSTRUCT(v)     ((ObjCStruct*)AS_OBJ(v))
#define AS_CFUNCTION(v)   ((ObjCFunction*)AS_OBJ(v))

/* Get number as double (works for int or float) */
#define AS_NUMBER(v)      (IS_INT(v) ? (double)AS_INT(v) : AS_FLOAT(v))

/* Reference counting */
void obj_incref(Object* obj);
void obj_decref(Object* obj);

/* Object allocation */
Object* allocate_object(size_t size, ObjectType type);

/* String operations */
ObjString* string_create(const char* chars, int length);
ObjString* string_concat(ObjString* a, ObjString* b);
uint32_t string_hash(const char* chars, int length);
ObjString* string_intern(ObjString* string);

/* Array operations */
ObjArray* array_create(void);
void array_push(ObjArray* array, Value value);
Value array_pop(ObjArray* array);
Value array_get(ObjArray* array, int index);
void array_set(ObjArray* array, int index, Value value);
int array_length(ObjArray* array);

/* Table operations */
ObjTable* table_create(void);
bool table_get(ObjTable* table, ObjString* key, Value* value);
bool table_set(ObjTable* table, ObjString* key, Value value, bool is_const);
bool table_delete(ObjTable* table, ObjString* key);
ObjArray* table_keys(ObjTable* table);
ObjArray* table_values(ObjTable* table);
bool table_has(ObjTable* table, ObjString* key);

/* Function operations */
ObjFunction* function_create(const char* name, int name_len, 
                              char** params, int* param_lens, int arity,
                              AstNode* body, Environment* closure);

/* Native function operations */
ObjNative* native_create(NativeFn function, int arity, const char* name);

/* Pointer operations */
ObjPointer* pointer_create(void* ptr, const char* type_name);

/* Value operations */
bool value_equals(Value a, Value b);
void value_print(Value value);
char* value_to_string(Value value);
bool value_is_truthy(Value value);
const char* value_type_name(Value value);

/* Free object */
void free_object(Object* obj);

/* Global object list for GC */
extern Object* all_objects;

#endif /* BRISK_VALUE_H */
