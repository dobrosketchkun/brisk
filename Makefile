# Brisk Language - Makefile
# Target Platform: Linux (WSL)

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS = -lm -lffi -ldl

# Source files
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN = brisk

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Default target
all: CFLAGS += -O2
all: $(BIN)

# Debug build
debug: CFLAGS += -g -O0 -DDEBUG
debug: $(BIN)

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: $(BIN)

# Link
$(BIN): $(BUILD_DIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BIN) $(LDFLAGS)

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN) test_lexer test_parser test_interp

# Test lexer
test_lexer: $(BUILD_DIR) $(BUILD_DIR)/lexer.o
	$(CC) $(CFLAGS) -g -O0 -DDEBUG tests/test_lexer.c $(BUILD_DIR)/lexer.o -o test_lexer
	./test_lexer

# Test parser
test_parser: $(BUILD_DIR) $(BUILD_DIR)/lexer.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/parser.o
	$(CC) $(CFLAGS) -g -O0 -DDEBUG tests/test_parser.c $(BUILD_DIR)/lexer.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/parser.o -o test_parser
	./test_parser

# Test interpreter
test_interp: debug
	./$(BIN) tests/test_interp.brisk

# Run all tests
test: test_lexer test_parser test_interp

# Run examples
examples: debug
	@echo "=== Running hello.brisk ==="
	./$(BIN) examples/hello.brisk
	@echo ""
	@echo "=== Running fibonacci.brisk ==="
	./$(BIN) examples/fibonacci.brisk
	@echo ""
	@echo "=== Running features.brisk ==="
	./$(BIN) examples/features.brisk

# Test C interop (requires libffi-dev)
test_ffi: debug
	@echo "=== Running c_interop.brisk ==="
	./$(BIN) examples/c_interop.brisk

# Run REPL
repl: debug
	./$(BIN)

.PHONY: all debug release clean test test_lexer test_parser test_interp examples repl
