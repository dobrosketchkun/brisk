/*
 * Brisk Language - Value System Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "memory.h"
#include "cffi.h"

/* Global object list for GC */
Object* all_objects = NULL;

/* String interning table */
static ObjTable* string_table = NULL;

/* Reference counting */
void obj_incref(Object* obj) {
    if (obj == NULL) return;
    obj->ref_count++;
}

void obj_decref(Object* obj) {
    if (obj == NULL) return;
    obj->ref_count--;
    if (obj->ref_count <= 0) {
        free_object(obj);
    }
}

/* Object allocation */
Object* allocate_object(size_t size, ObjectType type) {
    Object* obj = (Object*)mem_alloc(size);
    obj->type = type;
    obj->ref_count = 1;
    obj->marked = false;
    obj->next = all_objects;
    all_objects = obj;
    return obj;
}

/* String hash function (FNV-1a) */
uint32_t string_hash(const char* chars, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)chars[i];
        hash *= 16777619;
    }
    return hash;
}

/* Create a string */
ObjString* string_create(const char* chars, int length) {
    uint32_t hash = string_hash(chars, length);
    
    /* Check if string already interned */
    if (string_table != NULL) {
        /* Simple linear search for now */
        for (int i = 0; i < string_table->capacity; i++) {
            TableEntry* entry = &string_table->entries[i];
            if (entry->key != NULL &&
                entry->key->length == length &&
                entry->key->hash == hash &&
                memcmp(entry->key->chars, chars, length) == 0) {
                obj_incref((Object*)entry->key);
                return entry->key;
            }
        }
    }
    
    /* Allocate new string */
    ObjString* string = (ObjString*)allocate_object(
        sizeof(ObjString) + length + 1, OBJ_STRING);
    string->length = length;
    string->hash = hash;
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';
    
    /* Intern the string */
    if (string_table == NULL) {
        string_table = table_create();
    }
    obj_incref((Object*)string);  /* Table holds a reference */
    table_set(string_table, string, NIL_VAL, false);
    
    return string;
}

/* Concatenate two strings */
ObjString* string_concat(ObjString* a, ObjString* b) {
    int length = a->length + b->length;
    char* chars = mem_alloc(length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';
    
    ObjString* result = string_create(chars, length);
    mem_free(chars, length + 1);
    return result;
}

/* Intern a string */
ObjString* string_intern(ObjString* string) {
    /* Already interned during creation */
    return string;
}

/* Create an array */
ObjArray* array_create(void) {
    ObjArray* array = (ObjArray*)allocate_object(sizeof(ObjArray), OBJ_ARRAY);
    array->elements = NULL;
    array->count = 0;
    array->capacity = 0;
    return array;
}

/* Push to array */
void array_push(ObjArray* array, Value value) {
    if (array->count >= array->capacity) {
        int new_capacity = array->capacity < 8 ? 8 : array->capacity * 2;
        array->elements = mem_realloc(array->elements, 
                                       sizeof(Value) * array->capacity,
                                       sizeof(Value) * new_capacity);
        array->capacity = new_capacity;
    }
    array->elements[array->count++] = value;
    
    /* Incref if object */
    if (IS_OBJ(value)) {
        obj_incref(AS_OBJ(value));
    }
}

/* Pop from array */
Value array_pop(ObjArray* array) {
    if (array->count == 0) {
        return NIL_VAL;
    }
    Value value = array->elements[--array->count];
    /* Don't decref - caller now owns the reference */
    return value;
}

/* Get array element */
Value array_get(ObjArray* array, int index) {
    if (index < 0 || index >= array->count) {
        return NIL_VAL;
    }
    return array->elements[index];
}

/* Set array element */
void array_set(ObjArray* array, int index, Value value) {
    if (index < 0 || index >= array->count) {
        return;
    }
    
    /* Decref old value */
    Value old = array->elements[index];
    if (IS_OBJ(old)) {
        obj_decref(AS_OBJ(old));
    }
    
    /* Incref new value */
    if (IS_OBJ(value)) {
        obj_incref(AS_OBJ(value));
    }
    
    array->elements[index] = value;
}

/* Get array length */
int array_length(ObjArray* array) {
    return array->count;
}

/* Create a table */
ObjTable* table_create(void) {
    ObjTable* table = (ObjTable*)allocate_object(sizeof(ObjTable), OBJ_TABLE);
    table->entries = NULL;
    table->count = 0;
    table->capacity = 0;
    return table;
}

/* Find table entry */
static TableEntry* find_entry(TableEntry* entries, int capacity, ObjString* key) {
    if (capacity == 0) return NULL;
    
    uint32_t index = key->hash % capacity;
    TableEntry* tombstone = NULL;
    
    for (;;) {
        TableEntry* entry = &entries[index];
        
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                /* Empty entry - return tombstone if found */
                return tombstone != NULL ? tombstone : entry;
            } else {
                /* Tombstone */
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }
        
        index = (index + 1) % capacity;
    }
}

/* Grow table */
static void grow_table(ObjTable* table, int capacity) {
    TableEntry* entries = mem_alloc(sizeof(TableEntry) * capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
        entries[i].is_const = false;
    }
    
    /* Rehash existing entries */
    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        TableEntry* entry = &table->entries[i];
        if (entry->key == NULL) continue;
        
        TableEntry* dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        dest->is_const = entry->is_const;
        table->count++;
    }
    
    /* Free old entries */
    if (table->entries != NULL) {
        mem_free(table->entries, sizeof(TableEntry) * table->capacity);
    }
    
    table->entries = entries;
    table->capacity = capacity;
}

/* Get from table */
bool table_get(ObjTable* table, ObjString* key, Value* value) {
    if (table->count == 0) return false;
    
    TableEntry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;
    
    *value = entry->value;
    return true;
}

/* Set in table */
bool table_set(ObjTable* table, ObjString* key, Value value, bool is_const) {
    if (table->count + 1 > table->capacity * 0.75) {
        int capacity = table->capacity < 8 ? 8 : table->capacity * 2;
        grow_table(table, capacity);
    }
    
    TableEntry* entry = find_entry(table->entries, table->capacity, key);
    bool is_new = entry->key == NULL;
    
    if (is_new && IS_NIL(entry->value)) {
        table->count++;
        obj_incref((Object*)key);
    }
    
    /* Decref old value */
    if (!is_new && IS_OBJ(entry->value)) {
        obj_decref(AS_OBJ(entry->value));
    }
    
    /* Incref new value */
    if (IS_OBJ(value)) {
        obj_incref(AS_OBJ(value));
    }
    
    entry->key = key;
    entry->value = value;
    entry->is_const = is_const;
    
    return is_new;
}

/* Delete from table */
bool table_delete(ObjTable* table, ObjString* key) {
    if (table->count == 0) return false;
    
    TableEntry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;
    
    /* Decref key and value */
    obj_decref((Object*)entry->key);
    if (IS_OBJ(entry->value)) {
        obj_decref(AS_OBJ(entry->value));
    }
    
    /* Place tombstone */
    entry->key = NULL;
    entry->value = BOOL_VAL(true);  /* Tombstone marker */
    
    return true;
}

/* Get all keys */
ObjArray* table_keys(ObjTable* table) {
    ObjArray* keys = array_create();
    
    for (int i = 0; i < table->capacity; i++) {
        TableEntry* entry = &table->entries[i];
        if (entry->key != NULL) {
            array_push(keys, OBJ_VAL(entry->key));
        }
    }
    
    return keys;
}

/* Get all values */
ObjArray* table_values(ObjTable* table) {
    ObjArray* values = array_create();
    
    for (int i = 0; i < table->capacity; i++) {
        TableEntry* entry = &table->entries[i];
        if (entry->key != NULL) {
            array_push(values, entry->value);
        }
    }
    
    return values;
}

/* Check if key exists */
bool table_has(ObjTable* table, ObjString* key) {
    if (table->count == 0) return false;
    
    TableEntry* entry = find_entry(table->entries, table->capacity, key);
    return entry->key != NULL;
}

/* Create a function */
ObjFunction* function_create(const char* name, int name_len,
                              char** params, int* param_lens, int arity,
                              AstNode* body, Environment* closure) {
    ObjFunction* fn = (ObjFunction*)allocate_object(sizeof(ObjFunction), OBJ_FUNCTION);
    
    if (name != NULL) {
        fn->name = mem_alloc(name_len + 1);
        memcpy(fn->name, name, name_len);
        fn->name[name_len] = '\0';
    } else {
        fn->name = NULL;
    }
    
    fn->arity = arity;
    fn->body = body;
    fn->params = params;
    fn->param_lengths = param_lens;
    fn->closure = closure;
    
    return fn;
}

/* Create a native function */
ObjNative* native_create(NativeFn function, int arity, const char* name) {
    ObjNative* native = (ObjNative*)allocate_object(sizeof(ObjNative), OBJ_NATIVE);
    native->function = function;
    native->arity = arity;
    native->name = name;
    return native;
}

/* Create a pointer */
ObjPointer* pointer_create(void* ptr, const char* type_name) {
    ObjPointer* pointer = (ObjPointer*)allocate_object(sizeof(ObjPointer), OBJ_POINTER);
    pointer->ptr = ptr;
    if (type_name != NULL) {
        pointer->type_name = mem_alloc(strlen(type_name) + 1);
        strcpy(pointer->type_name, type_name);
    } else {
        pointer->type_name = NULL;
    }
    return pointer;
}

/* Check value equality */
bool value_equals(Value a, Value b) {
    if (a.type != b.type) {
        /* Allow int/float comparison */
        if (IS_NUMBER(a) && IS_NUMBER(b)) {
            return AS_NUMBER(a) == AS_NUMBER(b);
        }
        return false;
    }
    
    switch (a.type) {
        case VAL_NIL: return true;
        case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
        case VAL_INT: return AS_INT(a) == AS_INT(b);
        case VAL_FLOAT: return AS_FLOAT(a) == AS_FLOAT(b);
        case VAL_OBJ: {
            if (AS_OBJ(a)->type != AS_OBJ(b)->type) return false;
            
            switch (AS_OBJ(a)->type) {
                case OBJ_STRING: {
                    ObjString* sa = AS_STRING(a);
                    ObjString* sb = AS_STRING(b);
                    return sa->length == sb->length &&
                           sa->hash == sb->hash &&
                           memcmp(sa->chars, sb->chars, sa->length) == 0;
                }
                default:
                    /* For other objects, compare by identity */
                    return AS_OBJ(a) == AS_OBJ(b);
            }
        }
        default: return false;
    }
}

/* Print value */
void value_print(Value value) {
    switch (value.type) {
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_INT:
            printf("%ld", (long)AS_INT(value));
            break;
        case VAL_FLOAT:
            printf("%g", AS_FLOAT(value));
            break;
        case VAL_OBJ:
            switch (AS_OBJ(value)->type) {
                case OBJ_STRING:
                    printf("%s", AS_CSTRING(value));
                    break;
                case OBJ_ARRAY: {
                    ObjArray* arr = AS_ARRAY(value);
                    printf("[");
                    for (int i = 0; i < arr->count; i++) {
                        if (i > 0) printf(", ");
                        value_print(arr->elements[i]);
                    }
                    printf("]");
                    break;
                }
                case OBJ_TABLE:
                    printf("<table>");
                    break;
                case OBJ_FUNCTION: {
                    ObjFunction* fn = AS_FUNCTION(value);
                    if (fn->name != NULL) {
                        printf("<fn %s>", fn->name);
                    } else {
                        printf("<fn>");
                    }
                    break;
                }
                case OBJ_NATIVE:
                    printf("<native fn>");
                    break;
                case OBJ_POINTER:
                    printf("<ptr %p>", AS_POINTER(value)->ptr);
                    break;
                case OBJ_CSTRUCT:
                    printf("<cstruct>");
                    break;
                case OBJ_CFUNCTION:
                    printf("<cfn>");
                    break;
            }
            break;
    }
}

/* Convert value to string */
char* value_to_string(Value value) {
    char* buffer = mem_alloc(256);
    
    switch (value.type) {
        case VAL_NIL:
            strcpy(buffer, "nil");
            break;
        case VAL_BOOL:
            strcpy(buffer, AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_INT:
            snprintf(buffer, 256, "%ld", (long)AS_INT(value));
            break;
        case VAL_FLOAT:
            snprintf(buffer, 256, "%g", AS_FLOAT(value));
            break;
        case VAL_OBJ:
            if (AS_OBJ(value)->type == OBJ_STRING) {
                ObjString* str = AS_STRING(value);
                mem_free(buffer, 256);
                buffer = mem_alloc(str->length + 1);
                memcpy(buffer, str->chars, str->length);
                buffer[str->length] = '\0';
            } else {
                snprintf(buffer, 256, "<%s>", 
                         value_type_name(value));
            }
            break;
    }
    
    return buffer;
}

/* Check truthiness */
bool value_is_truthy(Value value) {
    switch (value.type) {
        case VAL_NIL: return false;
        case VAL_BOOL: return AS_BOOL(value);
        case VAL_INT: return AS_INT(value) != 0;
        case VAL_FLOAT: return AS_FLOAT(value) != 0.0;
        case VAL_OBJ: return true;  /* All objects are truthy */
        default: return false;
    }
}

/* Get value type name */
const char* value_type_name(Value value) {
    switch (value.type) {
        case VAL_NIL: return "nil";
        case VAL_BOOL: return "bool";
        case VAL_INT: return "int";
        case VAL_FLOAT: return "float";
        case VAL_OBJ:
            switch (AS_OBJ(value)->type) {
                case OBJ_STRING: return "string";
                case OBJ_ARRAY: return "array";
                case OBJ_TABLE: return "table";
                case OBJ_FUNCTION: return "function";
                case OBJ_NATIVE: return "native";
                case OBJ_POINTER: return "pointer";
                case OBJ_CSTRUCT: return "cstruct";
                case OBJ_CFUNCTION: return "cfunction";
                default: return "unknown";
            }
        default: return "unknown";
    }
}

/* Free an object */
void free_object(Object* obj) {
    switch (obj->type) {
        case OBJ_STRING: {
            ObjString* str = (ObjString*)obj;
            mem_free(obj, sizeof(ObjString) + str->length + 1);
            break;
        }
        case OBJ_ARRAY: {
            ObjArray* arr = (ObjArray*)obj;
            /* Decref all elements */
            for (int i = 0; i < arr->count; i++) {
                if (IS_OBJ(arr->elements[i])) {
                    obj_decref(AS_OBJ(arr->elements[i]));
                }
            }
            if (arr->elements != NULL) {
                mem_free(arr->elements, sizeof(Value) * arr->capacity);
            }
            mem_free(obj, sizeof(ObjArray));
            break;
        }
        case OBJ_TABLE: {
            ObjTable* table = (ObjTable*)obj;
            /* Decref all keys and values */
            for (int i = 0; i < table->capacity; i++) {
                TableEntry* entry = &table->entries[i];
                if (entry->key != NULL) {
                    obj_decref((Object*)entry->key);
                    if (IS_OBJ(entry->value)) {
                        obj_decref(AS_OBJ(entry->value));
                    }
                }
            }
            if (table->entries != NULL) {
                mem_free(table->entries, sizeof(TableEntry) * table->capacity);
            }
            mem_free(obj, sizeof(ObjTable));
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* fn = (ObjFunction*)obj;
            if (fn->name != NULL) {
                mem_free(fn->name, strlen(fn->name) + 1);
            }
            /* Note: params are owned by AST, don't free */
            mem_free(obj, sizeof(ObjFunction));
            break;
        }
        case OBJ_NATIVE: {
            mem_free(obj, sizeof(ObjNative));
            break;
        }
        case OBJ_POINTER: {
            ObjPointer* ptr = (ObjPointer*)obj;
            if (ptr->type_name != NULL) {
                mem_free(ptr->type_name, strlen(ptr->type_name) + 1);
            }
            mem_free(obj, sizeof(ObjPointer));
            break;
        }
        case OBJ_CSTRUCT: {
            ObjCStruct* cs = (ObjCStruct*)obj;
            if (cs->data != NULL) {
                /* Size unknown, use free directly */
                free(cs->data);
            }
            mem_free(obj, sizeof(ObjCStruct));
            break;
        }
        case OBJ_CFUNCTION: {
            mem_free(obj, sizeof(ObjCFunction));
            break;
        }
    }
}
