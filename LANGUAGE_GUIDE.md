# Brisk Language Guide

A comprehensive guide to the Brisk programming language.

## Table of Contents

1. [Basics](#basics)
2. [Types](#types)
3. [Variables](#variables)
4. [Operators](#operators)
5. [Control Flow](#control-flow)
6. [Functions](#functions)
7. [Collections](#collections)
8. [Pattern Matching](#pattern-matching)
9. [Modules](#modules)
10. [C Interoperability](#c-interoperability)
11. [Built-in Functions](#built-in-functions)
12. [REPL](#repl)

---

## Basics

### Comments

```brisk
# This is a single-line comment
# Brisk only has single-line comments
```

### Statements

Statements are separated by newlines. Semicolons are optional:

```brisk
x := 10
y := 20
println(x + y)
```

### Expressions

Brisk is expression-oriented. Most constructs return values:

```brisk
# If is an expression
result := if x > 0 { "positive" } else { "non-positive" }

# Match is an expression
name := match code {
    200 => "OK",
    404 => "Not Found",
    _ => "Unknown"
}

# Blocks return their last expression
value := {
    a := 10
    b := 20
    a + b  # This is returned
}
```

---

## Types

Brisk is dynamically typed. Types are checked at runtime.

### Primitive Types

| Type | Description | Examples |
|------|-------------|----------|
| `nil` | Absence of value | `nil` |
| `bool` | Boolean | `true`, `false` |
| `int` | 64-bit integer | `42`, `-17`, `0xFF`, `1_000_000` |
| `float` | 64-bit floating point | `3.14`, `-0.5`, `1e10` |
| `string` | UTF-8 text | `"hello"`, `"line\nbreak"` |

### Compound Types

| Type | Description | Examples |
|------|-------------|----------|
| `array` | Ordered collection | `[1, 2, 3]`, `[]` |
| `table` | Key-value map | `{a: 1, b: 2}`, `{}` |
| `function` | Callable | `fn(x) { x * 2 }` |

### Integer Literals

```brisk
decimal := 42
hex := 0xFF          # 255
with_sep := 1_000_000  # Underscores for readability
negative := -17
```

### Float Literals

```brisk
pi := 3.14159
scientific := 1.5e10
negative := -0.001
```

### String Literals

```brisk
simple := "Hello, World!"
with_escapes := "Line 1\nLine 2\tTabbed"
with_quotes := "She said \"Hi!\""
```

Escape sequences:
- `\n` - Newline
- `\t` - Tab
- `\r` - Carriage return
- `\\` - Backslash
- `\"` - Double quote

### Type Checking

```brisk
println(type(42))        # "int"
println(type(3.14))      # "float"
println(type("hello"))   # "string"
println(type([1,2,3]))   # "array"
println(type({a: 1}))    # "table"
println(type(fn(){}))    # "function"
println(type(nil))       # "nil"
```

### Type Conversion

```brisk
# To integer
int("42")      # 42
int(3.7)       # 3
int(true)      # 1

# To float
float("3.14")  # 3.14
float(42)      # 42.0

# To string
str(42)        # "42"
str(3.14)      # "3.14"
str([1,2,3])   # "[1, 2, 3]"

# To boolean
bool(0)        # false
bool("")       # false
bool(nil)      # false
bool(42)       # true
bool("hi")     # true
```

---

## Variables

### Mutable Variables

Use `:=` for declaration with initialization:

```brisk
x := 42
name := "Alice"
items := [1, 2, 3]
```

Use `=` for reassignment:

```brisk
x := 10
x = 20      # OK
x = "hello" # OK - dynamic typing
```

### Constants

Use `::` for constants (cannot be reassigned):

```brisk
PI :: 3.14159
MAX_SIZE :: 100

PI = 3.0  # Error: Cannot assign to constant 'PI'
```

### Scope

Variables are block-scoped:

```brisk
x := 10

if true {
    y := 20     # Only visible in this block
    x = 15      # Can access outer x
}

println(x)      # 15
println(y)      # Error: Undefined variable 'y'
```

---

## Operators

### Arithmetic

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3` → `8` |
| `-` | Subtraction | `5 - 3` → `2` |
| `*` | Multiplication | `5 * 3` → `15` |
| `/` | Division | `7 / 2` → `3` (int) |
| `%` | Modulo | `7 % 3` → `1` |

### Comparison

| Operator | Description | Example |
|----------|-------------|---------|
| `==` | Equal | `5 == 5` → `true` |
| `!=` | Not equal | `5 != 3` → `true` |
| `<` | Less than | `3 < 5` → `true` |
| `>` | Greater than | `5 > 3` → `true` |
| `<=` | Less or equal | `3 <= 3` → `true` |
| `>=` | Greater or equal | `5 >= 5` → `true` |

### Logical

| Operator | Description | Example |
|----------|-------------|---------|
| `and` | Logical AND | `true and false` → `false` |
| `or` | Logical OR | `true or false` → `true` |
| `!` / `not` | Logical NOT | `!true` → `false` |

### String

```brisk
# Concatenation
"Hello, " + "World!"  # "Hello, World!"

# Comparison
"abc" == "abc"  # true
"abc" < "abd"   # true (lexicographic)
```

### Ranges

```brisk
# Exclusive range (end not included)
0..5      # 0, 1, 2, 3, 4

# Used in for loops
for i in 0..5 {
    println(i)  # Prints 0, 1, 2, 3, 4
}
```

---

## Control Flow

### If/Elif/Else

```brisk
if x > 0 {
    println("positive")
} elif x < 0 {
    println("negative")
} else {
    println("zero")
}
```

As an expression:

```brisk
sign := if x > 0 { 1 } elif x < 0 { -1 } else { 0 }
```

### While Loop

```brisk
i := 0
while i < 5 {
    println(i)
    i = i + 1
}
```

### For Loop

With ranges:

```brisk
# 0 to 9
for i in 0..10 {
    println(i)
}

# Countdown (when start > end)
for i in 10..0 {
    println(i)  # 10, 9, 8, ..., 1
}
```

With arrays:

```brisk
fruits := ["apple", "banana", "cherry"]
for fruit in fruits {
    println(fruit)
}
```

### Break and Continue

```brisk
for i in 0..100 {
    if i == 5 {
        continue  # Skip to next iteration
    }
    if i == 10 {
        break     # Exit loop
    }
    println(i)
}
```

### Return

```brisk
fn find_first_even(arr) {
    for x in arr {
        if x % 2 == 0 {
            return x  # Early return
        }
    }
    return nil
}
```

---

## Functions

### Declaration

```brisk
fn greet(name) {
    println("Hello, " + name + "!")
}

fn add(a, b) {
    return a + b
}
```

### Implicit Return

The last expression in a function is implicitly returned:

```brisk
fn square(x) {
    x * x  # Implicitly returned
}

fn max(a, b) {
    if a > b { a } else { b }  # Expression returned
}
```

### Anonymous Functions (Lambdas)

```brisk
double := fn(x) { x * 2 }
println(double(5))  # 10

# Used with higher-order functions
numbers := [1, 2, 3, 4, 5]
squared := map(numbers, fn(x) { x * x })
```

### Higher-Order Functions

Functions can take functions as arguments:

```brisk
fn apply_twice(f, x) {
    f(f(x))
}

result := apply_twice(fn(x) { x + 1 }, 5)  # 7
```

### Closures

Functions capture their environment:

```brisk
fn make_counter() {
    count := 0
    fn() {
        count = count + 1
        count
    }
}

counter := make_counter()
println(counter())  # 1
println(counter())  # 2
println(counter())  # 3
```

### Recursion

```brisk
fn factorial(n) {
    if n <= 1 { return 1 }
    n * factorial(n - 1)
}

fn fibonacci(n) {
    if n <= 1 { return n }
    fibonacci(n - 1) + fibonacci(n - 2)
}
```

---

## Collections

### Arrays

Creating arrays:

```brisk
empty := []
numbers := [1, 2, 3, 4, 5]
mixed := [1, "two", 3.0, true]  # Mixed types OK
nested := [[1, 2], [3, 4]]
```

Accessing elements:

```brisk
arr := [10, 20, 30, 40]
println(arr[0])   # 10 (first)
println(arr[-1])  # 40 (last)
println(arr[1])   # 20
```

Modifying:

```brisk
arr := [1, 2, 3]
arr[0] = 100      # [100, 2, 3]
push(arr, 4)      # [100, 2, 3, 4]
pop(arr)          # Returns 4, arr is [100, 2, 3]
```

Array functions:

```brisk
arr := [1, 2, 3, 4, 5]

len(arr)              # 5
first(arr)            # 1
last(arr)             # 5
push(arr, 6)          # Append 6
pop(arr)              # Remove and return last
insert(arr, 0, 0)     # Insert 0 at index 0
remove(arr, 0)        # Remove at index 0
```

### Tables (Maps)

Creating tables:

```brisk
empty := {}
person := {
    name: "Alice",
    age: 30,
    active: true
}
```

Accessing fields:

```brisk
# Dot notation (for identifier keys)
println(person.name)   # "Alice"

# Bracket notation (for any key)
println(person["age"]) # 30

key := "name"
println(person[key])   # "Alice"
```

Modifying:

```brisk
person := {name: "Alice"}
person.age = 30           # Add field
person["email"] = "a@b.c" # Add with brackets
person.name = "Bob"       # Modify
```

Table functions:

```brisk
t := {a: 1, b: 2, c: 3}

keys(t)       # ["a", "b", "c"]
values(t)     # [1, 2, 3]
has(t, "a")   # true
has(t, "x")   # false
```

### Iteration

```brisk
# Iterate array
for item in [1, 2, 3] {
    println(item)
}

# Iterate table keys
for key in keys(person) {
    println(key + ": " + str(person[key]))
}
```

---

## Pattern Matching

### Basic Match

```brisk
fn describe(n) {
    match n {
        0 => "zero",
        1 => "one",
        2 => "two",
        _ => "many"
    }
}
```

### Range Patterns

```brisk
fn grade(score) {
    match score {
        90..101 => "A",
        80..90 => "B",
        70..80 => "C",
        60..70 => "D",
        _ => "F"
    }
}
```

### Match as Expression

```brisk
message := match status {
    200 => "OK",
    404 => "Not Found",
    500 => "Server Error",
    _ => "Unknown"
}
```

### Multiple Statements in Arm

```brisk
match command {
    "quit" => {
        println("Goodbye!")
        exit(0)
    },
    "help" => {
        println("Commands: quit, help")
    },
    _ => {
        println("Unknown command")
    }
}
```

---

## Modules

### Importing Brisk Files

```brisk
# Import another Brisk file
@import "lib/math_utils.brisk"

# Functions from the module are now available
result := square(5)
```

### Module File Example

```brisk
# lib/math_utils.brisk

fn square(x) {
    x * x
}

fn cube(x) {
    x * x * x
}

fn clamp(value, min_val, max_val) {
    if value < min_val { return min_val }
    if value > max_val { return max_val }
    value
}
```

### Import Paths

- Relative paths: `@import "lib/utils.brisk"`
- Paths are relative to the current script's location

---

## C Interoperability

### Importing C Headers

```brisk
@import "stdio.h"      # Standard library
@import "math.h"       # Math functions
@import "mylib.h"      # Custom library
```

### How Library Discovery Works

When you import a header, Brisk automatically looks for the library:

1. **Same directory**: `path/to/header.h` → `path/to/libheader.so`
2. **System library**: Falls back to system library path

Example structure:
```
project/
├── libs/
│   ├── raylib.h       # Header
│   └── libraylib.so   # Library (auto-discovered)
└── game.brisk
```

```brisk
# game.brisk
@import "libs/raylib.h"
InitWindow(800, 600, "Game")  # Just works!
```

### Type Mapping

| Brisk Type | C Type |
|------------|--------|
| `int` | `int`, `long`, `int64_t` |
| `float` | `float`, `double` |
| `string` | `char*`, `const char*` |
| `bool` | `bool`, `int` (0/1) |
| `nil` | `NULL`, `void` |

### Calling C Functions

```brisk
@import "math.h"

# C functions work like native Brisk functions
result := sqrt(16.0)    # 4.0
angle := sin(3.14159)   # ~0.0
power := pow(2.0, 10.0) # 1024.0
```

### Working with Colors (Raylib Example)

Raylib uses a `Color` struct `{r, g, b, a}`. On little-endian systems, pass colors as 32-bit integers in ABGR format:

```brisk
# ABGR format: 0xAABBGGRR
WHITE :: 0xFFFFFFFF
RED :: 0xFF0000FF
GREEN :: 0xFF00FF00
BLUE :: 0xFFFF0000
DARK :: 0xFF1D1D1D

ClearBackground(DARK)
DrawText("Hello!", 100, 100, 20, WHITE)
```

### Complete Raylib Example

```brisk
@import "libs/raylib.h"

# Colors (ABGR format)
DARK :: 0xFF1D1D1D
WHITE :: 0xFFFFFFFF
RED :: 0xFF0000FF

# Initialize
InitWindow(800, 600, "Brisk Game")
SetTargetFPS(60)

# Game state
x := 400
y := 300
speed := 5

# Game loop
while !WindowShouldClose() {
    # Input
    if IsKeyDown(262) { x = x + speed }  # RIGHT
    if IsKeyDown(263) { x = x - speed }  # LEFT
    if IsKeyDown(264) { y = y + speed }  # DOWN
    if IsKeyDown(265) { y = y - speed }  # UP
    
    # Draw
    BeginDrawing()
    ClearBackground(DARK)
    DrawCircle(x, y, 30, RED)
    DrawText("Arrow keys to move", 10, 10, 20, WHITE)
    EndDrawing()
}

CloseWindow()
```

---

## Built-in Functions

### I/O Functions

```brisk
print("no newline")
println("with newline")
println("multiple", "values", 123)

name := input("Enter name: ")
```

### Type Functions

```brisk
type(value)    # Returns type name as string
int(value)     # Convert to integer
float(value)   # Convert to float
str(value)     # Convert to string
bool(value)    # Convert to boolean
```

### Array Functions

```brisk
len(arr)                  # Length
push(arr, value)          # Append to end
pop(arr)                  # Remove and return last
first(arr)                # First element
last(arr)                 # Last element
insert(arr, index, value) # Insert at index
remove(arr, index)        # Remove at index
```

### String Functions

```brisk
len(s)                    # Length
substr(s, start, length)  # Substring
find(s, substring)        # Find index (-1 if not found)
replace(s, old, new)      # Replace all occurrences
split(s, delimiter)       # Split into array
join(arr, delimiter)      # Join array into string
upper(s)                  # To uppercase
lower(s)                  # To lowercase
trim(s)                   # Remove whitespace
```

### Math Functions

```brisk
abs(n)           # Absolute value
min(a, b, ...)   # Minimum
max(a, b, ...)   # Maximum
floor(n)         # Round down
ceil(n)          # Round up
round(n)         # Round to nearest
sqrt(n)          # Square root
pow(base, exp)   # Power
sin(n), cos(n), tan(n)  # Trigonometry
```

### Table Functions

```brisk
keys(table)      # Array of keys
values(table)    # Array of values
has(table, key)  # Check if key exists
```

### Utility Functions

```brisk
clock()              # Current time in seconds (float)
assert(cond, msg)    # Assert condition is true
error(msg)           # Raise error and exit
exit(code)           # Exit with code (default 0)
```

---

## REPL

Start the REPL:

```bash
./brisk
```

### REPL Commands

| Command | Description |
|---------|-------------|
| `:help` | Show help |
| `:quit` or `:exit` | Exit REPL |
| `:clear` | Clear screen |
| `:mem` | Show memory usage |

### REPL Usage

```
Brisk REPL v0.1.0
Type ':help' for commands, ':quit' to exit

>>> x := 42
42
>>> x * 2
84
>>> fn double(n) { n * 2 }
<function>
>>> double(21)
42
>>> :quit
Goodbye!
```

---

## Tips and Best Practices

### Use Constants for Configuration

```brisk
WIDTH :: 800
HEIGHT :: 600
TITLE :: "My Game"
FPS :: 60
```

### Keep Functions Small

```brisk
# Good: focused functions
fn is_valid_email(email) {
    find(email, "@") != -1
}

fn sanitize_input(s) {
    trim(lower(s))
}
```

### Use Pattern Matching for State

```brisk
fn handle_state(state) {
    match state {
        "idle" => update_idle(),
        "running" => update_running(),
        "paused" => update_paused(),
        _ => error("Unknown state: " + state)
    }
}
```

### Leverage C Libraries

Don't reinvent the wheel - use C libraries:

```brisk
@import "math.h"     # Math functions
@import "string.h"   # String manipulation
@import "time.h"     # Time functions
@import "raylib.h"   # Graphics
@import "sqlite3.h"  # Database
```

---

## Common Patterns

### Game Loop

```brisk
@import "raylib.h"

InitWindow(800, 600, "Game")
SetTargetFPS(60)

while !WindowShouldClose() {
    # Update
    update_game()
    
    # Draw
    BeginDrawing()
    ClearBackground(0xFF000000)
    draw_game()
    EndDrawing()
}

CloseWindow()
```

### Configuration Table

```brisk
config := {
    debug: true,
    max_enemies: 10,
    player_speed: 5.0,
    title: "My Game"
}

if config.debug {
    println("Debug mode enabled")
}
```

### State Machine

```brisk
state := "menu"

while true {
    match state {
        "menu" => {
            state = handle_menu()
        },
        "playing" => {
            state = handle_game()
        },
        "gameover" => {
            state = handle_gameover()
        },
        "quit" => {
            break
        }
    }
}
```

---

## Error Messages

Brisk provides helpful error messages:

```
[line 10] Runtime Error: Undefined variable 'foo'
[line 15] Runtime Error: Cannot call non-function value
[line 20] Runtime Error: Index out of bounds
[line 25, col 5] Error at '=': Expected expression
```

---

## Conclusion

Brisk is designed to be simple yet powerful, with seamless C interoperability as its core strength. Start with the examples, experiment in the REPL, and leverage the vast ecosystem of C libraries!

For more examples, see the `examples/` directory in the repository.
