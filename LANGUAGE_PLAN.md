# Brisk - A Minimal Interpreted Language with Native C Interop

## Philosophy

**Brisk** is a minimal, dynamically-typed interpreted language designed for one killer feature: **seamless, zero-friction C library interoperability**. Instead of being "yet another scripting language," Brisk treats C as a first-class citizen, allowing you to use any C library (SDL, Raylib, OpenGL, system APIs) without writing bindings or wrappers.

### Core Principles

1. **C is not foreign** - C functions feel native, not wrapped
2. **Minimal syntax, maximum utility** - Only features that earn their place
3. **No hidden magic** - Behavior is predictable and debuggable
4. **Practical over pure** - Pragmatic choices over academic elegance

---

## Language Overview

### Syntax Style

Brisk uses a clean, expression-oriented syntax inspired by Lua, Rust, and Go:

```brisk
# Comments start with hash

# Variables (dynamically typed, mutable by default)
x := 42
name := "hello"
pi :: 3.14159  # '::' for constants

# Functions
fn add(a, b) {
    a + b  # implicit return of last expression
}

# Explicit return
fn factorial(n) {
    if n <= 1 { return 1 }
    n * factorial(n - 1)
}

# Anonymous functions
double := fn(x) { x * 2 }

# Arrays and tables (like Lua, unified structure)
arr := [1, 2, 3, 4, 5]
map := { name: "brisk", version: 1 }

# Control flow
if condition {
    # ...
} elif other {
    # ...
} else {
    # ...
}

# Loops
for i in 0..10 {
    print(i)
}

for item in collection {
    print(item)
}

while condition {
    # ...
}

# Pattern matching (simple but powerful)
match value {
    0 => print("zero"),
    1..10 => print("small"),
    _ => print("other"),
}
```

---

## The C Interop System

This is the **core innovation** of Brisk. Three mechanisms work together:

### 1. Direct C Header Import

```brisk
# Import C headers directly - Brisk parses them
@import "stdio.h"
@import "SDL2/SDL.h"
@import "raylib.h"

# Functions are now available
printf("Hello from C!\n")
SDL_Init(SDL_INIT_VIDEO)
```

### 2. Inline C Blocks

For performance-critical code or complex C patterns:

```brisk
@c {
    #include <math.h>
    
    double fast_sqrt(double x) {
        return sqrt(x);
    }
}

# Now callable from Brisk
result := fast_sqrt(16.0)
```

### 3. C Type Annotations

When you need precise control over types:

```brisk
# Declare C types for FFI precision
@ctype WindowPtr = "SDL_Window*"
@ctype RendererPtr = "SDL_Renderer*"

# Use in function signatures
fn create_window() -> @WindowPtr {
    SDL_CreateWindow("Game", 100, 100, 800, 600, SDL_WINDOW_SHOWN)
}
```

### How C Interop Works (Implementation)

Brisk uses **libffi** combined with a **C header parser** (using a simplified C grammar or leveraging libclang):

1. **Header Parsing**: When you `@import`, Brisk parses the C header to extract:
   - Function signatures (names, parameter types, return types)
   - Struct definitions
   - Enum values
   - Macros (simple ones)

2. **Dynamic Loading**: Libraries are loaded via `dlopen` (Unix) / `LoadLibrary` (Windows)

3. **FFI Calls**: libffi handles the ABI-correct calling convention

4. **Type Marshalling**: Brisk values are converted to/from C types automatically

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│ Brisk Code  │ --> │ Type Marshal │ --> │  libffi     │
│             │     │              │     │  call       │
└─────────────┘     └──────────────┘     └─────────────┘
                                               │
                                               v
                                         ┌─────────────┐
                                         │ C Function  │
                                         │ (any lib)   │
                                         └─────────────┘
```

---

## Type System

### Brisk Types

| Type | Description | C Equivalent |
|------|-------------|--------------|
| `nil` | Null/nothing | `NULL` |
| `bool` | Boolean | `bool` / `int` |
| `int` | 64-bit integer | `int64_t` |
| `float` | 64-bit float | `double` |
| `str` | Immutable string | `const char*` |
| `array` | Dynamic array | - |
| `table` | Hash map | - |
| `fn` | Function/closure | - |
| `ptr` | Raw C pointer | `void*` |
| `cstruct` | C struct wrapper | `struct T` |

### Automatic Type Conversion

```brisk
# Brisk int -> C int (any size)
# Brisk float -> C float/double
# Brisk str -> C char* (null-terminated)
# Brisk array of int -> C int* (with length)
# Brisk nil -> C NULL
```

### C Struct Access

```brisk
@import "SDL2/SDL.h"

event := SDL_Event{}  # allocate C struct

while SDL_PollEvent(&event) {
    if event.type == SDL_QUIT {
        break
    }
}
```

---

## Memory Management

### Automatic for Brisk Values

Brisk uses **reference counting** with cycle detection for its own values:
- Simple, predictable, deterministic
- No unpredictable GC pauses (important for games/real-time)

### Manual for C Resources

C resources must be explicitly freed (Brisk doesn't track C heap):

```brisk
window := SDL_CreateWindow(...)
# ... use window ...
SDL_DestroyWindow(window)  # manual cleanup
```

### Defer Statement

For RAII-like cleanup patterns:

```brisk
fn load_and_process() {
    file := fopen("data.txt", "r")
    defer fclose(file)  # runs when scope exits
    
    # ... process file ...
    # fclose called automatically
}
```

---

## Standard Library

Minimal but practical:

```
std/
├── io          # print, input, file operations
├── str         # string manipulation
├── math        # math functions (wraps C math.h)
├── os          # system operations, env vars
├── collections # array/table utilities
└── ffi         # low-level FFI utilities
```

Most "standard library" needs are covered by importing C headers directly.

---

## Implementation Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        BRISK RUNTIME                        │
├──────────────┬──────────────┬──────────────┬───────────────┤
│    Lexer     │    Parser    │   Compiler   │  VM / Interp  │
│   (tokens)   │    (AST)     │  (bytecode)  │   (execute)   │
├──────────────┴──────────────┴──────────────┴───────────────┤
│                      VALUE SYSTEM                           │
│   (int, float, str, array, table, fn, ptr, cstruct)        │
├─────────────────────────────────────────────────────────────┤
│                     C INTEROP LAYER                         │
│  ┌───────────────┐  ┌────────────┐  ┌───────────────────┐  │
│  │ Header Parser │  │   libffi   │  │ Dynamic Loader    │  │
│  │ (C decls)     │  │  (calls)   │  │ (dlopen/LoadLib)  │  │
│  └───────────────┘  └────────────┘  └───────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### Component Breakdown

| Component | Purpose | Complexity |
|-----------|---------|------------|
| Lexer | Source → Tokens | Low |
| Parser | Tokens → AST | Medium |
| Bytecode Compiler | AST → Bytecode | Medium |
| Virtual Machine | Execute bytecode | Medium-High |
| C Header Parser | Parse C declarations | Medium |
| FFI Bridge | Type marshalling + calls | High |
| Memory Manager | Refcounting + cycles | Medium |

---

## Development Phases

### Phase 1: Core Language (Foundation)
**Target: ~2000 LOC**

- [ ] Lexer with all token types
- [ ] Recursive descent parser → AST
- [ ] Tree-walking interpreter (before bytecode)
- [ ] Basic types: nil, bool, int, float, str
- [ ] Variables, assignment, expressions
- [ ] Control flow: if/elif/else, while, for
- [ ] Functions (no closures yet)
- [ ] Arrays and tables
- [ ] Basic REPL

### Phase 2: C Interop (The Hard Part)
**Target: ~3000 LOC**

- [ ] Integrate libffi
- [ ] Dynamic library loading (cross-platform)
- [ ] Simple C header parser (function declarations)
- [ ] `@import` directive for headers
- [ ] Type marshalling (Brisk ↔ C)
- [ ] Pointer type and operations
- [ ] C struct support (read/write fields)
- [ ] `@c` inline blocks (via TCC or pre-compilation)

### Phase 3: Refinement
**Target: ~1500 LOC**

- [ ] Closures and upvalues
- [ ] Pattern matching
- [ ] Defer statement
- [ ] Module system
- [ ] Error handling (try/catch or Result type)
- [ ] Bytecode compiler + VM (for speed)

### Phase 4: Polish
**Target: ~1000 LOC**

- [ ] Standard library modules
- [ ] Better error messages with line numbers
- [ ] Simple debugger
- [ ] Performance optimization
- [ ] Documentation

---

## Dependencies

### Required C Libraries

| Library | Purpose | Notes |
|---------|---------|-------|
| **libffi** | Foreign function calls | Core dependency |
| **TCC** (optional) | JIT compile `@c` blocks | Or use pre-compilation |

### Build System

Use a simple Makefile or CMake:

```makefile
CC = gcc
CFLAGS = -Wall -O2 -g
LIBS = -lffi -ldl

brisk: src/*.c
    $(CC) $(CFLAGS) -o brisk src/*.c $(LIBS)
```

---

## File Structure

```
brisk/
├── src/
│   ├── main.c           # Entry point, REPL
│   ├── lexer.c/h        # Tokenization
│   ├── parser.c/h       # Parsing → AST
│   ├── ast.c/h          # AST node definitions
│   ├── interp.c/h       # Tree-walking interpreter
│   ├── value.c/h        # Value types and operations
│   ├── table.c/h        # Hash table implementation
│   ├── memory.c/h       # Allocation, refcounting
│   ├── cffi.c/h         # C interop (libffi wrapper)
│   ├── cheader.c/h      # C header parser
│   └── builtins.c/h     # Built-in functions
├── lib/
│   └── std/             # Standard library (Brisk code)
├── examples/
│   ├── hello.brisk
│   ├── raylib_game.brisk
│   └── sdl_window.brisk
├── tests/
├── Makefile
└── README.md
```

---

## Example Programs

### Hello World

```brisk
print("Hello, Brisk!")
```

### Using C Standard Library

```brisk
@import "stdio.h"
@import "stdlib.h"
@import "time.h"

srand(time(nil))
num := rand() % 100

printf("Random number: %d\n", num)
```

### SDL2 Window

```brisk
@import "SDL2/SDL.h"

SDL_Init(SDL_INIT_VIDEO)

window := SDL_CreateWindow(
    "Brisk + SDL2",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    800, 600,
    SDL_WINDOW_SHOWN
)

running := true
event := SDL_Event{}

while running {
    while SDL_PollEvent(&event) {
        if event.type == SDL_QUIT {
            running = false
        }
    }
    SDL_Delay(16)
}

SDL_DestroyWindow(window)
SDL_Quit()
```

### Raylib Game

```brisk
@import "raylib.h"

InitWindow(800, 600, "Brisk + Raylib")
SetTargetFPS(60)

x := 400.0
y := 300.0
speed := 5.0

while !WindowShouldClose() {
    # Input
    if IsKeyDown(KEY_RIGHT) { x = x + speed }
    if IsKeyDown(KEY_LEFT)  { x = x - speed }
    if IsKeyDown(KEY_DOWN)  { y = y + speed }
    if IsKeyDown(KEY_UP)    { y = y - speed }
    
    # Draw
    BeginDrawing()
    ClearBackground(RAYWHITE)
    DrawCircle(x, y, 20.0, RED)
    DrawText("Move with arrow keys!", 10, 10, 20, DARKGRAY)
    EndDrawing()
}

CloseWindow()
```

---

## Challenges & Solutions

### Challenge 1: Parsing C Headers

C headers are complex. Solutions:

1. **Simplified Parser**: Only parse what's needed (function decls, structs, enums)
2. **libclang**: Use LLVM's C parser (heavy but complete)
3. **Pre-generated Bindings**: Ship with common library bindings

**Recommendation**: Start with simplified parser, add libclang as optional for complex headers.

### Challenge 2: C Macros

Many C APIs use macros (`SDL_WINDOWPOS_CENTERED`). Solutions:

1. **Evaluate simple macros**: Integer constants, simple expressions
2. **Pre-process headers**: Run through C preprocessor first
3. **Manual definition**: Define commonly used macros in Brisk

### Challenge 3: Callback Functions

C APIs like SDL need callbacks. Solution:

```brisk
# Create a C-callable wrapper around Brisk function
callback := @cfunc fn(data: ptr) -> int {
    print("Callback called!")
    0
}

SomeAPI_SetCallback(callback)
```

libffi provides `ffi_closure` for this.

### Challenge 4: Struct Alignment

C structs have platform-specific alignment. Solution:

- Use compiler's alignment info
- Or use libclang to get accurate layout
- Or accept minimal struct support initially

---

## Success Metrics

A successful Brisk implementation should:

1. ✅ Run a Raylib game with input handling and rendering
2. ✅ Create an SDL2 window and handle events
3. ✅ Call arbitrary C functions from shared libraries
4. ✅ Have a responsive REPL for exploration
5. ✅ Be under 10,000 lines of C code total
6. ✅ Compile with standard C99 and minimal dependencies

---

## Next Steps

1. **Set up project structure** - Create src/ with initial files
2. **Implement lexer** - Tokenize Brisk source code
3. **Implement parser** - Build AST from tokens
4. **Tree-walking interpreter** - Execute AST directly
5. **Add libffi integration** - Call first C function
6. **Build C header parser** - Parse simple declarations
7. **Iterate and refine** - Add features as needed

---

## Open Design Questions

1. **Error handling**: Exceptions vs Result type vs simple error codes?
2. **Async/concurrency**: Worth including? Or defer to C threads?
3. **String interpolation**: `$"Hello {name}"` or concatenation only?
4. **Operator overloading**: For custom types?

Let me know your preferences on these, and we can refine the design before implementation.

