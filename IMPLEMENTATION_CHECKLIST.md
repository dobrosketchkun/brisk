# Brisk Implementation Checklist (Linux/WSL)

A granular, step-by-step implementation guide. Each checkbox is one atomic task.

**Target Platform:** Linux (WSL) only. Cross-platform support deferred.

---

## Phase 0: Project Setup

### 0.1 Directory Structure
- [ ] Create `src/` directory
- [ ] Create `include/` directory for headers
- [ ] Create `examples/` directory
- [ ] Create `tests/` directory
- [ ] Create `lib/` directory for standard library

### 0.2 Build System
- [ ] Create `Makefile` with basic structure
- [ ] Add `CC = gcc` variable
- [ ] Add `CFLAGS = -Wall -Wextra -std=c99` variable
- [ ] Add `LDFLAGS = -lffi -ldl -lm` variable
- [ ] Add `all` target
- [ ] Add `clean` target
- [ ] Add `debug` target with `-g -O0 -DDEBUG`
- [ ] Add `release` target with `-O2`
- [ ] Test that empty project compiles

### 0.3 Install Dependencies
- [ ] Install libffi-dev: `sudo apt install libffi-dev`
- [ ] Verify ffi.h is available

### 0.4 Entry Point
- [ ] Create `src/main.c` with `main()` function
- [ ] Add command-line argument parsing skeleton
- [ ] Add `--help` flag handling
- [ ] Add `--version` flag handling
- [ ] Add file path argument handling
- [ ] Add REPL mode when no file given
- [ ] Verify compilation and execution

---

## Phase 1: Lexer (Tokenizer)

### 1.1 Token Type Definitions
- [ ] Create `include/token.h`
- [ ] Define `TokenType` enum
- [ ] Add `TOKEN_EOF` for end of file
- [ ] Add `TOKEN_ERROR` for lexer errors
- [ ] Add `TOKEN_NEWLINE` for line tracking

### 1.2 Literal Tokens
- [ ] Add `TOKEN_INT` for integer literals
- [ ] Add `TOKEN_FLOAT` for float literals
- [ ] Add `TOKEN_STRING` for string literals
- [ ] Add `TOKEN_TRUE` for `true`
- [ ] Add `TOKEN_FALSE` for `false`
- [ ] Add `TOKEN_NIL` for `nil`

### 1.3 Identifier and Keyword Tokens
- [ ] Add `TOKEN_IDENTIFIER` for names
- [ ] Add `TOKEN_FN` for `fn`
- [ ] Add `TOKEN_IF` for `if`
- [ ] Add `TOKEN_ELIF` for `elif`
- [ ] Add `TOKEN_ELSE` for `else`
- [ ] Add `TOKEN_WHILE` for `while`
- [ ] Add `TOKEN_FOR` for `for`
- [ ] Add `TOKEN_IN` for `in`
- [ ] Add `TOKEN_RETURN` for `return`
- [ ] Add `TOKEN_BREAK` for `break`
- [ ] Add `TOKEN_CONTINUE` for `continue`
- [ ] Add `TOKEN_MATCH` for `match`
- [ ] Add `TOKEN_DEFER` for `defer`
- [ ] Add `TOKEN_AND` for `and`
- [ ] Add `TOKEN_OR` for `or`
- [ ] Add `TOKEN_NOT` for `not`

### 1.4 Operator Tokens
- [ ] Add `TOKEN_PLUS` for `+`
- [ ] Add `TOKEN_MINUS` for `-`
- [ ] Add `TOKEN_STAR` for `*`
- [ ] Add `TOKEN_SLASH` for `/`
- [ ] Add `TOKEN_PERCENT` for `%`
- [ ] Add `TOKEN_EQ` for `=`
- [ ] Add `TOKEN_EQEQ` for `==`
- [ ] Add `TOKEN_NEQ` for `!=`
- [ ] Add `TOKEN_LT` for `<`
- [ ] Add `TOKEN_GT` for `>`
- [ ] Add `TOKEN_LTE` for `<=`
- [ ] Add `TOKEN_GTE` for `>=`
- [ ] Add `TOKEN_AMPERSAND` for `&` (address-of)
- [ ] Add `TOKEN_DOTDOT` for `..` (range)
- [ ] Add `TOKEN_ARROW` for `=>` (match arm)
- [ ] Add `TOKEN_RARROW` for `->` (return type)

### 1.5 Delimiter Tokens
- [ ] Add `TOKEN_LPAREN` for `(`
- [ ] Add `TOKEN_RPAREN` for `)`
- [ ] Add `TOKEN_LBRACE` for `{`
- [ ] Add `TOKEN_RBRACE` for `}`
- [ ] Add `TOKEN_LBRACKET` for `[`
- [ ] Add `TOKEN_RBRACKET` for `]`
- [ ] Add `TOKEN_COMMA` for `,`
- [ ] Add `TOKEN_DOT` for `.`
- [ ] Add `TOKEN_COLON` for `:`
- [ ] Add `TOKEN_COLONCOLON` for `::` (const)
- [ ] Add `TOKEN_COLONEQ` for `:=` (var decl)
- [ ] Add `TOKEN_SEMICOLON` for `;` (optional)
- [ ] Add `TOKEN_HASH` for `#` (comment start)

### 1.6 Special Tokens
- [ ] Add `TOKEN_AT` for `@` (directives)
- [ ] Add `TOKEN_UNDERSCORE` for `_` (wildcard)

### 1.7 Token Structure
- [ ] Create `Token` struct in `token.h`
- [ ] Add `TokenType type` field
- [ ] Add `const char* start` field (pointer to source)
- [ ] Add `int length` field
- [ ] Add `int line` field
- [ ] Add `int column` field
- [ ] Create `token_type_name()` function for debugging

### 1.8 Lexer Structure
- [ ] Create `include/lexer.h`
- [ ] Create `src/lexer.c`
- [ ] Define `Lexer` struct
- [ ] Add `const char* source` field
- [ ] Add `const char* start` field (token start)
- [ ] Add `const char* current` field (current char)
- [ ] Add `int line` field
- [ ] Add `int column` field

### 1.9 Lexer Core Functions
- [ ] Implement `lexer_init(Lexer*, const char* source)`
- [ ] Implement `lexer_next_token(Lexer*)` signature
- [ ] Implement `is_at_end()` helper
- [ ] Implement `advance()` helper (consume char)
- [ ] Implement `peek()` helper (look at current)
- [ ] Implement `peek_next()` helper (look ahead)
- [ ] Implement `match(char expected)` helper

### 1.10 Token Creation
- [ ] Implement `make_token(TokenType)` helper
- [ ] Implement `error_token(const char* message)` helper
- [ ] Set line/column in token creation

### 1.11 Whitespace and Comments
- [ ] Implement `skip_whitespace()` function
- [ ] Handle spaces
- [ ] Handle tabs
- [ ] Handle `\r`
- [ ] Handle `\n` (increment line, reset column)
- [ ] Handle `#` single-line comments
- [ ] Skip until end of line for comments

### 1.12 Number Lexing
- [ ] Implement `scan_number()` function
- [ ] Consume leading digits
- [ ] Check for `.` followed by digit (float)
- [ ] Consume fractional digits
- [ ] Return `TOKEN_INT` or `TOKEN_FLOAT`
- [ ] Handle `_` as digit separator (optional)

### 1.13 String Lexing
- [ ] Implement `scan_string()` function
- [ ] Consume opening `"`
- [ ] Consume characters until closing `"`
- [ ] Handle unterminated string error
- [ ] Handle `\n` escape sequence
- [ ] Handle `\t` escape sequence
- [ ] Handle `\\` escape sequence
- [ ] Handle `\"` escape sequence
- [ ] Handle `\0` escape sequence

### 1.14 Identifier and Keyword Lexing
- [ ] Implement `scan_identifier()` function
- [ ] Consume alphanumeric and `_` characters
- [ ] Implement `check_keyword()` helper
- [ ] Create keyword lookup (trie or switch)
- [ ] Return `TOKEN_IDENTIFIER` or keyword token

### 1.15 Operator Lexing
- [ ] Handle single-char operators in `lexer_next_token`
- [ ] Handle `=` vs `==` vs `=>`
- [ ] Handle `:` vs `::` vs `:=`
- [ ] Handle `!` vs `!=`
- [ ] Handle `<` vs `<=`
- [ ] Handle `>` vs `>=`
- [ ] Handle `.` vs `..`
- [ ] Handle `-` vs `->`

### 1.16 Lexer Testing
- [ ] Create `tests/test_lexer.c`
- [ ] Test empty input
- [ ] Test single tokens
- [ ] Test all keywords
- [ ] Test all operators
- [ ] Test integers
- [ ] Test floats
- [ ] Test strings with escapes
- [ ] Test comments
- [ ] Test multiline input
- [ ] Test error cases

---

## Phase 2: AST (Abstract Syntax Tree)

### 2.1 AST Node Types
- [ ] Create `include/ast.h`
- [ ] Define `NodeType` enum

### 2.2 Expression Node Types
- [ ] Add `NODE_LITERAL_INT`
- [ ] Add `NODE_LITERAL_FLOAT`
- [ ] Add `NODE_LITERAL_STRING`
- [ ] Add `NODE_LITERAL_BOOL`
- [ ] Add `NODE_LITERAL_NIL`
- [ ] Add `NODE_IDENTIFIER`
- [ ] Add `NODE_BINARY` (e.g., `a + b`)
- [ ] Add `NODE_UNARY` (e.g., `-x`, `not x`)
- [ ] Add `NODE_CALL` (function call)
- [ ] Add `NODE_INDEX` (e.g., `arr[i]`)
- [ ] Add `NODE_FIELD` (e.g., `obj.field`)
- [ ] Add `NODE_ARRAY` (array literal)
- [ ] Add `NODE_TABLE` (table literal)
- [ ] Add `NODE_LAMBDA` (anonymous function)
- [ ] Add `NODE_RANGE` (e.g., `0..10`)
- [ ] Add `NODE_ADDRESS_OF` (e.g., `&x`)

### 2.3 Statement Node Types
- [ ] Add `NODE_VAR_DECL` (`:=`)
- [ ] Add `NODE_CONST_DECL` (`::`)
- [ ] Add `NODE_ASSIGNMENT` (`=`)
- [ ] Add `NODE_EXPR_STMT` (expression as statement)
- [ ] Add `NODE_BLOCK` (block of statements)
- [ ] Add `NODE_IF`
- [ ] Add `NODE_WHILE`
- [ ] Add `NODE_FOR`
- [ ] Add `NODE_RETURN`
- [ ] Add `NODE_BREAK`
- [ ] Add `NODE_CONTINUE`
- [ ] Add `NODE_FN_DECL` (function declaration)
- [ ] Add `NODE_MATCH`
- [ ] Add `NODE_DEFER`

### 2.4 Special Node Types
- [ ] Add `NODE_PROGRAM` (root node)
- [ ] Add `NODE_IMPORT` (`@import`)
- [ ] Add `NODE_C_BLOCK` (`@c { ... }`)
- [ ] Add `NODE_CTYPE` (`@ctype`)

### 2.5 AST Node Structure
- [ ] Create `AstNode` struct
- [ ] Add `NodeType type` field
- [ ] Add `int line` field
- [ ] Add `int column` field
- [ ] Create union for node-specific data

### 2.6 Literal Node Data
- [ ] Add `int64_t int_value` for int literals
- [ ] Add `double float_value` for float literals
- [ ] Add `char* string_value` for string literals
- [ ] Add `bool bool_value` for bool literals

### 2.7 Identifier Node Data
- [ ] Add `char* name` for identifiers
- [ ] Add `int name_length` field

### 2.8 Binary Expression Data
- [ ] Add `TokenType operator` field
- [ ] Add `AstNode* left` field
- [ ] Add `AstNode* right` field

### 2.9 Unary Expression Data
- [ ] Add `TokenType operator` field
- [ ] Add `AstNode* operand` field

### 2.10 Call Expression Data
- [ ] Add `AstNode* callee` field
- [ ] Add `AstNode** arguments` field
- [ ] Add `int arg_count` field

### 2.11 Index Expression Data
- [ ] Add `AstNode* object` field
- [ ] Add `AstNode* index` field

### 2.12 Field Access Data
- [ ] Add `AstNode* object` field
- [ ] Add `char* field_name` field

### 2.13 Array/Table Literal Data
- [ ] Add `AstNode** elements` field
- [ ] Add `int element_count` field
- [ ] Add `char** keys` for table (NULL for array)

### 2.14 Variable Declaration Data
- [ ] Add `char* name` field
- [ ] Add `AstNode* initializer` field
- [ ] Add `bool is_const` field

### 2.15 Assignment Data
- [ ] Add `AstNode* target` field
- [ ] Add `AstNode* value` field

### 2.16 Block Data
- [ ] Add `AstNode** statements` field
- [ ] Add `int statement_count` field

### 2.17 If Statement Data
- [ ] Add `AstNode* condition` field
- [ ] Add `AstNode* then_branch` field
- [ ] Add `AstNode* else_branch` field (nullable)

### 2.18 While Statement Data
- [ ] Add `AstNode* condition` field
- [ ] Add `AstNode* body` field

### 2.19 For Statement Data
- [ ] Add `char* iterator_name` field
- [ ] Add `AstNode* iterable` field
- [ ] Add `AstNode* body` field

### 2.20 Function Declaration Data
- [ ] Add `char* name` field
- [ ] Add `char** parameters` field
- [ ] Add `int param_count` field
- [ ] Add `AstNode* body` field

### 2.21 Return Statement Data
- [ ] Add `AstNode* value` field (nullable)

### 2.22 Match Statement Data
- [ ] Add `AstNode* value` field
- [ ] Add `AstNode** patterns` field
- [ ] Add `AstNode** bodies` field
- [ ] Add `int arm_count` field

### 2.23 Import Data
- [ ] Add `char* path` field

### 2.24 AST Memory Management
- [ ] Create `src/ast.c`
- [ ] Implement `ast_create_node(NodeType)` function
- [ ] Implement `ast_free_node(AstNode*)` function
- [ ] Implement `ast_free_tree(AstNode*)` for recursive free
- [ ] Create node constructor for each type

### 2.25 AST Debug Printing
- [ ] Implement `ast_print(AstNode*)` function
- [ ] Print with indentation for tree structure
- [ ] Print node type names
- [ ] Print literal values
- [ ] Print operator symbols

---

## Phase 3: Parser

### 3.1 Parser Structure
- [ ] Create `include/parser.h`
- [ ] Create `src/parser.c`
- [ ] Define `Parser` struct
- [ ] Add `Lexer* lexer` field
- [ ] Add `Token current` field
- [ ] Add `Token previous` field
- [ ] Add `bool had_error` field
- [ ] Add `bool panic_mode` field

### 3.2 Parser Core Functions
- [ ] Implement `parser_init(Parser*, Lexer*)`
- [ ] Implement `advance()` - consume token
- [ ] Implement `check(TokenType)` - check current
- [ ] Implement `match(TokenType)` - check and consume
- [ ] Implement `consume(TokenType, message)` - expect token
- [ ] Implement `synchronize()` - error recovery

### 3.3 Error Handling
- [ ] Implement `error_at(Token*, message)` function
- [ ] Implement `error(message)` - error at previous
- [ ] Implement `error_at_current(message)` - error at current
- [ ] Print line number in errors
- [ ] Print column in errors
- [ ] Print token text in errors

### 3.4 Expression Parsing - Precedence
- [ ] Define `Precedence` enum
- [ ] Add `PREC_NONE`
- [ ] Add `PREC_ASSIGNMENT` (=)
- [ ] Add `PREC_OR` (or)
- [ ] Add `PREC_AND` (and)
- [ ] Add `PREC_EQUALITY` (== !=)
- [ ] Add `PREC_COMPARISON` (< > <= >=)
- [ ] Add `PREC_RANGE` (..)
- [ ] Add `PREC_TERM` (+ -)
- [ ] Add `PREC_FACTOR` (* / %)
- [ ] Add `PREC_UNARY` (- not &)
- [ ] Add `PREC_CALL` (. () [])
- [ ] Add `PREC_PRIMARY`

### 3.5 Pratt Parser Setup
- [ ] Define `ParseRule` struct
- [ ] Add `prefix` function pointer
- [ ] Add `infix` function pointer
- [ ] Add `precedence` field
- [ ] Create `rules` table indexed by TokenType
- [ ] Implement `get_rule(TokenType)` function

### 3.6 Expression Parsing Core
- [ ] Implement `parse_expression()` function
- [ ] Implement `parse_precedence(Precedence)` function
- [ ] Call prefix rule for current token
- [ ] Loop while precedence allows infix
- [ ] Call infix rule

### 3.7 Literal Parsing
- [ ] Implement `parse_number()` - int and float
- [ ] Implement `parse_string()` - string literal
- [ ] Implement `parse_boolean()` - true/false
- [ ] Implement `parse_nil()` - nil
- [ ] Register in rules table as prefix

### 3.8 Identifier Parsing
- [ ] Implement `parse_identifier()` function
- [ ] Create identifier node from token
- [ ] Register in rules table as prefix

### 3.9 Grouping
- [ ] Implement `parse_grouping()` - `( expr )`
- [ ] Consume `(`
- [ ] Parse expression
- [ ] Consume `)`
- [ ] Register `(` in rules table as prefix

### 3.10 Unary Operators
- [ ] Implement `parse_unary()` function
- [ ] Save operator token
- [ ] Parse operand with PREC_UNARY
- [ ] Create unary node
- [ ] Register `-`, `not`, `&` as prefix

### 3.11 Binary Operators
- [ ] Implement `parse_binary()` function
- [ ] Get operator and precedence
- [ ] Parse right side with precedence + 1
- [ ] Create binary node
- [ ] Register `+`, `-`, `*`, `/`, `%` as infix
- [ ] Register `==`, `!=`, `<`, `>`, `<=`, `>=` as infix
- [ ] Register `and`, `or` as infix

### 3.12 Range Operator
- [ ] Implement `parse_range()` function
- [ ] Parse end expression
- [ ] Create range node
- [ ] Register `..` as infix

### 3.13 Call Expression
- [ ] Implement `parse_call()` function
- [ ] Parse argument list
- [ ] Handle empty arguments
- [ ] Handle multiple arguments with comma
- [ ] Consume `)`
- [ ] Create call node
- [ ] Register `(` as infix

### 3.14 Index Expression
- [ ] Implement `parse_index()` function
- [ ] Parse index expression
- [ ] Consume `]`
- [ ] Create index node
- [ ] Register `[` as infix

### 3.15 Field Access
- [ ] Implement `parse_field()` function
- [ ] Consume identifier after `.`
- [ ] Create field access node
- [ ] Register `.` as infix

### 3.16 Array Literal
- [ ] Implement `parse_array()` function
- [ ] Consume `[`
- [ ] Parse comma-separated elements
- [ ] Handle trailing comma
- [ ] Consume `]`
- [ ] Create array node
- [ ] Register `[` as prefix

### 3.17 Table Literal
- [ ] Implement `parse_table()` function
- [ ] Consume `{`
- [ ] Parse `key: value` pairs
- [ ] Handle trailing comma
- [ ] Consume `}`
- [ ] Create table node
- [ ] Register `{` as prefix (context-dependent)

### 3.18 Anonymous Function
- [ ] Implement `parse_lambda()` function
- [ ] Consume `fn`
- [ ] Parse parameter list
- [ ] Parse body block
- [ ] Create lambda node
- [ ] Register `fn` as prefix

### 3.19 Statement Parsing
- [ ] Implement `parse_statement()` function
- [ ] Dispatch based on current token
- [ ] Handle expression statements

### 3.20 Variable Declaration
- [ ] Implement `parse_var_decl()` function
- [ ] Parse identifier
- [ ] Consume `:=`
- [ ] Parse initializer expression
- [ ] Create var decl node

### 3.21 Constant Declaration
- [ ] Implement `parse_const_decl()` function
- [ ] Parse identifier
- [ ] Consume `::`
- [ ] Parse initializer expression
- [ ] Create const decl node

### 3.22 Assignment Statement
- [ ] Implement `parse_assignment()` function
- [ ] Parse left-hand side (identifier, index, field)
- [ ] Consume `=`
- [ ] Parse right-hand side
- [ ] Create assignment node

### 3.23 Block Statement
- [ ] Implement `parse_block()` function
- [ ] Consume `{`
- [ ] Parse statements until `}`
- [ ] Consume `}`
- [ ] Create block node

### 3.24 If Statement
- [ ] Implement `parse_if()` function
- [ ] Consume `if`
- [ ] Parse condition expression
- [ ] Parse then block
- [ ] Handle `elif` as nested if
- [ ] Handle `else` block
- [ ] Create if node

### 3.25 While Statement
- [ ] Implement `parse_while()` function
- [ ] Consume `while`
- [ ] Parse condition expression
- [ ] Parse body block
- [ ] Create while node

### 3.26 For Statement
- [ ] Implement `parse_for()` function
- [ ] Consume `for`
- [ ] Parse iterator identifier
- [ ] Consume `in`
- [ ] Parse iterable expression
- [ ] Parse body block
- [ ] Create for node

### 3.27 Return Statement
- [ ] Implement `parse_return()` function
- [ ] Consume `return`
- [ ] Parse optional value expression
- [ ] Create return node

### 3.28 Break/Continue
- [ ] Implement `parse_break()` function
- [ ] Implement `parse_continue()` function
- [ ] Create respective nodes

### 3.29 Function Declaration
- [ ] Implement `parse_fn_decl()` function
- [ ] Consume `fn`
- [ ] Parse function name
- [ ] Consume `(`
- [ ] Parse parameter list
- [ ] Consume `)`
- [ ] Parse optional return type annotation
- [ ] Parse body block
- [ ] Create fn decl node

### 3.30 Match Statement
- [ ] Implement `parse_match()` function
- [ ] Consume `match`
- [ ] Parse value expression
- [ ] Consume `{`
- [ ] Parse match arms
- [ ] Each arm: pattern `=>` expression/block
- [ ] Handle `_` wildcard pattern
- [ ] Handle range patterns
- [ ] Handle literal patterns
- [ ] Consume `}`
- [ ] Create match node

### 3.31 Defer Statement
- [ ] Implement `parse_defer()` function
- [ ] Consume `defer`
- [ ] Parse statement/expression
- [ ] Create defer node

### 3.32 Import Directive
- [ ] Implement `parse_import()` function
- [ ] Consume `@`
- [ ] Consume `import`
- [ ] Parse string literal (path)
- [ ] Create import node

### 3.33 C Block Directive
- [ ] Implement `parse_c_block()` function
- [ ] Consume `@`
- [ ] Consume `c`
- [ ] Consume `{`
- [ ] Capture raw C code until matching `}`
- [ ] Create c_block node

### 3.34 Program Parsing
- [ ] Implement `parse_program()` function
- [ ] Parse statements until EOF
- [ ] Create program node with statement list
- [ ] Return AST root

### 3.35 Parser Entry Point
- [ ] Implement `parse(const char* source)` function
- [ ] Create lexer
- [ ] Create parser
- [ ] Call `parse_program()`
- [ ] Return AST or NULL on error

### 3.36 Parser Testing
- [ ] Create `tests/test_parser.c`
- [ ] Test literal expressions
- [ ] Test binary expressions
- [ ] Test operator precedence
- [ ] Test unary expressions
- [ ] Test function calls
- [ ] Test variable declarations
- [ ] Test if statements
- [ ] Test while loops
- [ ] Test for loops
- [ ] Test function declarations
- [ ] Test complex nested structures
- [ ] Test error recovery

---

## Phase 4: Value System

### 4.1 Value Type Enum
- [ ] Create `include/value.h`
- [ ] Define `ValueType` enum
- [ ] Add `VAL_NIL`
- [ ] Add `VAL_BOOL`
- [ ] Add `VAL_INT`
- [ ] Add `VAL_FLOAT`
- [ ] Add `VAL_STRING`
- [ ] Add `VAL_ARRAY`
- [ ] Add `VAL_TABLE`
- [ ] Add `VAL_FUNCTION`
- [ ] Add `VAL_NATIVE_FN`
- [ ] Add `VAL_POINTER`
- [ ] Add `VAL_CSTRUCT`

### 4.2 Value Structure
- [ ] Create `Value` struct
- [ ] Add `ValueType type` field
- [ ] Create union for value data
- [ ] Add `bool boolean` in union
- [ ] Add `int64_t integer` in union
- [ ] Add `double floating` in union
- [ ] Add `Object* object` in union (for heap types)

### 4.3 Object Header
- [ ] Create `Object` struct (base for heap objects)
- [ ] Add `ObjectType type` field
- [ ] Add `int ref_count` field
- [ ] Add `Object* next` for GC list
- [ ] Add `bool marked` for cycle detection

### 4.4 String Object
- [ ] Create `ObjString` struct
- [ ] Embed `Object` header
- [ ] Add `int length` field
- [ ] Add `uint32_t hash` field
- [ ] Add `char chars[]` flexible array

### 4.5 Array Object
- [ ] Create `ObjArray` struct
- [ ] Embed `Object` header
- [ ] Add `Value* elements` field
- [ ] Add `int count` field
- [ ] Add `int capacity` field

### 4.6 Table Object
- [ ] Create `ObjTable` struct
- [ ] Embed `Object` header
- [ ] Add entry struct (key-value pair)
- [ ] Add `Entry* entries` field
- [ ] Add `int count` field
- [ ] Add `int capacity` field

### 4.7 Function Object
- [ ] Create `ObjFunction` struct
- [ ] Embed `Object` header
- [ ] Add `char* name` field
- [ ] Add `int arity` field
- [ ] Add `AstNode* body` field (for tree-walk)
- [ ] Add `ObjUpvalue** upvalues` for closures
- [ ] Add `int upvalue_count` field

### 4.8 Native Function Object
- [ ] Create `NativeFn` typedef for C function pointer
- [ ] Create `ObjNative` struct
- [ ] Embed `Object` header
- [ ] Add `NativeFn function` field
- [ ] Add `int arity` field

### 4.9 Pointer Object
- [ ] Create `ObjPointer` struct
- [ ] Embed `Object` header
- [ ] Add `void* ptr` field
- [ ] Add `char* type_name` field (C type)

### 4.10 Value Creation Macros/Functions
- [ ] Create `src/value.c`
- [ ] Implement `NIL_VAL` macro
- [ ] Implement `BOOL_VAL(b)` macro
- [ ] Implement `INT_VAL(n)` macro
- [ ] Implement `FLOAT_VAL(n)` macro
- [ ] Implement `OBJ_VAL(o)` macro

### 4.11 Value Type Check Macros
- [ ] Implement `IS_NIL(v)` macro
- [ ] Implement `IS_BOOL(v)` macro
- [ ] Implement `IS_INT(v)` macro
- [ ] Implement `IS_FLOAT(v)` macro
- [ ] Implement `IS_STRING(v)` macro
- [ ] Implement `IS_ARRAY(v)` macro
- [ ] Implement `IS_TABLE(v)` macro
- [ ] Implement `IS_FUNCTION(v)` macro
- [ ] Implement `IS_NATIVE(v)` macro
- [ ] Implement `IS_POINTER(v)` macro

### 4.12 Value Extraction Macros
- [ ] Implement `AS_BOOL(v)` macro
- [ ] Implement `AS_INT(v)` macro
- [ ] Implement `AS_FLOAT(v)` macro
- [ ] Implement `AS_STRING(v)` macro
- [ ] Implement `AS_ARRAY(v)` macro
- [ ] Implement `AS_TABLE(v)` macro
- [ ] Implement `AS_FUNCTION(v)` macro
- [ ] Implement `AS_NATIVE(v)` macro
- [ ] Implement `AS_POINTER(v)` macro

### 4.13 Value Operations
- [ ] Implement `value_equals(Value, Value)` function
- [ ] Implement `value_print(Value)` function
- [ ] Implement `value_to_string(Value)` function
- [ ] Implement `value_is_truthy(Value)` function

### 4.14 String Operations
- [ ] Implement `string_create(const char*, int len)` function
- [ ] Implement `string_hash(const char*, int len)` function
- [ ] Implement `string_concat(ObjString*, ObjString*)` function
- [ ] Implement string interning table
- [ ] Implement `string_intern(ObjString*)` function

### 4.15 Array Operations
- [ ] Implement `array_create()` function
- [ ] Implement `array_push(ObjArray*, Value)` function
- [ ] Implement `array_pop(ObjArray*)` function
- [ ] Implement `array_get(ObjArray*, int)` function
- [ ] Implement `array_set(ObjArray*, int, Value)` function
- [ ] Implement `array_length(ObjArray*)` function

### 4.16 Table Operations
- [ ] Implement `table_create()` function
- [ ] Implement `table_get(ObjTable*, ObjString* key)` function
- [ ] Implement `table_set(ObjTable*, ObjString* key, Value)` function
- [ ] Implement `table_delete(ObjTable*, ObjString* key)` function
- [ ] Implement table growing/rehashing

---

## Phase 5: Memory Management

### 5.1 Memory Allocation
- [ ] Create `include/memory.h`
- [ ] Create `src/memory.c`
- [ ] Implement `mem_alloc(size_t)` function
- [ ] Implement `mem_realloc(void*, size_t, size_t)` function
- [ ] Implement `mem_free(void*, size_t)` function
- [ ] Track total bytes allocated

### 5.2 Reference Counting
- [ ] Implement `obj_incref(Object*)` function
- [ ] Implement `obj_decref(Object*)` function
- [ ] Free object when refcount hits 0
- [ ] Handle nested object decrefs

### 5.3 Object Allocation
- [ ] Implement `allocate_object(size_t, ObjectType)` function
- [ ] Initialize refcount to 1
- [ ] Add to object list

### 5.4 Object Deallocation
- [ ] Implement `free_object(Object*)` function
- [ ] Handle each object type
- [ ] Free strings
- [ ] Free arrays (and decref elements)
- [ ] Free tables (and decref entries)
- [ ] Free functions

### 5.5 Cycle Detection (Optional for Phase 1)
- [ ] Implement mark phase
- [ ] Implement sweep phase
- [ ] Track potential cycle roots
- [ ] Run cycle detection periodically

---

## Phase 6: Tree-Walking Interpreter

### 6.1 Environment Structure
- [ ] Create `include/env.h`
- [ ] Create `src/env.c`
- [ ] Define `Environment` struct
- [ ] Add `ObjTable* variables` field
- [ ] Add `Environment* enclosing` field (parent scope)

### 6.2 Environment Operations
- [ ] Implement `env_create(Environment* enclosing)` function
- [ ] Implement `env_destroy(Environment*)` function
- [ ] Implement `env_define(Environment*, char* name, Value)` function
- [ ] Implement `env_get(Environment*, char* name)` function
- [ ] Implement `env_set(Environment*, char* name, Value)` function
- [ ] Handle undefined variable errors

### 6.3 Interpreter Structure
- [ ] Create `include/interp.h`
- [ ] Create `src/interp.c`
- [ ] Define `Interpreter` struct
- [ ] Add `Environment* global` field
- [ ] Add `Environment* current` field
- [ ] Add `Value return_value` field
- [ ] Add `bool returning` flag
- [ ] Add `bool breaking` flag
- [ ] Add `bool continuing` flag

### 6.4 Interpreter Initialization
- [ ] Implement `interp_init(Interpreter*)` function
- [ ] Create global environment
- [ ] Register built-in functions

### 6.5 Expression Evaluation
- [ ] Implement `eval(Interpreter*, AstNode*)` function
- [ ] Dispatch based on node type
- [ ] Return `Value`

### 6.6 Literal Evaluation
- [ ] Implement `eval_int_literal()`
- [ ] Implement `eval_float_literal()`
- [ ] Implement `eval_string_literal()`
- [ ] Implement `eval_bool_literal()`
- [ ] Implement `eval_nil_literal()`

### 6.7 Identifier Evaluation
- [ ] Implement `eval_identifier()`
- [ ] Look up in current environment
- [ ] Walk up enclosing environments
- [ ] Error if undefined

### 6.8 Binary Expression Evaluation
- [ ] Implement `eval_binary()`
- [ ] Evaluate left operand
- [ ] Evaluate right operand
- [ ] Handle `+` for numbers
- [ ] Handle `+` for strings (concatenation)
- [ ] Handle `-`, `*`, `/`, `%` for numbers
- [ ] Handle `==`, `!=` for all types
- [ ] Handle `<`, `>`, `<=`, `>=` for numbers
- [ ] Handle `and`, `or` with short-circuit
- [ ] Type check operands

### 6.9 Unary Expression Evaluation
- [ ] Implement `eval_unary()`
- [ ] Handle `-` (negate)
- [ ] Handle `not` (boolean not)
- [ ] Handle `&` (address-of for C interop)

### 6.10 Call Expression Evaluation
- [ ] Implement `eval_call()`
- [ ] Evaluate callee
- [ ] Check callee is callable
- [ ] Evaluate arguments
- [ ] Check arity matches
- [ ] Handle Brisk function call
- [ ] Handle native function call

### 6.11 Function Call Mechanics
- [ ] Create new environment for function
- [ ] Bind parameters to arguments
- [ ] Execute function body
- [ ] Handle return value
- [ ] Restore environment

### 6.12 Index Expression Evaluation
- [ ] Implement `eval_index()`
- [ ] Evaluate object
- [ ] Evaluate index
- [ ] Handle array indexing
- [ ] Handle table indexing (string key)

### 6.13 Field Access Evaluation
- [ ] Implement `eval_field()`
- [ ] Evaluate object
- [ ] Look up field name
- [ ] Handle table field access
- [ ] Handle C struct field access (later)

### 6.14 Array Literal Evaluation
- [ ] Implement `eval_array()`
- [ ] Create new array
- [ ] Evaluate each element
- [ ] Add to array

### 6.15 Table Literal Evaluation
- [ ] Implement `eval_table()`
- [ ] Create new table
- [ ] Evaluate each key-value pair
- [ ] Add to table

### 6.16 Range Evaluation
- [ ] Implement `eval_range()`
- [ ] Create range object or array
- [ ] Store start and end values

### 6.17 Lambda Evaluation
- [ ] Implement `eval_lambda()`
- [ ] Create function object
- [ ] Capture current environment (closures)

### 6.18 Statement Execution
- [ ] Implement `exec(Interpreter*, AstNode*)` function
- [ ] Dispatch based on node type
- [ ] Handle control flow flags

### 6.19 Variable Declaration Execution
- [ ] Implement `exec_var_decl()`
- [ ] Evaluate initializer
- [ ] Define in current environment
- [ ] Handle redeclaration error

### 6.20 Constant Declaration Execution
- [ ] Implement `exec_const_decl()`
- [ ] Same as var but mark as constant
- [ ] Error on reassignment

### 6.21 Assignment Execution
- [ ] Implement `exec_assignment()`
- [ ] Handle identifier assignment
- [ ] Handle index assignment
- [ ] Handle field assignment
- [ ] Check for const violation

### 6.22 Block Execution
- [ ] Implement `exec_block()`
- [ ] Create new environment
- [ ] Execute each statement
- [ ] Restore environment
- [ ] Handle control flow flags

### 6.23 If Statement Execution
- [ ] Implement `exec_if()`
- [ ] Evaluate condition
- [ ] Check truthiness
- [ ] Execute then branch or else branch

### 6.24 While Loop Execution
- [ ] Implement `exec_while()`
- [ ] Loop while condition is truthy
- [ ] Execute body
- [ ] Handle break flag
- [ ] Handle continue flag

### 6.25 For Loop Execution
- [ ] Implement `exec_for()`
- [ ] Evaluate iterable
- [ ] Handle range iteration
- [ ] Handle array iteration
- [ ] Bind iterator variable each iteration
- [ ] Execute body
- [ ] Handle break/continue

### 6.26 Return Execution
- [ ] Implement `exec_return()`
- [ ] Evaluate return value
- [ ] Set returning flag
- [ ] Store return value

### 6.27 Break/Continue Execution
- [ ] Implement `exec_break()`
- [ ] Set breaking flag
- [ ] Implement `exec_continue()`
- [ ] Set continuing flag

### 6.28 Function Declaration Execution
- [ ] Implement `exec_fn_decl()`
- [ ] Create function object
- [ ] Bind to name in current environment

### 6.29 Match Statement Execution
- [ ] Implement `exec_match()`
- [ ] Evaluate value
- [ ] Check each pattern
- [ ] Execute matching arm body
- [ ] Handle wildcard pattern

### 6.30 Defer Execution
- [ ] Implement defer stack per scope
- [ ] Push deferred statement on entry
- [ ] Pop and execute on scope exit
- [ ] Execute even on early return/break

### 6.31 Program Execution
- [ ] Implement `exec_program()`
- [ ] Execute each top-level statement
- [ ] Handle top-level expressions

### 6.32 Interpreter Entry Point
- [ ] Implement `interpret(const char* source)` function
- [ ] Parse source to AST
- [ ] Create interpreter
- [ ] Execute program
- [ ] Return result

---

## Phase 7: Built-in Functions

### 7.1 I/O Functions
- [ ] Implement `print(...)` - variadic print
- [ ] Implement `println(...)` - print with newline
- [ ] Implement `input()` - read line from stdin
- [ ] Implement `input(prompt)` - print prompt, read line

### 7.2 Type Functions
- [ ] Implement `type(value)` - return type as string
- [ ] Implement `int(value)` - convert to int
- [ ] Implement `float(value)` - convert to float
- [ ] Implement `str(value)` - convert to string
- [ ] Implement `bool(value)` - convert to bool

### 7.3 Array Functions
- [ ] Implement `len(array)` - array length
- [ ] Implement `push(array, value)` - append
- [ ] Implement `pop(array)` - remove last
- [ ] Implement `insert(array, index, value)`
- [ ] Implement `remove(array, index)`

### 7.4 String Functions
- [ ] Implement `len(string)` - string length
- [ ] Implement `substr(string, start, len)`
- [ ] Implement `find(string, substring)`
- [ ] Implement `replace(string, old, new)`
- [ ] Implement `split(string, delimiter)`
- [ ] Implement `join(array, delimiter)`
- [ ] Implement `upper(string)`
- [ ] Implement `lower(string)`
- [ ] Implement `trim(string)`

### 7.5 Math Functions
- [ ] Implement `abs(n)`
- [ ] Implement `min(a, b)`
- [ ] Implement `max(a, b)`
- [ ] Implement `floor(n)`
- [ ] Implement `ceil(n)`
- [ ] Implement `round(n)`
- [ ] Implement `sqrt(n)`
- [ ] Implement `pow(base, exp)`

### 7.6 Table Functions
- [ ] Implement `keys(table)` - return array of keys
- [ ] Implement `values(table)` - return array of values
- [ ] Implement `has(table, key)` - check key exists

### 7.7 Utility Functions
- [ ] Implement `assert(condition, message)`
- [ ] Implement `error(message)` - raise runtime error
- [ ] Implement `clock()` - current time in seconds

### 7.8 Register Built-ins
- [ ] Create `register_builtins(Environment*)` function
- [ ] Register all built-in functions
- [ ] Call during interpreter init

---

## Phase 8: REPL

### 8.1 REPL Structure
- [ ] Create `src/repl.c`
- [ ] Implement `run_repl()` function
- [ ] Print welcome message
- [ ] Print version

### 8.2 Input Handling
- [ ] Print prompt `> `
- [ ] Read line from stdin
- [ ] Handle EOF (exit)
- [ ] Handle empty input

### 8.3 Multi-line Input
- [ ] Detect incomplete expressions (unclosed braces/parens)
- [ ] Continue prompt `... `
- [ ] Accumulate lines

### 8.4 Evaluation
- [ ] Parse input
- [ ] Execute
- [ ] Print result (if expression)
- [ ] Print errors nicely

### 8.5 REPL Commands
- [ ] Handle `:quit` or `:exit`
- [ ] Handle `:help`
- [ ] Handle `:clear` (clear screen)
- [ ] Handle `:env` (print environment)

### 8.6 History (Optional)
- [ ] Add GNU readline library support
- [ ] `sudo apt install libreadline-dev`
- [ ] Link with `-lreadline`
- [ ] Arrow key navigation
- [ ] History file

---

## Phase 9: C Interop - Core FFI

### 9.1 libffi Integration
- [ ] Verify libffi-dev is installed
- [ ] Update Makefile with `-lffi`
- [ ] Create `include/cffi.h`
- [ ] Create `src/cffi.c`
- [ ] Include `<ffi.h>`

### 9.2 C Type Representation
- [ ] Define `CType` enum
- [ ] Add `CTYPE_VOID`
- [ ] Add `CTYPE_CHAR`
- [ ] Add `CTYPE_SHORT`
- [ ] Add `CTYPE_INT`
- [ ] Add `CTYPE_LONG`
- [ ] Add `CTYPE_LONGLONG`
- [ ] Add `CTYPE_UCHAR`, `CTYPE_USHORT`, etc.
- [ ] Add `CTYPE_FLOAT`
- [ ] Add `CTYPE_DOUBLE`
- [ ] Add `CTYPE_POINTER`
- [ ] Add `CTYPE_STRUCT`

### 9.3 C Type to ffi_type Mapping
- [ ] Implement `ctype_to_ffi(CType)` function
- [ ] Return appropriate `ffi_type*`
- [ ] Handle all basic types

### 9.4 Dynamic Library Loading (Linux)
- [ ] Create `include/dynload.h`
- [ ] Create `src/dynload.c`
- [ ] Include `<dlfcn.h>`
- [ ] Implement `lib_open(const char* path)` using `dlopen()`
- [ ] Use `RTLD_NOW | RTLD_GLOBAL` flags
- [ ] Implement `lib_close(void* handle)` using `dlclose()`
- [ ] Implement `lib_symbol(void* handle, const char* name)` using `dlsym()`
- [ ] Implement `lib_error()` using `dlerror()`
- [ ] Handle NULL library (search in already-loaded libs)

### 9.5 C Function Descriptor
- [ ] Define `CFunctionDesc` struct
- [ ] Add `char* name` field
- [ ] Add `CType return_type` field
- [ ] Add `CType* param_types` field
- [ ] Add `int param_count` field
- [ ] Add `bool is_variadic` field
- [ ] Add `void* func_ptr` field

### 9.6 FFI Call Preparation
- [ ] Implement `ffi_prep_call(CFunctionDesc*)` function
- [ ] Create `ffi_cif` structure
- [ ] Call `ffi_prep_cif` or `ffi_prep_cif_var`
- [ ] Handle prep errors

### 9.7 Type Marshalling: Brisk → C
- [ ] Implement `marshal_to_c(Value, CType, void* out)` function
- [ ] Handle nil → NULL
- [ ] Handle bool → int
- [ ] Handle int → any C integer type
- [ ] Handle float → float/double
- [ ] Handle string → char*
- [ ] Handle pointer → void*
- [ ] Handle array → pointer (with caution)

### 9.8 Type Marshalling: C → Brisk
- [ ] Implement `marshal_from_c(void* in, CType)` function
- [ ] Handle void → nil
- [ ] Handle integers → int
- [ ] Handle float/double → float
- [ ] Handle char* → string
- [ ] Handle pointers → pointer object

### 9.9 FFI Call Execution
- [ ] Implement `ffi_call_function(CFunctionDesc*, Value* args)` function
- [ ] Allocate argument storage
- [ ] Marshal each argument
- [ ] Call `ffi_call`
- [ ] Marshal return value
- [ ] Return Brisk Value

### 9.10 C Function Wrapper
- [ ] Create `ObjCFunction` struct
- [ ] Embed `Object` header
- [ ] Add `CFunctionDesc* desc` field
- [ ] Add `ffi_cif cif` field

### 9.11 Calling C Functions from Brisk
- [ ] Handle `ObjCFunction` in `eval_call()`
- [ ] Validate argument count
- [ ] Call `ffi_call_function`
- [ ] Return result

---

## Phase 10: C Header Parsing

### 10.1 Header Parser Structure
- [ ] Create `include/cheader.h`
- [ ] Create `src/cheader.c`
- [ ] Define `CHeaderParser` struct
- [ ] Add lexer for C syntax

### 10.2 C Lexer (Simplified)
- [ ] Tokenize C identifiers
- [ ] Tokenize C keywords (int, void, struct, etc.)
- [ ] Tokenize C punctuation
- [ ] Handle C comments (`//` and `/* */`)
- [ ] Handle preprocessor directives (skip or basic)

### 10.3 Type Parsing
- [ ] Implement `parse_c_type()` function
- [ ] Parse `void`
- [ ] Parse `char`, `short`, `int`, `long`
- [ ] Parse `unsigned` modifier
- [ ] Parse `signed` modifier
- [ ] Parse `float`, `double`
- [ ] Parse pointer (`*`)
- [ ] Parse `const` modifier
- [ ] Parse typedef names

### 10.4 Function Declaration Parsing
- [ ] Implement `parse_function_decl()` function
- [ ] Parse return type
- [ ] Parse function name
- [ ] Parse `(`
- [ ] Parse parameter list
- [ ] Parse `)`
- [ ] Handle `void` parameter
- [ ] Handle variadic `...`
- [ ] Create `CFunctionDesc`

### 10.5 Struct Parsing
- [ ] Define `CStructDesc` struct
- [ ] Add field list
- [ ] Implement `parse_struct()` function
- [ ] Parse `struct` keyword
- [ ] Parse struct name
- [ ] Parse `{`
- [ ] Parse field declarations
- [ ] Parse `}`
- [ ] Calculate field offsets (basic)

### 10.6 Enum Parsing
- [ ] Define `CEnumDesc` struct
- [ ] Implement `parse_enum()` function
- [ ] Parse `enum` keyword
- [ ] Parse enum name
- [ ] Parse `{`
- [ ] Parse enumerators
- [ ] Handle explicit values
- [ ] Parse `}`

### 10.7 Typedef Parsing
- [ ] Implement `parse_typedef()` function
- [ ] Track typedef name → type mapping

### 10.8 Macro Handling (Simple)
- [ ] Implement `parse_define()` function
- [ ] Handle `#define NAME value`
- [ ] Parse integer constant macros
- [ ] Parse simple expression macros
- [ ] Store as Brisk constants

### 10.9 Header File Loading
- [ ] Implement `load_header(const char* path)` function
- [ ] Read file contents
- [ ] Handle include paths
- [ ] Parse declarations
- [ ] Store in symbol table

### 10.10 Include Path Handling (Linux)
- [ ] Define default include paths:
  - [ ] `/usr/include`
  - [ ] `/usr/local/include`
  - [ ] `/usr/include/x86_64-linux-gnu` (for system headers)
- [ ] Allow custom include paths via `-I` flag
- [ ] Handle `<>` vs `""` includes

### 10.11 System Header Special Cases
- [ ] Handle `stdio.h` common functions
- [ ] Handle `stdlib.h` common functions
- [ ] Handle `string.h` common functions
- [ ] Handle `math.h` common functions
- [ ] Pre-define common macros (NULL, etc.)

---

## Phase 11: @import Implementation

### 11.1 Import Directive Execution
- [ ] Implement `exec_import()` function
- [ ] Extract header path from node
- [ ] Determine if system or local header
- [ ] Find header file

### 11.2 Library Discovery (Linux)
- [ ] Map header to library:
  - [ ] `stdio.h`, `stdlib.h`, etc. → libc (no explicit load needed)
  - [ ] `math.h` → libm (`-lm`, `libm.so.6`)
  - [ ] `SDL2/SDL.h` → `libSDL2.so` or `libSDL2-2.0.so.0`
  - [ ] `raylib.h` → `libraylib.so`
- [ ] Search in standard library paths:
  - [ ] `/usr/lib`
  - [ ] `/usr/lib/x86_64-linux-gnu`
  - [ ] `/usr/local/lib`
- [ ] Allow explicit library annotation

### 11.3 Library Loading on Import
- [ ] Load associated library with `dlopen()`
- [ ] Cache loaded libraries
- [ ] Handle library not found error

### 11.4 Symbol Registration
- [ ] Parse header declarations
- [ ] Create `ObjCFunction` for each function
- [ ] Look up function pointer with `dlsym()`
- [ ] Register in global environment

### 11.5 Constant Registration
- [ ] Register enum values
- [ ] Register macro constants
- [ ] Make available as Brisk constants

### 11.6 Struct Registration
- [ ] Create struct type descriptors
- [ ] Register struct constructors
- [ ] Allow `StructName{}` syntax

### 11.7 Import Caching
- [ ] Track already-imported headers
- [ ] Don't re-import same header
- [ ] Handle circular includes

---

## Phase 12: C Struct Support

### 12.1 C Struct Object
- [ ] Create `ObjCStruct` struct
- [ ] Embed `Object` header
- [ ] Add `CStructDesc* desc` field
- [ ] Add `void* data` field (raw C memory)

### 12.2 Struct Allocation
- [ ] Implement `cstruct_create(CStructDesc*)` function
- [ ] Allocate raw memory for struct
- [ ] Zero-initialize
- [ ] Return `ObjCStruct`

### 12.3 Field Access
- [ ] Implement `cstruct_get_field(ObjCStruct*, char* name)` function
- [ ] Find field in descriptor
- [ ] Calculate offset
- [ ] Read raw memory
- [ ] Marshal to Brisk value

### 12.4 Field Assignment
- [ ] Implement `cstruct_set_field(ObjCStruct*, char* name, Value)` function
- [ ] Find field in descriptor
- [ ] Calculate offset
- [ ] Marshal Brisk value to C
- [ ] Write raw memory

### 12.5 Struct in Interpreter
- [ ] Handle `cstruct` in `eval_field()`
- [ ] Handle `cstruct` field assignment
- [ ] Handle passing structs to C functions
- [ ] Handle receiving structs from C functions

### 12.6 Address-of Operator
- [ ] Implement `&struct` for C interop
- [ ] Return pointer to struct data
- [ ] Required for functions like `SDL_PollEvent(&event)`

---

## Phase 13: Callbacks (C calling Brisk)

### 13.1 Closure for FFI
- [ ] Use `ffi_closure` from libffi
- [ ] Implement `create_c_callback(ObjFunction*)` function
- [ ] Allocate closure with `ffi_closure_alloc()`
- [ ] Prepare callback wrapper with `ffi_prep_closure_loc()`

### 13.2 Callback Wrapper
- [ ] Implement callback dispatch function
- [ ] Receive C arguments
- [ ] Marshal to Brisk values
- [ ] Call Brisk function
- [ ] Marshal return value back to C

### 13.3 @cfunc Syntax
- [ ] Parse `@cfunc` annotation
- [ ] Create callable C function pointer
- [ ] Register for use with C APIs

---

## Phase 14: @c Block Implementation

### 14.1 TCC Integration (Option A - Recommended)
- [ ] Install libtcc-dev: `sudo apt install tcc libtcc-dev`
- [ ] Add `-ltcc` to Makefile LDFLAGS
- [ ] Create `include/cinline.h`
- [ ] Create `src/cinline.c`
- [ ] Include `<libtcc.h>`
- [ ] Implement `compile_c_block(const char* code)` function
- [ ] Use `tcc_new()`, `tcc_set_output_type(TCC_OUTPUT_MEMORY)`
- [ ] Use `tcc_compile_string()`, `tcc_relocate()`
- [ ] Get symbol addresses with `tcc_get_symbol()`

### 14.2 Pre-compilation (Option B)
- [ ] Write C code to temp file `/tmp/brisk_inline_XXXX.c`
- [ ] Invoke `gcc -shared -fPIC -o /tmp/brisk_inline_XXXX.so`
- [ ] Load with `dlopen()`
- [ ] Clean up temp files

### 14.3 C Block Execution
- [ ] Implement `exec_c_block()` function
- [ ] Compile C code
- [ ] Extract function declarations
- [ ] Register as callable functions

---

## Phase 15: Error Handling

### 15.1 Error Type
- [ ] Create `BriskError` struct
- [ ] Add `char* message` field
- [ ] Add `int line` field
- [ ] Add `int column` field
- [ ] Add `char* file` field

### 15.2 Runtime Errors
- [ ] Implement `runtime_error(const char* fmt, ...)` function
- [ ] Print error with location
- [ ] Optionally print stack trace
- [ ] Set error state

### 15.3 Error Recovery
- [ ] Allow REPL to continue after errors
- [ ] Reset interpreter state on error

### 15.4 Try/Catch (Optional)
- [ ] Add `try`/`catch` syntax
- [ ] Implement exception unwinding
- [ ] Or use Result type approach

---

## Phase 16: Module System

### 16.1 Module Loading
- [ ] Implement `load_module(const char* path)` function
- [ ] Read source file
- [ ] Parse and execute
- [ ] Return module's exports

### 16.2 Import Syntax for Brisk Modules
- [ ] Handle `import "path/to/module.brisk"`
- [ ] Execute module in isolated environment
- [ ] Bind module to name

### 16.3 Export Mechanism
- [ ] Define what gets exported (all top-level, or explicit)
- [ ] Handle re-exports

### 16.4 Module Caching
- [ ] Cache loaded modules
- [ ] Don't re-execute on second import

---

## Phase 17: Polish and Testing

### 17.1 Comprehensive Error Messages
- [ ] Review all error messages
- [ ] Add context to errors
- [ ] Suggest fixes where possible

### 17.2 Line Number Tracking
- [ ] Ensure all nodes have line info
- [ ] Print line numbers in errors
- [ ] Print source line in errors

### 17.3 Test Suite
- [ ] Create test runner script
- [ ] Test all operators
- [ ] Test all statements
- [ ] Test C interop
- [ ] Test edge cases
- [ ] Test error cases

### 17.4 Example Programs
- [ ] Create `examples/hello.brisk`
- [ ] Create `examples/fibonacci.brisk`
- [ ] Create `examples/sdl_window.brisk`
- [ ] Create `examples/raylib_game.brisk`
- [ ] Create `examples/file_io.brisk`

### 17.5 Documentation
- [ ] Write README.md
- [ ] Document syntax
- [ ] Document built-in functions
- [ ] Document C interop
- [ ] Add examples in docs

---

## Phase 18: Bytecode VM (Optional Enhancement)

### 18.1 Bytecode Definition
- [ ] Create `include/bytecode.h`
- [ ] Define `OpCode` enum
- [ ] Define bytecode format

### 18.2 Compiler
- [ ] Create `src/compiler.c`
- [ ] Compile AST to bytecode
- [ ] Handle all expression types
- [ ] Handle all statement types

### 18.3 Virtual Machine
- [ ] Create `src/vm.c`
- [ ] Implement stack-based VM
- [ ] Implement bytecode dispatch loop
- [ ] Handle all opcodes

### 18.4 Integration
- [ ] Replace tree-walking with VM
- [ ] Maintain same semantics

---

## Quick Reference: File Checklist

### Headers (include/)
- [ ] `token.h`
- [ ] `lexer.h`
- [ ] `ast.h`
- [ ] `parser.h`
- [ ] `value.h`
- [ ] `memory.h`
- [ ] `env.h`
- [ ] `interp.h`
- [ ] `builtins.h`
- [ ] `cffi.h`
- [ ] `dynload.h`
- [ ] `cheader.h`

### Source Files (src/)
- [ ] `main.c`
- [ ] `lexer.c`
- [ ] `ast.c`
- [ ] `parser.c`
- [ ] `value.c`
- [ ] `memory.c`
- [ ] `env.c`
- [ ] `interp.c`
- [ ] `builtins.c`
- [ ] `repl.c`
- [ ] `cffi.c`
- [ ] `dynload.c`
- [ ] `cheader.c`

---

## Linux Dependencies Summary

```bash
# Essential
sudo apt update
sudo apt install build-essential gcc make

# libffi for FFI calls
sudo apt install libffi-dev

# Optional: TCC for @c blocks
sudo apt install tcc libtcc-dev

# Optional: readline for nicer REPL
sudo apt install libreadline-dev

# For testing with SDL2
sudo apt install libsdl2-dev

# For testing with Raylib
sudo apt install libraylib-dev
# Or build from source: https://github.com/raysan5/raylib
```

---

## Estimated Lines of Code

| Component | Estimated LOC |
|-----------|---------------|
| Lexer | ~400 |
| AST | ~300 |
| Parser | ~1200 |
| Values | ~400 |
| Memory | ~200 |
| Environment | ~150 |
| Interpreter | ~1500 |
| Built-ins | ~500 |
| REPL | ~200 |
| FFI Core | ~600 |
| Dynamic Loading | ~100 |
| Header Parsing | ~1000 |
| C Struct Support | ~400 |
| **Total** | **~7000** |

---

## Next Action

Start with **Phase 0: Project Setup** → then **Phase 1: Lexer**.

Each checkbox is one focused task. Complete them in order, testing as you go.
