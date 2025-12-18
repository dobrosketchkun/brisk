/*
 * Brisk Language - Tree-Walking Interpreter Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "interp.h"
#include "parser.h"
#include "memory.h"
#include "cffi.h"
#include "cheader.h"
#include "dynload.h"

/* Forward declarations */
static Value eval_binary(Interpreter* interp, AstNode* node);
static Value eval_unary(Interpreter* interp, AstNode* node);
static Value eval_call(Interpreter* interp, AstNode* node);
static void exec_block(Interpreter* interp, AstNode* node);
static void exec_if(Interpreter* interp, AstNode* node);
static void exec_while(Interpreter* interp, AstNode* node);
static void exec_for(Interpreter* interp, AstNode* node);
static void register_builtins(Interpreter* interp);

/* Runtime error */
void runtime_error(Interpreter* interp, int line, const char* format, ...) {
    interp->had_error = true;
    interp->error_line = line;
    
    va_list args;
    va_start(args, format);
    vsnprintf(interp->error_message, sizeof(interp->error_message), format, args);
    va_end(args);
    
    fprintf(stderr, "[line %d] Runtime Error: %s\n", line, interp->error_message);
}

/* Initialize interpreter */
void interp_init(Interpreter* interp) {
    interp->global = env_create(NULL);
    interp->current = interp->global;
    interp->return_value = NIL_VAL;
    interp->last_value = NIL_VAL;
    interp->returning = false;
    interp->breaking = false;
    interp->continuing = false;
    interp->had_error = false;
    interp->error_message[0] = '\0';
    interp->error_line = 0;
    interp->defer_stack = NULL;
    
    register_builtins(interp);
}

/* Destroy interpreter */
void interp_destroy(Interpreter* interp) {
    /* Execute any remaining defers */
    while (interp->defer_stack != NULL) {
        DeferEntry* entry = interp->defer_stack;
        interp->defer_stack = entry->next;
        exec(interp, entry->statement);
        mem_free(entry, sizeof(DeferEntry));
    }
    
    env_decref(interp->global);
}

/* Push defer */
static void push_defer(Interpreter* interp, AstNode* stmt) {
    DeferEntry* entry = mem_alloc(sizeof(DeferEntry));
    entry->statement = stmt;
    entry->next = interp->defer_stack;
    interp->defer_stack = entry;
}

/* Pop and execute defers until marker */
static void pop_defers(Interpreter* interp, DeferEntry* marker) {
    while (interp->defer_stack != marker) {
        DeferEntry* entry = interp->defer_stack;
        interp->defer_stack = entry->next;
        
        /* Temporarily clear control flow flags */
        bool was_returning = interp->returning;
        bool was_breaking = interp->breaking;
        bool was_continuing = interp->continuing;
        interp->returning = false;
        interp->breaking = false;
        interp->continuing = false;
        
        exec(interp, entry->statement);
        
        /* Restore flags */
        interp->returning = was_returning;
        interp->breaking = was_breaking;
        interp->continuing = was_continuing;
        
        mem_free(entry, sizeof(DeferEntry));
    }
}

/* Evaluate expression */
Value eval(Interpreter* interp, AstNode* node) {
    if (node == NULL || interp->had_error) {
        return NIL_VAL;
    }
    
    switch (node->type) {
        case NODE_LITERAL_INT:
            return INT_VAL(node->as.int_literal.value);
            
        case NODE_LITERAL_FLOAT:
            return FLOAT_VAL(node->as.float_literal.value);
            
        case NODE_LITERAL_STRING: {
            ObjString* str = string_create(node->as.string_literal.value,
                                           node->as.string_literal.length);
            return OBJ_VAL(str);
        }
        
        case NODE_LITERAL_BOOL:
            return BOOL_VAL(node->as.bool_literal.value);
            
        case NODE_LITERAL_NIL:
            return NIL_VAL;
            
        case NODE_IDENTIFIER: {
            Value value;
            if (!env_get(interp->current, 
                         node->as.identifier.name,
                         node->as.identifier.name_length,
                         &value)) {
                runtime_error(interp, node->line, "Undefined variable '%.*s'",
                             node->as.identifier.name_length,
                             node->as.identifier.name);
                return NIL_VAL;
            }
            return value;
        }
        
        case NODE_BINARY:
            return eval_binary(interp, node);
            
        case NODE_UNARY:
            return eval_unary(interp, node);
            
        case NODE_CALL:
            return eval_call(interp, node);
            
        case NODE_INDEX: {
            Value object = eval(interp, node->as.index.object);
            if (interp->had_error) return NIL_VAL;
            
            Value index = eval(interp, node->as.index.index);
            if (interp->had_error) return NIL_VAL;
            
            if (IS_ARRAY(object)) {
                if (!IS_INT(index)) {
                    runtime_error(interp, node->line, "Array index must be integer");
                    return NIL_VAL;
                }
                int idx = (int)AS_INT(index);
                ObjArray* arr = AS_ARRAY(object);
                if (idx < 0 || idx >= arr->count) {
                    runtime_error(interp, node->line, "Array index out of bounds");
                    return NIL_VAL;
                }
                return arr->elements[idx];
            }
            else if (IS_TABLE(object)) {
                if (!IS_STRING(index)) {
                    runtime_error(interp, node->line, "Table key must be string");
                    return NIL_VAL;
                }
                Value value;
                if (!table_get(AS_TABLE(object), AS_STRING(index), &value)) {
                    return NIL_VAL;
                }
                return value;
            }
            else if (IS_STRING(object)) {
                if (!IS_INT(index)) {
                    runtime_error(interp, node->line, "String index must be integer");
                    return NIL_VAL;
                }
                int idx = (int)AS_INT(index);
                ObjString* str = AS_STRING(object);
                if (idx < 0 || idx >= str->length) {
                    runtime_error(interp, node->line, "String index out of bounds");
                    return NIL_VAL;
                }
                return OBJ_VAL(string_create(&str->chars[idx], 1));
            }
            else {
                runtime_error(interp, node->line, "Cannot index type %s",
                             value_type_name(object));
                return NIL_VAL;
            }
        }
        
        case NODE_FIELD: {
            Value object = eval(interp, node->as.field.object);
            if (interp->had_error) return NIL_VAL;
            
            if (IS_TABLE(object)) {
                ObjString* key = string_create(node->as.field.field_name,
                                               node->as.field.field_name_length);
                Value value;
                if (!table_get(AS_TABLE(object), key, &value)) {
                    obj_decref((Object*)key);
                    return NIL_VAL;
                }
                obj_decref((Object*)key);
                return value;
            }
            else {
                runtime_error(interp, node->line, "Cannot access field on type %s",
                             value_type_name(object));
                return NIL_VAL;
            }
        }
        
        case NODE_ARRAY: {
            ObjArray* arr = array_create();
            for (int i = 0; i < node->as.array.element_count; i++) {
                Value elem = eval(interp, node->as.array.elements[i]);
                if (interp->had_error) {
                    obj_decref((Object*)arr);
                    return NIL_VAL;
                }
                array_push(arr, elem);
            }
            return OBJ_VAL(arr);
        }
        
        case NODE_TABLE: {
            ObjTable* table = table_create();
            for (int i = 0; i < node->as.table.count; i++) {
                ObjString* key = string_create(node->as.table.keys[i],
                                               node->as.table.key_lengths[i]);
                Value value = eval(interp, node->as.table.values[i]);
                if (interp->had_error) {
                    obj_decref((Object*)key);
                    obj_decref((Object*)table);
                    return NIL_VAL;
                }
                table_set(table, key, value, false);
                obj_decref((Object*)key);
            }
            return OBJ_VAL(table);
        }
        
        case NODE_RANGE: {
            Value start = eval(interp, node->as.range.start);
            if (interp->had_error) return NIL_VAL;
            
            Value end = eval(interp, node->as.range.end);
            if (interp->had_error) return NIL_VAL;
            
            if (!IS_INT(start) || !IS_INT(end)) {
                runtime_error(interp, node->line, "Range bounds must be integers");
                return NIL_VAL;
            }
            
            /* Create array from range */
            int64_t s = AS_INT(start);
            int64_t e = AS_INT(end);
            ObjArray* arr = array_create();
            
            if (s <= e) {
                for (int64_t i = s; i < e; i++) {
                    array_push(arr, INT_VAL(i));
                }
            } else {
                for (int64_t i = s; i > e; i--) {
                    array_push(arr, INT_VAL(i));
                }
            }
            
            return OBJ_VAL(arr);
        }
        
        case NODE_LAMBDA: {
            ObjFunction* fn = function_create(
                NULL, 0,
                node->as.lambda.parameters,
                node->as.lambda.param_lengths,
                node->as.lambda.param_count,
                node->as.lambda.body,
                interp->current
            );
            env_incref(interp->current);
            return OBJ_VAL(fn);
        }
        
        case NODE_ADDRESS_OF: {
            /* For C interop - get address of a value */
            Value operand = eval(interp, node->as.address_of.operand);
            if (interp->had_error) return NIL_VAL;
            
            if (IS_CSTRUCT(operand)) {
                /* Return pointer to struct data */
                ObjCStruct* cs = AS_CSTRUCT(operand);
                return OBJ_VAL(pointer_create(cs->data, "void*"));
            }
            
            runtime_error(interp, node->line, "Cannot take address of this value");
            return NIL_VAL;
        }
        
        default:
            runtime_error(interp, node->line, "Unknown expression type");
            return NIL_VAL;
    }
}

/* Evaluate binary expression */
static Value eval_binary(Interpreter* interp, AstNode* node) {
    Value left = eval(interp, node->as.binary.left);
    if (interp->had_error) return NIL_VAL;
    
    /* Short-circuit for and/or */
    if (node->as.binary.operator == TOKEN_AND) {
        if (!value_is_truthy(left)) return left;
        return eval(interp, node->as.binary.right);
    }
    if (node->as.binary.operator == TOKEN_OR) {
        if (value_is_truthy(left)) return left;
        return eval(interp, node->as.binary.right);
    }
    
    Value right = eval(interp, node->as.binary.right);
    if (interp->had_error) return NIL_VAL;
    
    TokenType op = node->as.binary.operator;
    
    /* Equality - works for all types */
    if (op == TOKEN_EQEQ) {
        return BOOL_VAL(value_equals(left, right));
    }
    if (op == TOKEN_NEQ) {
        return BOOL_VAL(!value_equals(left, right));
    }
    
    /* String concatenation */
    if (op == TOKEN_PLUS && IS_STRING(left) && IS_STRING(right)) {
        ObjString* result = string_concat(AS_STRING(left), AS_STRING(right));
        return OBJ_VAL(result);
    }
    
    /* String + anything = string concatenation */
    if (op == TOKEN_PLUS && IS_STRING(left)) {
        char* right_str = value_to_string(right);
        ObjString* right_obj = string_create(right_str, strlen(right_str));
        mem_free(right_str, 256);
        ObjString* result = string_concat(AS_STRING(left), right_obj);
        obj_decref((Object*)right_obj);
        return OBJ_VAL(result);
    }
    
    /* Numeric operations */
    if (!IS_NUMBER(left) || !IS_NUMBER(right)) {
        runtime_error(interp, node->line, "Operands must be numbers");
        return NIL_VAL;
    }
    
    /* If either is float, result is float */
    bool use_float = IS_FLOAT(left) || IS_FLOAT(right);
    
    if (use_float) {
        double l = AS_NUMBER(left);
        double r = AS_NUMBER(right);
        
        switch (op) {
            case TOKEN_PLUS: return FLOAT_VAL(l + r);
            case TOKEN_MINUS: return FLOAT_VAL(l - r);
            case TOKEN_STAR: return FLOAT_VAL(l * r);
            case TOKEN_SLASH:
                if (r == 0) {
                    runtime_error(interp, node->line, "Division by zero");
                    return NIL_VAL;
                }
                return FLOAT_VAL(l / r);
            case TOKEN_PERCENT:
                if (r == 0) {
                    runtime_error(interp, node->line, "Modulo by zero");
                    return NIL_VAL;
                }
                return FLOAT_VAL(fmod(l, r));
            case TOKEN_LT: return BOOL_VAL(l < r);
            case TOKEN_GT: return BOOL_VAL(l > r);
            case TOKEN_LTE: return BOOL_VAL(l <= r);
            case TOKEN_GTE: return BOOL_VAL(l >= r);
            default:
                runtime_error(interp, node->line, "Unknown operator");
                return NIL_VAL;
        }
    } else {
        int64_t l = AS_INT(left);
        int64_t r = AS_INT(right);
        
        switch (op) {
            case TOKEN_PLUS: return INT_VAL(l + r);
            case TOKEN_MINUS: return INT_VAL(l - r);
            case TOKEN_STAR: return INT_VAL(l * r);
            case TOKEN_SLASH:
                if (r == 0) {
                    runtime_error(interp, node->line, "Division by zero");
                    return NIL_VAL;
                }
                return INT_VAL(l / r);
            case TOKEN_PERCENT:
                if (r == 0) {
                    runtime_error(interp, node->line, "Modulo by zero");
                    return NIL_VAL;
                }
                return INT_VAL(l % r);
            case TOKEN_LT: return BOOL_VAL(l < r);
            case TOKEN_GT: return BOOL_VAL(l > r);
            case TOKEN_LTE: return BOOL_VAL(l <= r);
            case TOKEN_GTE: return BOOL_VAL(l >= r);
            default:
                runtime_error(interp, node->line, "Unknown operator");
                return NIL_VAL;
        }
    }
}

/* Evaluate unary expression */
static Value eval_unary(Interpreter* interp, AstNode* node) {
    Value operand = eval(interp, node->as.unary.operand);
    if (interp->had_error) return NIL_VAL;
    
    switch (node->as.unary.operator) {
        case TOKEN_MINUS:
            if (IS_INT(operand)) {
                return INT_VAL(-AS_INT(operand));
            }
            if (IS_FLOAT(operand)) {
                return FLOAT_VAL(-AS_FLOAT(operand));
            }
            runtime_error(interp, node->line, "Operand must be a number");
            return NIL_VAL;
            
        case TOKEN_NOT:
        case TOKEN_BANG:
            return BOOL_VAL(!value_is_truthy(operand));
            
        default:
            runtime_error(interp, node->line, "Unknown unary operator");
            return NIL_VAL;
    }
}

/* Evaluate function call */
static Value eval_call(Interpreter* interp, AstNode* node) {
    Value callee = eval(interp, node->as.call.callee);
    if (interp->had_error) return NIL_VAL;
    
    /* Evaluate arguments */
    int arg_count = node->as.call.arg_count;
    Value* args = NULL;
    if (arg_count > 0) {
        args = mem_alloc(sizeof(Value) * arg_count);
        for (int i = 0; i < arg_count; i++) {
            args[i] = eval(interp, node->as.call.arguments[i]);
            if (interp->had_error) {
                mem_free(args, sizeof(Value) * arg_count);
                return NIL_VAL;
            }
        }
    }
    
    Value result = NIL_VAL;
    
    if (IS_NATIVE(callee)) {
        ObjNative* native = AS_NATIVE(callee);
        
        /* Check arity (-1 means variadic) */
        if (native->arity >= 0 && arg_count != native->arity) {
            runtime_error(interp, node->line, "Expected %d arguments but got %d",
                         native->arity, arg_count);
            if (args) mem_free(args, sizeof(Value) * arg_count);
            return NIL_VAL;
        }
        
        result = native->function(arg_count, args);
    }
    else if (IS_CFUNCTION(callee)) {
        ObjCFunction* cfn = AS_CFUNCTION(callee);
        result = cffi_call(cfn->desc, arg_count, args);
    }
    else if (IS_FUNCTION(callee)) {
        ObjFunction* fn = AS_FUNCTION(callee);
        
        /* Check arity */
        if (arg_count != fn->arity) {
            runtime_error(interp, node->line, "Expected %d arguments but got %d",
                         fn->arity, arg_count);
            if (args) mem_free(args, sizeof(Value) * arg_count);
            return NIL_VAL;
        }
        
        /* Create new environment for function */
        Environment* fn_env = env_create(fn->closure);
        
        /* Bind parameters to arguments */
        for (int i = 0; i < fn->arity; i++) {
            env_define(fn_env, fn->params[i], fn->param_lengths[i], args[i], false);
        }
        
        /* Save current environment */
        Environment* previous = interp->current;
        interp->current = fn_env;
        
        /* Remember defer stack position */
        DeferEntry* defer_marker = interp->defer_stack;
        
        /* Reset last_value for implicit return tracking */
        interp->last_value = NIL_VAL;
        
        /* Execute function body */
        exec(interp, fn->body);
        
        /* Pop defers */
        pop_defers(interp, defer_marker);
        
        /* Restore environment */
        interp->current = previous;
        env_decref(fn_env);
        
        /* Get return value (explicit or implicit) */
        if (interp->returning) {
            result = interp->return_value;
            interp->returning = false;
        } else {
            /* Use last expression value as implicit return */
            result = interp->last_value;
        }
    }
    else {
        runtime_error(interp, node->line, "Can only call functions");
    }
    
    if (args) mem_free(args, sizeof(Value) * arg_count);
    return result;
}

/* Execute statement */
void exec(Interpreter* interp, AstNode* node) {
    if (node == NULL || interp->had_error) return;
    if (interp->returning || interp->breaking || interp->continuing) return;
    
    switch (node->type) {
        case NODE_VAR_DECL:
        case NODE_CONST_DECL: {
            Value value = eval(interp, node->as.var_decl.initializer);
            if (interp->had_error) return;
            
            bool is_const = node->as.var_decl.is_const;
            if (!env_define(interp->current, 
                           node->as.var_decl.name,
                           node->as.var_decl.name_length,
                           value, is_const)) {
                runtime_error(interp, node->line, "Variable '%.*s' already defined",
                             node->as.var_decl.name_length,
                             node->as.var_decl.name);
            }
            break;
        }
        
        case NODE_ASSIGNMENT: {
            Value value = eval(interp, node->as.assignment.value);
            if (interp->had_error) return;
            
            AstNode* target = node->as.assignment.target;
            
            if (target->type == NODE_IDENTIFIER) {
                /* Check if const */
                if (env_is_const(interp->current,
                                target->as.identifier.name,
                                target->as.identifier.name_length)) {
                    runtime_error(interp, node->line, "Cannot assign to constant '%.*s'",
                                 target->as.identifier.name_length,
                                 target->as.identifier.name);
                    return;
                }
                
                if (!env_set(interp->current,
                            target->as.identifier.name,
                            target->as.identifier.name_length,
                            value)) {
                    runtime_error(interp, node->line, "Undefined variable '%.*s'",
                                 target->as.identifier.name_length,
                                 target->as.identifier.name);
                }
            }
            else if (target->type == NODE_INDEX) {
                Value object = eval(interp, target->as.index.object);
                if (interp->had_error) return;
                
                Value index = eval(interp, target->as.index.index);
                if (interp->had_error) return;
                
                if (IS_ARRAY(object)) {
                    if (!IS_INT(index)) {
                        runtime_error(interp, node->line, "Array index must be integer");
                        return;
                    }
                    array_set(AS_ARRAY(object), (int)AS_INT(index), value);
                }
                else if (IS_TABLE(object)) {
                    if (!IS_STRING(index)) {
                        runtime_error(interp, node->line, "Table key must be string");
                        return;
                    }
                    table_set(AS_TABLE(object), AS_STRING(index), value, false);
                }
                else {
                    runtime_error(interp, node->line, "Cannot index type %s",
                                 value_type_name(object));
                }
            }
            else if (target->type == NODE_FIELD) {
                Value object = eval(interp, target->as.field.object);
                if (interp->had_error) return;
                
                if (IS_TABLE(object)) {
                    ObjString* key = string_create(target->as.field.field_name,
                                                   target->as.field.field_name_length);
                    table_set(AS_TABLE(object), key, value, false);
                    obj_decref((Object*)key);
                }
                else {
                    runtime_error(interp, node->line, "Cannot set field on type %s",
                                 value_type_name(object));
                }
            }
            else {
                runtime_error(interp, node->line, "Invalid assignment target");
            }
            break;
        }
        
        case NODE_EXPR_STMT: {
            /* Track last expression value for implicit return */
            interp->last_value = eval(interp, node->as.unary.operand);
            break;
        }
        
        case NODE_BLOCK:
            exec_block(interp, node);
            break;
        
        case NODE_PROGRAM:
            /* Execute all statements in the program */
            for (int i = 0; i < node->as.program.statement_count; i++) {
                exec(interp, node->as.program.statements[i]);
                if (interp->had_error) break;
            }
            break;
            
        case NODE_IF:
            exec_if(interp, node);
            break;
            
        case NODE_WHILE:
            exec_while(interp, node);
            break;
            
        case NODE_FOR:
            exec_for(interp, node);
            break;
            
        case NODE_RETURN: {
            if (node->as.return_stmt.value != NULL) {
                interp->return_value = eval(interp, node->as.return_stmt.value);
            } else {
                interp->return_value = NIL_VAL;
            }
            interp->returning = true;
            break;
        }
        
        case NODE_BREAK:
            interp->breaking = true;
            break;
            
        case NODE_CONTINUE:
            interp->continuing = true;
            break;
            
        case NODE_FN_DECL: {
            ObjFunction* fn = function_create(
                node->as.fn_decl.name,
                node->as.fn_decl.name_length,
                node->as.fn_decl.parameters,
                node->as.fn_decl.param_lengths,
                node->as.fn_decl.param_count,
                node->as.fn_decl.body,
                interp->current
            );
            env_incref(interp->current);
            
            env_define(interp->current,
                      node->as.fn_decl.name,
                      node->as.fn_decl.name_length,
                      OBJ_VAL(fn), false);
            break;
        }
        
        case NODE_MATCH: {
            Value value = eval(interp, node->as.match_stmt.value);
            if (interp->had_error) return;
            
            for (int i = 0; i < node->as.match_stmt.arm_count; i++) {
                AstNode* pattern = node->as.match_stmt.patterns[i];
                AstNode* body = node->as.match_stmt.bodies[i];
                bool matched = false;
                
                /* Wildcard matches everything */
                if (pattern->type == NODE_IDENTIFIER &&
                    pattern->as.identifier.name_length == 1 &&
                    pattern->as.identifier.name[0] == '_') {
                    matched = true;
                }
                /* Range pattern */
                else if (pattern->type == NODE_RANGE) {
                    if (IS_INT(value)) {
                        Value start = eval(interp, pattern->as.range.start);
                        Value end = eval(interp, pattern->as.range.end);
                        
                        if (IS_INT(start) && IS_INT(end)) {
                            int64_t v = AS_INT(value);
                            int64_t s = AS_INT(start);
                            int64_t e = AS_INT(end);
                            if (v >= s && v < e) {
                                matched = true;
                            }
                        }
                    }
                }
                /* Literal pattern */
                else {
                    Value pattern_val = eval(interp, pattern);
                    if (interp->had_error) return;
                    
                    if (value_equals(value, pattern_val)) {
                        matched = true;
                    }
                }
                
                if (matched) {
                    /* Execute body - check if it's a statement or expression */
                    if (body->type == NODE_BLOCK) {
                        exec(interp, body);
                    } else {
                        /* It's an expression - evaluate and store as last_value */
                        interp->last_value = eval(interp, body);
                    }
                    return;
                }
            }
            break;
        }
        
        case NODE_DEFER:
            push_defer(interp, node->as.defer_stmt.statement);
            break;
            
        case NODE_IMPORT: {
            /* Execute @import */
            const char* import_path = node->as.import.path;
            size_t path_len = strlen(import_path);
            
            /* Check if it's a Brisk module (.brisk file) */
            if (path_len > 6 && strcmp(import_path + path_len - 6, ".brisk") == 0) {
                /* Import a Brisk module */
                char resolved_path[512];
                
                /* Try relative to current file first, then current directory */
                if (import_path[0] == '/' || import_path[0] == '.') {
                    snprintf(resolved_path, sizeof(resolved_path), "%s", import_path);
                } else {
                    snprintf(resolved_path, sizeof(resolved_path), "./%s", import_path);
                }
                
                /* Read the module file */
                FILE* file = fopen(resolved_path, "rb");
                if (!file) {
                    /* Try lib/ directory */
                    snprintf(resolved_path, sizeof(resolved_path), "lib/%s", import_path);
                    file = fopen(resolved_path, "rb");
                }
                
                if (!file) {
                    runtime_error(interp, node->line, "Cannot find module '%s'", import_path);
                    break;
                }
                
                fseek(file, 0, SEEK_END);
                long size = ftell(file);
                rewind(file);
                
                char* source = mem_alloc(size + 1);
                size_t read_size = fread(source, 1, size, file);
                source[read_size] = '\0';
                fclose(file);
                
                /* Parse the module */
                Lexer lexer;
                lexer_init(&lexer, source);
                
                Parser parser;
                parser_init(&parser, &lexer);
                
                AstNode* module_ast = parse_program(&parser);
                
                if (parser.had_error || !module_ast) {
                    runtime_error(interp, node->line, "Failed to parse module '%s'", import_path);
                    mem_free(source, size + 1);
                    break;
                }
                
                /* Execute the module in the current global environment */
                /* This makes all top-level definitions available */
                exec(interp, module_ast);
                
                /* Note: We do NOT free module_ast or source here because
                   ObjFunction stores pointers to the AST nodes.
                   TODO: Implement proper module caching to manage this memory */
                break;
            }
            
            /* Otherwise, it's a C header */
            const char* header_path = import_path;
            
            /* Find the header file */
            char* full_path = cheader_find_include(header_path, true);
            if (!full_path) {
                runtime_error(interp, node->line, "Cannot find header '%s'", header_path);
                break;
            }
            
            /* Parse the header */
            CHeaderParser hparser;
            cheader_init(&hparser);
            
            if (!cheader_load(&hparser, full_path)) {
                runtime_error(interp, node->line, "Failed to parse header '%s'", header_path);
                mem_free(full_path, strlen(full_path) + 1);
                cheader_free(&hparser);
                break;
            }
            
            /* Get library handle */
            LibHandle lib = lib_open(NULL);
            
            /* Try to find associated library */
            /* math.h -> libm */
            if (strstr(header_path, "math.h")) {
                LibHandle libm = lib_open("m");
                if (libm) lib = libm;
            }
            /* raylib -> libraylib.so */
            if (strstr(header_path, "raylib")) {
                /* Try common locations for raylib */
                LibHandle raylib = lib_open("raylib");
                if (!raylib) raylib = lib_open("./experiments/raylib_lib/libraylib.so");
                if (!raylib) raylib = lib_open("experiments/raylib_lib/libraylib.so");
                if (!raylib) raylib = lib_open("./experiments/raylib/src/libraylib.so");
                if (!raylib) raylib = lib_open("./libraylib.so");
                if (raylib) lib = raylib;
            }
            
            /* Register declarations */
            cheader_register(&hparser, interp->global, lib);
            
            /* For math.h, many functions are defined via macros (__MATHCALL).
               Register common math functions directly if not already present */
            if (strstr(header_path, "math.h")) {
                static const char* math_funcs_1[] = {
                    "sin", "cos", "tan", "asin", "acos", "atan",
                    "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
                    "exp", "exp2", "log", "log10", "log2",
                    "sqrt", "cbrt", "fabs", "floor", "ceil", "round", "trunc",
                    NULL
                };
                static const char* math_funcs_2[] = {
                    "atan2", "pow", "fmod", "hypot", "remainder", "copysign",
                    "fmin", "fmax", "fdim",
                    NULL
                };
                
                CType param1[1] = {CTYPE_DOUBLE};
                CType param2[2] = {CTYPE_DOUBLE, CTYPE_DOUBLE};
                
                for (int i = 0; math_funcs_1[i]; i++) {
                    /* Skip if already defined */
                    Value existing;
                    if (env_get(interp->global, math_funcs_1[i], strlen(math_funcs_1[i]), &existing))
                        continue;
                        
                    void* fn_ptr = lib_symbol(lib, math_funcs_1[i]);
                    if (fn_ptr) {
                        CFunctionDesc* desc = cfunc_create(math_funcs_1[i], CTYPE_DOUBLE, param1, 1, false, fn_ptr);
                        if (desc && cfunc_prepare(desc)) {
                            ObjCFunction* cfn = cfunction_create(desc);
                            env_define(interp->global, math_funcs_1[i], strlen(math_funcs_1[i]),
                                       OBJ_VAL((Object*)cfn), false);
                        }
                    }
                }
                
                for (int i = 0; math_funcs_2[i]; i++) {
                    Value existing;
                    if (env_get(interp->global, math_funcs_2[i], strlen(math_funcs_2[i]), &existing))
                        continue;
                        
                    void* fn_ptr = lib_symbol(lib, math_funcs_2[i]);
                    if (fn_ptr) {
                        CFunctionDesc* desc = cfunc_create(math_funcs_2[i], CTYPE_DOUBLE, param2, 2, false, fn_ptr);
                        if (desc && cfunc_prepare(desc)) {
                            ObjCFunction* cfn = cfunction_create(desc);
                            env_define(interp->global, math_funcs_2[i], strlen(math_funcs_2[i]),
                                       OBJ_VAL((Object*)cfn), false);
                        }
                    }
                }
            }
            
            cheader_free(&hparser);
            mem_free(full_path, strlen(full_path) + 1);
            break;
        }
            
        case NODE_C_BLOCK:
            /* TODO: Implement @c blocks */
            runtime_error(interp, node->line, "@c blocks not yet implemented");
            break;
            
        default:
            runtime_error(interp, node->line, "Unknown statement type");
            break;
    }
}

/* Execute block */
static void exec_block(Interpreter* interp, AstNode* node) {
    Environment* previous = interp->current;
    interp->current = env_create(previous);
    
    DeferEntry* defer_marker = interp->defer_stack;
    
    for (int i = 0; i < node->as.block.statement_count; i++) {
        exec(interp, node->as.block.statements[i]);
        if (interp->returning || interp->breaking || 
            interp->continuing || interp->had_error) {
            break;
        }
    }
    
    pop_defers(interp, defer_marker);
    
    env_decref(interp->current);
    interp->current = previous;
}

/* Execute if statement */
static void exec_if(Interpreter* interp, AstNode* node) {
    Value condition = eval(interp, node->as.if_stmt.condition);
    if (interp->had_error) return;
    
    if (value_is_truthy(condition)) {
        exec(interp, node->as.if_stmt.then_branch);
    } else if (node->as.if_stmt.else_branch != NULL) {
        exec(interp, node->as.if_stmt.else_branch);
    }
}

/* Execute while loop */
static void exec_while(Interpreter* interp, AstNode* node) {
    while (!interp->had_error) {
        Value condition = eval(interp, node->as.while_stmt.condition);
        if (interp->had_error) return;
        
        if (!value_is_truthy(condition)) break;
        
        exec(interp, node->as.while_stmt.body);
        
        if (interp->returning) return;
        if (interp->breaking) {
            interp->breaking = false;
            break;
        }
        if (interp->continuing) {
            interp->continuing = false;
        }
    }
}

/* Execute for loop */
static void exec_for(Interpreter* interp, AstNode* node) {
    Value iterable = eval(interp, node->as.for_stmt.iterable);
    if (interp->had_error) return;
    
    if (!IS_ARRAY(iterable)) {
        runtime_error(interp, node->line, "Can only iterate over arrays");
        return;
    }
    
    ObjArray* arr = AS_ARRAY(iterable);
    
    Environment* previous = interp->current;
    interp->current = env_create(previous);
    
    /* Define iterator variable */
    env_define(interp->current,
               node->as.for_stmt.iterator_name,
               node->as.for_stmt.iterator_name_length,
               NIL_VAL, false);
    
    for (int i = 0; i < arr->count && !interp->had_error; i++) {
        /* Update iterator */
        env_set(interp->current,
                node->as.for_stmt.iterator_name,
                node->as.for_stmt.iterator_name_length,
                arr->elements[i]);
        
        exec(interp, node->as.for_stmt.body);
        
        if (interp->returning) break;
        if (interp->breaking) {
            interp->breaking = false;
            break;
        }
        if (interp->continuing) {
            interp->continuing = false;
        }
    }
    
    env_decref(interp->current);
    interp->current = previous;
}

/* Execute program */
void exec_program(Interpreter* interp, AstNode* program) {
    if (program == NULL || program->type != NODE_PROGRAM) return;
    
    for (int i = 0; i < program->as.program.statement_count; i++) {
        exec(interp, program->as.program.statements[i]);
        if (interp->had_error) break;
    }
}

/* Built-in functions - implemented separately */
#include "builtins.h"

static void register_builtins(Interpreter* interp) {
    register_all_builtins(interp->global);
}

/* Main entry point */
int interpret(const char* source) {
    AstNode* ast = parse(source);
    if (ast == NULL) {
        return 1;  /* Parse error */
    }
    
    Interpreter interp;
    interp_init(&interp);
    
    exec_program(&interp, ast);
    
    int result = interp.had_error ? 1 : 0;
    
    interp_destroy(&interp);
    ast_free_tree(ast);
    
    return result;
}

/* Run from file */
int interpret_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'\n", path);
        return 1;
    }
    
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);
    
    char* source = malloc(file_size + 1);
    if (source == NULL) {
        fprintf(stderr, "Error: Not enough memory to read '%s'\n", path);
        fclose(file);
        return 1;
    }
    
    size_t bytes_read = fread(source, 1, file_size, file);
    source[bytes_read] = '\0';
    fclose(file);
    
    int result = interpret(source);
    free(source);
    return result;
}
