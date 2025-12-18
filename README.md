# Prelude

This is just an experiment about creating a programming language from scratch with AI. The idea is to create a plan with it and give it a go without interfering. Obviously, at a later stage, it should be guided by tests, results, and capslock swearing and such, but otherwise - it's fully AI slop as it is.

And since mister Tsoding will surely never come here, I've already made a raylib speedrun.

---
---

# Brisk

A minimal interpreted language with **seamless C interoperability**.

Brisk's killer feature: import any C library header directly and call its functions without writing bindings.

```brisk
@import "raylib.h"

InitWindow(800, 600, "Hello from Brisk!")
while !WindowShouldClose() {
    BeginDrawing()
    ClearBackground(0xFF1D1D1D)
    DrawText("Brisk + C = ❤️", 280, 280, 40, 0xFFFFFFFF)
    EndDrawing()
}
CloseWindow()
```

## Features

- **Direct C Interop** - `@import` any C header, call functions immediately
- **Clean Syntax** - Python/Rust-inspired, expression-oriented
- **Dynamic Typing** - Simple and flexible
- **First-class Functions** - Closures, lambdas, higher-order functions
- **Pattern Matching** - Elegant `match` expressions
- **Module System** - Import Brisk files with `@import`
- **Interactive REPL** - Explore and prototype

## Quick Start

### Build

```bash
# Dependencies (Ubuntu/Debian/WSL)
sudo apt install build-essential libffi-dev

# Build
make debug

# Run REPL
./brisk

# Run a script
./brisk examples/hello.brisk
```

### Hello World

```brisk
println("Hello, Brisk!")
```

### Variables

```brisk
x := 42          # Mutable variable
x = x + 1        # Reassignment

PI :: 3.14159    # Constant (immutable)
```

### Functions

```brisk
fn greet(name) {
    println("Hello, " + name + "!")
}

fn factorial(n) {
    if n <= 1 { return 1 }
    n * factorial(n - 1)
}

# Implicit return - last expression is returned
fn add(a, b) {
    a + b
}

# Anonymous functions
double := fn(x) { x * 2 }
```

### Control Flow

```brisk
# If/elif/else
if x > 0 {
    println("positive")
} elif x < 0 {
    println("negative")
} else {
    println("zero")
}

# While loop
while running {
    update()
}

# For loop with range
for i in 0..10 {
    println(i)
}

# For loop with array
for item in [1, 2, 3] {
    println(item)
}
```

### Collections

```brisk
# Arrays
arr := [1, 2, 3, 4, 5]
push(arr, 6)
println(arr[0])     # 1
println(len(arr))   # 6

# Tables (maps)
person := {
    name: "Alice",
    age: 30
}
println(person.name)
println(person["age"])
```

### Pattern Matching

```brisk
fn describe(n) {
    match n {
        0 => "zero",
        1..10 => "small",
        _ => "big"
    }
}
```

## C Interoperability

The core innovation of Brisk - use C libraries directly:

```brisk
# Import C headers - functions become available immediately
@import "stdio.h"
@import "math.h"

printf("Hello from C!\n")
println("sqrt(16) =", sqrt(16.0))
println("sin(PI/2) =", sin(1.5708))
```

### Using External Libraries

Place the library (`.so` file) next to its header, and Brisk finds it automatically:

```
myproject/
├── libs/
│   ├── raylib.h
│   └── libraylib.so
└── game.brisk
```

```brisk
# game.brisk
@import "libs/raylib.h"

InitWindow(800, 600, "My Game")
# ... raylib functions just work
```

### Importing Brisk Modules

```brisk
# Import other Brisk files
@import "lib/math_utils.brisk"

result := square(5)  # Functions from the module
```

## Examples

### Raylib Graphics

```brisk
@import "raylib.h"

# Colors in ABGR format (little-endian)
DARK :: 0xFF1D1D1D
WHITE :: 0xFFFFFFFF

InitWindow(800, 600, "Brisk + Raylib")
SetTargetFPS(60)

x := 400
while !WindowShouldClose() {
    if IsKeyDown(262) { x = x + 5 }  # RIGHT
    if IsKeyDown(263) { x = x - 5 }  # LEFT
    
    BeginDrawing()
    ClearBackground(DARK)
    DrawCircle(x, 300, 40, WHITE)
    EndDrawing()
}
CloseWindow()
```

### Math with C

```brisk
@import "math.h"

println("=== Math Functions ===")
println("sqrt(2) =", sqrt(2.0))
println("pow(2, 10) =", pow(2.0, 10.0))
println("sin(0) =", sin(0.0))
println("log(e) =", log(2.718281828))
```

## Built-in Functions

### I/O
- `print(...)` - Print values
- `println(...)` - Print with newline
- `input([prompt])` - Read line from stdin

### Types
- `type(v)` - Get type name
- `int(v)`, `float(v)`, `str(v)`, `bool(v)` - Conversions

### Arrays
- `len(arr)` - Length
- `push(arr, v)` - Append
- `pop(arr)` - Remove last
- `first(arr)`, `last(arr)` - First/last element

### Strings
- `len(s)` - Length
- `substr(s, start, len)` - Substring
- `split(s, delim)` - Split to array
- `join(arr, delim)` - Join array
- `upper(s)`, `lower(s)`, `trim(s)` - Transform

### Math
- `abs(n)`, `min(...)`, `max(...)`
- `floor(n)`, `ceil(n)`, `round(n)`
- `sqrt(n)`, `pow(a, b)`

### Tables
- `keys(t)`, `values(t)` - Get keys/values
- `has(t, key)` - Check key exists

### Utility
- `clock()` - Current time in seconds
- `assert(cond, msg)` - Assert condition
- `exit([code])` - Exit program

## REPL Commands

```
:help   - Show help
:quit   - Exit REPL
:clear  - Clear screen
:mem    - Show memory usage
```

## Project Structure

```
brisk/
├── src/           # C source files
├── include/       # C headers
├── examples/      # Example programs
├── experiments/   # Experiments (raylib demo)
├── lib/           # Brisk standard library
├── tests/         # Test files
└── Makefile
```

## Building

```bash
make all       # Optimized build
make debug     # Debug build (with symbols)
make release   # Release build
make clean     # Clean artifacts
make test      # Run tests
make examples  # Run examples
```

### Requirements

- GCC or Clang (C99)
- libffi-dev
- Make

## How It Works

Brisk uses **libffi** for foreign function calls and a **custom C header parser**:

1. `@import "header.h"` parses the C header
2. Function signatures are extracted
3. Library is loaded from the same directory (`libname.so`)
4. libffi handles calling conventions
5. Types are automatically marshalled between Brisk and C

```
Brisk Code → Parser → AST → Interpreter
                              ↓
                         C Interop Layer
                              ↓
                    libffi + Dynamic Loader
                              ↓
                         C Libraries
```

## License

MIT License

## See Also

- [LANGUAGE_GUIDE.md](LANGUAGE_GUIDE.md) - Comprehensive language documentation
- [LANGUAGE_PLAN.md](LANGUAGE_PLAN.md) - Original design document
- [examples/](examples/) - Example programs
