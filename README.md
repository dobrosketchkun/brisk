# Brisk

A minimal interpreted language with native C interoperability.

## Features

- **Clean Syntax**: Python/Rust-inspired syntax with minimal boilerplate
- **Dynamic Typing**: Simple, flexible type system
- **First-class Functions**: Closures, lambdas, and higher-order functions
- **Native C Interop**: Call C functions directly via `@import`
- **Pattern Matching**: Simple but powerful `match` expressions
- **REPL**: Interactive development and exploration

## Quick Start

### Building

```bash
# Install dependencies (Ubuntu/Debian)
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

### Variables and Constants

```brisk
# Mutable variable
x := 42
x = x + 1

# Constant (cannot be reassigned)
PI :: 3.14159
```

### Functions

```brisk
fn add(a, b) {
    a + b  # Implicit return
}

fn factorial(n) {
    if n <= 1 { return 1 }
    n * factorial(n - 1)
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
while condition {
    # ...
}

# For loop (ranges)
for i in 0..10 {
    println(i)
}

# For loop (arrays)
for item in [1, 2, 3] {
    println(item)
}
```

### Collections

```brisk
# Arrays
arr := [1, 2, 3, 4, 5]
push(arr, 6)
println(arr[0])  # 1
println(len(arr))  # 6

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
        _ => "other"
    }
}
```

### C Interop

```brisk
# Import C headers directly
@import "stdio.h"
@import "math.h"

# Call C functions
printf("Hello from C!\n")
println("sqrt(16) =", sqrt(16.0))
```

## Built-in Functions

### I/O
- `print(...)` - Print values
- `println(...)` - Print values with newline
- `input([prompt])` - Read line from stdin

### Type Conversion
- `type(value)` - Get type name
- `int(value)` - Convert to integer
- `float(value)` - Convert to float
- `str(value)` - Convert to string
- `bool(value)` - Convert to boolean

### Array Functions
- `len(array)` - Get length
- `push(array, value)` - Append value
- `pop(array)` - Remove and return last value
- `insert(array, index, value)` - Insert at index
- `remove(array, index)` - Remove at index

### String Functions
- `len(string)` - Get length
- `substr(string, start, [len])` - Substring
- `find(string, substring)` - Find index
- `replace(string, old, new)` - Replace all occurrences
- `split(string, delimiter)` - Split into array
- `join(array, delimiter)` - Join array into string
- `upper(string)` - Convert to uppercase
- `lower(string)` - Convert to lowercase
- `trim(string)` - Remove leading/trailing whitespace

### Math Functions
- `abs(n)` - Absolute value
- `min(...)` - Minimum value
- `max(...)` - Maximum value
- `floor(n)` - Floor
- `ceil(n)` - Ceiling
- `round(n)` - Round
- `sqrt(n)` - Square root
- `pow(base, exp)` - Power
- `sin(n)`, `cos(n)`, `tan(n)` - Trigonometry

### Table Functions
- `keys(table)` - Get array of keys
- `values(table)` - Get array of values
- `has(table, key)` - Check if key exists

### Utility
- `assert(condition, [message])` - Assert condition
- `error(message)` - Raise error and exit
- `clock()` - Current time in seconds
- `exit([code])` - Exit program

## REPL Commands

- `:help` - Show help
- `:quit` or `:exit` - Exit REPL
- `:clear` - Clear screen
- `:mem` - Show memory usage

## Project Structure

```
brisk/
├── src/           # Source files
├── include/       # Header files
├── examples/      # Example programs
├── tests/         # Test files
├── lib/           # Standard library
└── Makefile
```

## Examples

See the `examples/` directory for more examples:

- `hello.brisk` - Hello World
- `fibonacci.brisk` - Fibonacci sequence
- `features.brisk` - Language feature showcase
- `c_interop.brisk` - C library interop demo

## Building from Source

### Requirements

- GCC or Clang
- Make
- libffi-dev (for C interop)

### Build Commands

```bash
make all      # Build optimized
make debug    # Build with debug symbols
make release  # Build optimized release
make clean    # Clean build artifacts
make test     # Run all tests
make examples # Run example programs
make repl     # Start REPL
```

## License

MIT License
