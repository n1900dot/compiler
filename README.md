# Compiler Frontend

A modern C++ compiler frontend implementation featuring lexical analysis, parsing, semantic analysis, and preprocessing capabilities.

## Features

- **Preprocessor**: Handles source code preprocessing directives
- **Lexer**: Tokenizes source code into meaningful tokens
- **Parser**: Builds Abstract Syntax Tree (AST) from tokens
- **Semantic Analyzer**: Performs semantic analysis and symbol table management
- **Error Reporting**: Comprehensive error messages with source locations

## Requirements

- C++23 compatible compiler (GCC 13+, Clang 16+, or MSVC)
- CMake 3.25 or higher
- Catch2 (optional, for testing)

## Building

### Basic Build

```bash
mkdir build
cd build
cmake ..
make
```

### Build with Tests

Ensure Catch2 is installed on your system:

```bash
# Ubuntu/Debian
sudo apt install catch2

# Then build
mkdir build
cd build
cmake ..
make
```

### Build without Tests

```bash
mkdir build
cd build
cmake .. -DBUILD_TESTS=OFF
make
```

## Usage

After building, compile source files using:

```bash
./compiler <source_file>
```

Example:
```bash
./compiler example.cpp
```

## Project Structure

```
.
├── CMakeLists.txt          # Build configuration
├── include/
│   └── compiler/           # Header files
├── src/
│   ├── main.cpp            # Entry point
│   ├── common/             # Shared utilities (error handling, source locations)
│   ├── lexer/              # Lexical analysis (tokenizer)
│   ├── parser/             # Parsing and AST generation
│   ├── semantic/           # Semantic analysis and symbol tables
│   └── preprocessor/       # Preprocessor directives
└── tests/
    ├── test_lexer.cpp      # Lexer unit tests
    ├── test_parser.cpp     # Parser unit tests
    └── test_semantic.cpp   # Semantic analyzer unit tests
```

## Running Tests

If built with tests enabled:

```bash
cd build
ctest
# or run directly
./compiler_tests
```

## Compilation Pipeline

1. **Preprocessing**: Process directives and prepare source code
2. **Lexing**: Convert source code into tokens
3. **Parsing**: Build AST from token stream
4. **Semantic Analysis**: Validate semantics and build symbol tables

## License

This project is provided as-is for educational and development purposes.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.
