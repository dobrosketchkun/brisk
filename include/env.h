/*
 * Brisk Language - Environment (Scope)
 */

#ifndef BRISK_ENV_H
#define BRISK_ENV_H

#include "value.h"

/* Environment structure */
struct Environment {
    ObjTable* variables;
    Environment* enclosing;
    int ref_count;
};

/* Environment operations */
Environment* env_create(Environment* enclosing);
void env_destroy(Environment* env);
void env_incref(Environment* env);
void env_decref(Environment* env);

/* Variable operations */
bool env_define(Environment* env, const char* name, int length, Value value, bool is_const);
bool env_get(Environment* env, const char* name, int length, Value* value);
bool env_set(Environment* env, const char* name, int length, Value value);
bool env_is_const(Environment* env, const char* name, int length);

/* Get from specific scope (no parent lookup) */
bool env_get_local(Environment* env, const char* name, int length, Value* value);

#endif /* BRISK_ENV_H */
