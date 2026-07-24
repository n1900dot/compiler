cmake_minimum_required(VERSION 3.25)
project(CompilerFrontend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wconversion -Wsign-conversion
        -Wshadow -Wnon-virtual-dtor
        -Wold-style-cast -Wcast-align
        -Wunused -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )
elseif(MSVC)
    add_compile_options(/W4 /WX)
endif()

# Main executable
add_executable(compiler
    src/main.cpp
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/parser/ast_printer.cpp
    src/semantic/analyzer.cpp
    src/semantic/symbol_table.cpp
    src/preprocessor/preprocessor.cpp
    src/common/error.cpp
    src/common/source_location.cpp
)

target_include_directories(compiler PRIVATE src)

# Testing with Catch2
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    # Try find_package with common paths
    list(APPEND CMAKE_PREFIX_PATH /usr /usr/local /opt)
    find_package(Catch2 3 QUIET)

    if(NOT Catch2_FOUND)
        # Manual fallback: check if headers exist
        if(EXISTS /usr/include/catch2/catch_test_macros.hpp)
            message(STATUS "Catch2 headers found at /usr/include/catch2")
            # Create the targets manually
            if(NOT TARGET Catch2::Catch2)
                add_library(Catch2::Catch2 INTERFACE IMPORTED)
                set_target_properties(Catch2::Catch2 PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES /usr/include/catch2
                )
            endif()
            if(NOT TARGET Catch2::Catch2WithMain)
                add_library(Catch2::Catch2WithMain INTERFACE IMPORTED)
                set_target_properties(Catch2::Catch2WithMain PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES /usr/include/catch2
                    INTERFACE_LINK_LIBRARIES Catch2::Catch2
                )
            endif()
            set(Catch2_FOUND TRUE)
        elseif(EXISTS /usr/local/include/catch2/catch_test_macros.hpp)
            message(STATUS "Catch2 headers found at /usr/local/include/catch2")
            if(NOT TARGET Catch2::Catch2)
                add_library(Catch2::Catch2 INTERFACE IMPORTED)
                set_target_properties(Catch2::Catch2 PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES /usr/local/include/catch2
                )
            endif()
            if(NOT TARGET Catch2::Catch2WithMain)
                add_library(Catch2::Catch2WithMain INTERFACE IMPORTED)
                set_target_properties(Catch2::Catch2WithMain PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES /usr/local/include/catch2
                    INTERFACE_LINK_LIBRARIES Catch2::Catch2
                )
            endif()
            set(Catch2_FOUND TRUE)
        endif()
    endif()

    if(Catch2_FOUND)
        enable_testing()
        add_executable(compiler_tests
            tests/test_lexer.cpp
            tests/test_parser.cpp
            tests/test_semantic.cpp
            src/lexer/lexer.cpp
            src/lexer/token.cpp
            src/parser/parser.cpp
            src/parser/ast_printer.cpp
            src/semantic/analyzer.cpp
            src/semantic/symbol_table.cpp
            src/preprocessor/preprocessor.cpp
            src/common/error.cpp
            src/common/source_location.cpp
        )
        target_include_directories(compiler_tests PRIVATE src)
        target_link_libraries(compiler_tests PRIVATE Catch2::Catch2WithMain)
        add_test(NAME compiler_tests COMMAND compiler_tests)
        message(STATUS "Tests enabled")
    else()
        message(STATUS "Catch2 not found. Install it with:")
        message(STATUS "  Ubuntu/Debian: sudo apt install catch2")
        message(STATUS "  Or build with: cmake .. -DBUILD_TESTS=OFF")
    endif()
endif()
