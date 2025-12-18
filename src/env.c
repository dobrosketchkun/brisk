/*
 * Brisk Language - Environment Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "memory.h"

Environment* env_create(Environment* enclosing) {
    Environment* env = mem_alloc(sizeof(Environment));
    env->variables = table_create();
    env->enclosing = enclosing;
    env->ref_count = 1;
    
    if (enclosing != NULL) {
        env_incref(enclosing);
    }
    
    return env;
}

void env_incref(Environment* env) {
    if (env == NULL) return;
    env->ref_count++;
}

void env_decref(Environment* env) {
    if (env == NULL) return;
    env->ref_count--;
    if (env->ref_count <= 0) {
        env_destroy(env);
    }
}

void env_destroy(Environment* env) {
    if (env == NULL) return;
    
    if (env->variables != NULL) {
        obj_decref((Object*)env->variables);
    }
    
    if (env->enclosing != NULL) {
        env_decref(env->enclosing);
    }
    
    mem_free(env, sizeof(Environment));
}

bool env_define(Environment* env, const char* name, int length, Value value, bool is_const) {
    ObjString* key = string_create(name, length);
    
    /* Check if already defined in this scope */
    Value existing;
    if (table_get(env->variables, key, &existing)) {
        obj_decref((Object*)key);
        return false;  /* Already defined */
    }
    
    table_set(env->variables, key, value, is_const);
    obj_decref((Object*)key);  /* table_set increfs */
    return true;
}

bool env_get(Environment* env, const char* name, int length, Value* value) {
    ObjString* key = string_create(name, length);
    
    Environment* current = env;
    while (current != NULL) {
        if (table_get(current->variables, key, value)) {
            obj_decref((Object*)key);
            return true;
        }
        current = current->enclosing;
    }
    
    obj_decref((Object*)key);
    return false;
}

bool env_get_local(Environment* env, const char* name, int length, Value* value) {
    ObjString* key = string_create(name, length);
    bool found = table_get(env->variables, key, value);
    obj_decref((Object*)key);
    return found;
}

/* Check if variable is const - helper */
static bool find_entry_const(Environment* env, ObjString* key, bool* is_const) {
    if (env->variables->count == 0) return false;
    
    uint32_t index = key->hash % env->variables->capacity;
    
    for (;;) {
        TableEntry* entry = &env->variables->entries[index];
        
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                return false;
            }
        } else if (entry->key->length == key->length &&
                   entry->key->hash == key->hash &&
                   memcmp(entry->key->chars, key->chars, key->length) == 0) {
            *is_const = entry->is_const;
            return true;
        }
        
        index = (index + 1) % env->variables->capacity;
    }
}

bool env_is_const(Environment* env, const char* name, int length) {
    ObjString* key = string_create(name, length);
    bool is_const = false;
    
    Environment* current = env;
    while (current != NULL) {
        if (find_entry_const(current, key, &is_const)) {
            obj_decref((Object*)key);
            return is_const;
        }
        current = current->enclosing;
    }
    
    obj_decref((Object*)key);
    return false;
}

bool env_set(Environment* env, const char* name, int length, Value value) {
    ObjString* key = string_create(name, length);
    
    Environment* current = env;
    while (current != NULL) {
        Value existing;
        if (table_get(current->variables, key, &existing)) {
            /* Check if const */
            bool is_const = false;
            if (find_entry_const(current, key, &is_const) && is_const) {
                obj_decref((Object*)key);
                return false;  /* Cannot assign to const */
            }
            
            table_set(current->variables, key, value, false);
            obj_decref((Object*)key);
            return true;
        }
        current = current->enclosing;
    }
    
    obj_decref((Object*)key);
    return false;  /* Variable not found */
}
