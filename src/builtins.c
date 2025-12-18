/*
 * Brisk Language - Built-in Functions Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "builtins.h"
#include "value.h"
#include "memory.h"

/* Helper to register a native function */
static void register_native(Environment* env, const char* name, NativeFn fn, int arity) {
    ObjNative* native = native_create(fn, arity, name);
    env_define(env, name, strlen(name), OBJ_VAL(native), true);
}

/* ============ I/O Functions ============ */

static Value native_print(int arg_count, Value* args) {
    for (int i = 0; i < arg_count; i++) {
        if (i > 0) printf(" ");
        value_print(args[i]);
    }
    return NIL_VAL;
}

static Value native_println(int arg_count, Value* args) {
    for (int i = 0; i < arg_count; i++) {
        if (i > 0) printf(" ");
        value_print(args[i]);
    }
    printf("\n");
    return NIL_VAL;
}

static Value native_input(int arg_count, Value* args) {
    if (arg_count > 0 && IS_STRING(args[0])) {
        printf("%s", AS_CSTRING(args[0]));
        fflush(stdout);
    }
    
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return NIL_VAL;
    }
    
    /* Remove trailing newline */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
    }
    
    return OBJ_VAL(string_create(buffer, len));
}

/* ============ Type Functions ============ */

static Value native_type(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    
    const char* type_name = value_type_name(args[0]);
    return OBJ_VAL(string_create(type_name, strlen(type_name)));
}

static Value native_int(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    
    Value val = args[0];
    if (IS_INT(val)) return val;
    if (IS_FLOAT(val)) return INT_VAL((int64_t)AS_FLOAT(val));
    if (IS_BOOL(val)) return INT_VAL(AS_BOOL(val) ? 1 : 0);
    if (IS_STRING(val)) {
        char* end;
        int64_t result = strtoll(AS_CSTRING(val), &end, 10);
        if (*end == '\0') return INT_VAL(result);
    }
    return NIL_VAL;
}

static Value native_float(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    
    Value val = args[0];
    if (IS_FLOAT(val)) return val;
    if (IS_INT(val)) return FLOAT_VAL((double)AS_INT(val));
    if (IS_STRING(val)) {
        char* end;
        double result = strtod(AS_CSTRING(val), &end);
        if (*end == '\0') return FLOAT_VAL(result);
    }
    return NIL_VAL;
}

static Value native_str(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    
    char* str = value_to_string(args[0]);
    ObjString* result = string_create(str, strlen(str));
    mem_free(str, 256);
    return OBJ_VAL(result);
}

static Value native_bool(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    return BOOL_VAL(value_is_truthy(args[0]));
}

/* ============ Array Functions ============ */

static Value native_len(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    
    Value val = args[0];
    if (IS_STRING(val)) return INT_VAL(AS_STRING(val)->length);
    if (IS_ARRAY(val)) return INT_VAL(AS_ARRAY(val)->count);
    if (IS_TABLE(val)) return INT_VAL(AS_TABLE(val)->count);
    return NIL_VAL;
}

static Value native_push(int arg_count, Value* args) {
    if (arg_count != 2) return NIL_VAL;
    if (!IS_ARRAY(args[0])) return NIL_VAL;
    
    array_push(AS_ARRAY(args[0]), args[1]);
    return NIL_VAL;
}

static Value native_pop(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    if (!IS_ARRAY(args[0])) return NIL_VAL;
    
    return array_pop(AS_ARRAY(args[0]));
}

static Value native_first(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    if (!IS_ARRAY(args[0])) return NIL_VAL;
    
    ObjArray* arr = AS_ARRAY(args[0]);
    if (arr->count == 0) return NIL_VAL;
    return arr->elements[0];
}

static Value native_last(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    if (!IS_ARRAY(args[0])) return NIL_VAL;
    
    ObjArray* arr = AS_ARRAY(args[0]);
    if (arr->count == 0) return NIL_VAL;
    return arr->elements[arr->count - 1];
}

static Value native_insert(int arg_count, Value* args) {
    if (arg_count != 3) return NIL_VAL;
    if (!IS_ARRAY(args[0]) || !IS_INT(args[1])) return NIL_VAL;
    
    ObjArray* arr = AS_ARRAY(args[0]);
    int index = (int)AS_INT(args[1]);
    Value value = args[2];
    
    if (index < 0 || index > arr->count) return NIL_VAL;
    
    /* Make room */
    array_push(arr, NIL_VAL);
    
    /* Shift elements */
    for (int i = arr->count - 1; i > index; i--) {
        arr->elements[i] = arr->elements[i - 1];
    }
    
    arr->elements[index] = value;
    if (IS_OBJ(value)) obj_incref(AS_OBJ(value));
    
    return NIL_VAL;
}

static Value native_remove(int arg_count, Value* args) {
    if (arg_count != 2) return NIL_VAL;
    if (!IS_ARRAY(args[0]) || !IS_INT(args[1])) return NIL_VAL;
    
    ObjArray* arr = AS_ARRAY(args[0]);
    int index = (int)AS_INT(args[1]);
    
    if (index < 0 || index >= arr->count) return NIL_VAL;
    
    Value removed = arr->elements[index];
    
    /* Shift elements */
    for (int i = index; i < arr->count - 1; i++) {
        arr->elements[i] = arr->elements[i + 1];
    }
    arr->count--;
    
    return removed;
}

/* ============ String Functions ============ */

static Value native_substr(int arg_count, Value* args) {
    if (arg_count < 2 || arg_count > 3) return NIL_VAL;
    if (!IS_STRING(args[0]) || !IS_INT(args[1])) return NIL_VAL;
    
    ObjString* str = AS_STRING(args[0]);
    int start = (int)AS_INT(args[1]);
    int length = (arg_count == 3 && IS_INT(args[2])) 
                 ? (int)AS_INT(args[2]) 
                 : str->length - start;
    
    if (start < 0) start = 0;
    if (start >= str->length) return OBJ_VAL(string_create("", 0));
    if (start + length > str->length) length = str->length - start;
    
    return OBJ_VAL(string_create(str->chars + start, length));
}

static Value native_find(int arg_count, Value* args) {
    if (arg_count != 2) return NIL_VAL;
    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) return NIL_VAL;
    
    ObjString* haystack = AS_STRING(args[0]);
    ObjString* needle = AS_STRING(args[1]);
    
    char* found = strstr(haystack->chars, needle->chars);
    if (found == NULL) return INT_VAL(-1);
    
    return INT_VAL(found - haystack->chars);
}

static Value native_replace(int arg_count, Value* args) {
    if (arg_count != 3) return NIL_VAL;
    if (!IS_STRING(args[0]) || !IS_STRING(args[1]) || !IS_STRING(args[2])) return NIL_VAL;
    
    ObjString* str = AS_STRING(args[0]);
    ObjString* old = AS_STRING(args[1]);
    ObjString* new = AS_STRING(args[2]);
    
    /* Count occurrences */
    int count = 0;
    char* pos = str->chars;
    while ((pos = strstr(pos, old->chars)) != NULL) {
        count++;
        pos += old->length;
    }
    
    if (count == 0) {
        obj_incref((Object*)str);
        return OBJ_VAL(str);
    }
    
    /* Build new string */
    int new_len = str->length + count * (new->length - old->length);
    char* result = mem_alloc(new_len + 1);
    
    char* src = str->chars;
    char* dst = result;
    while ((pos = strstr(src, old->chars)) != NULL) {
        int before = pos - src;
        memcpy(dst, src, before);
        dst += before;
        memcpy(dst, new->chars, new->length);
        dst += new->length;
        src = pos + old->length;
    }
    strcpy(dst, src);
    
    ObjString* result_str = string_create(result, new_len);
    mem_free(result, new_len + 1);
    return OBJ_VAL(result_str);
}

static Value native_split(int arg_count, Value* args) {
    if (arg_count != 2) return NIL_VAL;
    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) return NIL_VAL;
    
    ObjString* str = AS_STRING(args[0]);
    ObjString* delim = AS_STRING(args[1]);
    
    ObjArray* result = array_create();
    
    if (delim->length == 0) {
        /* Split into characters */
        for (int i = 0; i < str->length; i++) {
            array_push(result, OBJ_VAL(string_create(&str->chars[i], 1)));
        }
    } else {
        char* start = str->chars;
        char* pos;
        while ((pos = strstr(start, delim->chars)) != NULL) {
            array_push(result, OBJ_VAL(string_create(start, pos - start)));
            start = pos + delim->length;
        }
        array_push(result, OBJ_VAL(string_create(start, strlen(start))));
    }
    
    return OBJ_VAL(result);
}

static Value native_join(int arg_count, Value* args) {
    if (arg_count != 2) return NIL_VAL;
    if (!IS_ARRAY(args[0]) || !IS_STRING(args[1])) return NIL_VAL;
    
    ObjArray* arr = AS_ARRAY(args[0]);
    ObjString* delim = AS_STRING(args[1]);
    
    if (arr->count == 0) return OBJ_VAL(string_create("", 0));
    
    /* Calculate total length */
    int total_len = 0;
    for (int i = 0; i < arr->count; i++) {
        if (IS_STRING(arr->elements[i])) {
            total_len += AS_STRING(arr->elements[i])->length;
        }
        if (i > 0) total_len += delim->length;
    }
    
    char* result = mem_alloc(total_len + 1);
    char* pos = result;
    
    for (int i = 0; i < arr->count; i++) {
        if (i > 0) {
            memcpy(pos, delim->chars, delim->length);
            pos += delim->length;
        }
        if (IS_STRING(arr->elements[i])) {
            ObjString* s = AS_STRING(arr->elements[i]);
            memcpy(pos, s->chars, s->length);
            pos += s->length;
        }
    }
    *pos = '\0';
    
    ObjString* result_str = string_create(result, total_len);
    mem_free(result, total_len + 1);
    return OBJ_VAL(result_str);
}

static Value native_upper(int arg_count, Value* args) {
    if (arg_count != 1 || !IS_STRING(args[0])) return NIL_VAL;
    
    ObjString* str = AS_STRING(args[0]);
    char* result = mem_alloc(str->length + 1);
    
    for (int i = 0; i < str->length; i++) {
        char c = str->chars[i];
        result[i] = (c >= 'a' && c <= 'z') ? c - 32 : c;
    }
    result[str->length] = '\0';
    
    ObjString* result_str = string_create(result, str->length);
    mem_free(result, str->length + 1);
    return OBJ_VAL(result_str);
}

static Value native_lower(int arg_count, Value* args) {
    if (arg_count != 1 || !IS_STRING(args[0])) return NIL_VAL;
    
    ObjString* str = AS_STRING(args[0]);
    char* result = mem_alloc(str->length + 1);
    
    for (int i = 0; i < str->length; i++) {
        char c = str->chars[i];
        result[i] = (c >= 'A' && c <= 'Z') ? c + 32 : c;
    }
    result[str->length] = '\0';
    
    ObjString* result_str = string_create(result, str->length);
    mem_free(result, str->length + 1);
    return OBJ_VAL(result_str);
}

static Value native_trim(int arg_count, Value* args) {
    if (arg_count != 1 || !IS_STRING(args[0])) return NIL_VAL;
    
    ObjString* str = AS_STRING(args[0]);
    
    int start = 0;
    while (start < str->length && 
           (str->chars[start] == ' ' || str->chars[start] == '\t' ||
            str->chars[start] == '\n' || str->chars[start] == '\r')) {
        start++;
    }
    
    int end = str->length;
    while (end > start && 
           (str->chars[end-1] == ' ' || str->chars[end-1] == '\t' ||
            str->chars[end-1] == '\n' || str->chars[end-1] == '\r')) {
        end--;
    }
    
    return OBJ_VAL(string_create(str->chars + start, end - start));
}

/* ============ Math Functions ============ */

static Value native_abs(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    if (IS_INT(args[0])) {
        int64_t n = AS_INT(args[0]);
        return INT_VAL(n < 0 ? -n : n);
    }
    if (IS_FLOAT(args[0])) {
        return FLOAT_VAL(fabs(AS_FLOAT(args[0])));
    }
    return NIL_VAL;
}

static Value native_min(int arg_count, Value* args) {
    if (arg_count < 2) return NIL_VAL;
    
    Value min = args[0];
    for (int i = 1; i < arg_count; i++) {
        if (IS_NUMBER(args[i]) && IS_NUMBER(min)) {
            if (AS_NUMBER(args[i]) < AS_NUMBER(min)) {
                min = args[i];
            }
        }
    }
    return min;
}

static Value native_max(int arg_count, Value* args) {
    if (arg_count < 2) return NIL_VAL;
    
    Value max = args[0];
    for (int i = 1; i < arg_count; i++) {
        if (IS_NUMBER(args[i]) && IS_NUMBER(max)) {
            if (AS_NUMBER(args[i]) > AS_NUMBER(max)) {
                max = args[i];
            }
        }
    }
    return max;
}

static Value native_floor(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    if (IS_INT(args[0])) return args[0];
    if (IS_FLOAT(args[0])) return INT_VAL((int64_t)floor(AS_FLOAT(args[0])));
    return NIL_VAL;
}

static Value native_ceil(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    if (IS_INT(args[0])) return args[0];
    if (IS_FLOAT(args[0])) return INT_VAL((int64_t)ceil(AS_FLOAT(args[0])));
    return NIL_VAL;
}

static Value native_round(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    if (IS_INT(args[0])) return args[0];
    if (IS_FLOAT(args[0])) return INT_VAL((int64_t)round(AS_FLOAT(args[0])));
    return NIL_VAL;
}

static Value native_sqrt(int arg_count, Value* args) {
    if (arg_count != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    return FLOAT_VAL(sqrt(AS_NUMBER(args[0])));
}

static Value native_pow(int arg_count, Value* args) {
    if (arg_count != 2) return NIL_VAL;
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) return NIL_VAL;
    return FLOAT_VAL(pow(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

static Value native_sin(int arg_count, Value* args) {
    if (arg_count != 1 || !IS_NUMBER(args[0])) return NIL_VAL;
    return FLOAT_VAL(sin(AS_NUMBER(args[0])));
}

static Value native_cos(int arg_count, Value* args) {
    if (arg_count != 1 || !IS_NUMBER(args[0])) return NIL_VAL;
    return FLOAT_VAL(cos(AS_NUMBER(args[0])));
}

static Value native_tan(int arg_count, Value* args) {
    if (arg_count != 1 || !IS_NUMBER(args[0])) return NIL_VAL;
    return FLOAT_VAL(tan(AS_NUMBER(args[0])));
}

/* ============ Table Functions ============ */

static Value native_keys(int arg_count, Value* args) {
    if (arg_count != 1 || !IS_TABLE(args[0])) return NIL_VAL;
    return OBJ_VAL(table_keys(AS_TABLE(args[0])));
}

static Value native_values(int arg_count, Value* args) {
    if (arg_count != 1 || !IS_TABLE(args[0])) return NIL_VAL;
    return OBJ_VAL(table_values(AS_TABLE(args[0])));
}

static Value native_has(int arg_count, Value* args) {
    if (arg_count != 2) return NIL_VAL;
    if (!IS_TABLE(args[0]) || !IS_STRING(args[1])) return NIL_VAL;
    return BOOL_VAL(table_has(AS_TABLE(args[0]), AS_STRING(args[1])));
}

/* ============ Utility Functions ============ */

static Value native_assert(int arg_count, Value* args) {
    if (arg_count < 1) return NIL_VAL;
    
    if (!value_is_truthy(args[0])) {
        if (arg_count >= 2 && IS_STRING(args[1])) {
            fprintf(stderr, "Assertion failed: %s\n", AS_CSTRING(args[1]));
        } else {
            fprintf(stderr, "Assertion failed\n");
        }
        exit(1);
    }
    return NIL_VAL;
}

static Value native_error(int arg_count, Value* args) {
    if (arg_count >= 1 && IS_STRING(args[0])) {
        fprintf(stderr, "Error: %s\n", AS_CSTRING(args[0]));
    } else {
        fprintf(stderr, "Error\n");
    }
    exit(1);
    return NIL_VAL;
}

static Value native_clock(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;
    return FLOAT_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value native_exit(int arg_count, Value* args) {
    int code = 0;
    if (arg_count >= 1 && IS_INT(args[0])) {
        code = (int)AS_INT(args[0]);
    }
    exit(code);
    return NIL_VAL;
}

/* Register all built-in functions */
void register_all_builtins(Environment* env) {
    /* I/O */
    register_native(env, "print", native_print, -1);
    register_native(env, "println", native_println, -1);
    register_native(env, "input", native_input, -1);
    
    /* Type conversion */
    register_native(env, "type", native_type, 1);
    register_native(env, "int", native_int, 1);
    register_native(env, "float", native_float, 1);
    register_native(env, "str", native_str, 1);
    register_native(env, "bool", native_bool, 1);
    
    /* Array */
    register_native(env, "len", native_len, 1);
    register_native(env, "push", native_push, 2);
    register_native(env, "pop", native_pop, 1);
    register_native(env, "first", native_first, 1);
    register_native(env, "last", native_last, 1);
    register_native(env, "insert", native_insert, 3);
    register_native(env, "remove", native_remove, 2);
    
    /* String */
    register_native(env, "substr", native_substr, -1);
    register_native(env, "find", native_find, 2);
    register_native(env, "replace", native_replace, 3);
    register_native(env, "split", native_split, 2);
    register_native(env, "join", native_join, 2);
    register_native(env, "upper", native_upper, 1);
    register_native(env, "lower", native_lower, 1);
    register_native(env, "trim", native_trim, 1);
    
    /* Math */
    register_native(env, "abs", native_abs, 1);
    register_native(env, "min", native_min, -1);
    register_native(env, "max", native_max, -1);
    register_native(env, "floor", native_floor, 1);
    register_native(env, "ceil", native_ceil, 1);
    register_native(env, "round", native_round, 1);
    register_native(env, "sqrt", native_sqrt, 1);
    register_native(env, "pow", native_pow, 2);
    register_native(env, "sin", native_sin, 1);
    register_native(env, "cos", native_cos, 1);
    register_native(env, "tan", native_tan, 1);
    
    /* Table */
    register_native(env, "keys", native_keys, 1);
    register_native(env, "values", native_values, 1);
    register_native(env, "has", native_has, 2);
    
    /* Utility */
    register_native(env, "assert", native_assert, -1);
    register_native(env, "error", native_error, -1);
    register_native(env, "clock", native_clock, 0);
    register_native(env, "exit", native_exit, -1);
}
