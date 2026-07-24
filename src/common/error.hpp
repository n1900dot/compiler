#pragma once
#include <string>
#include <expected>
#include <vector>
#include <cstdint>
#include "source_location.hpp"

namespace compiler {

enum class ErrorCode : uint8_t {
    // Lexer errors
    UnexpectedCharacter,
    UnterminatedString,
    UnterminatedComment,
    InvalidNumberLiteral,

    // Parser errors
    UnexpectedToken,
    ExpectedToken,
    MissingSemicolon,
    MissingClosingParen,
    MissingClosingBrace,
    InvalidExpression,

    // Semantic errors
    UndefinedSymbol,
    RedefinedSymbol,
    TypeMismatch,
    InvalidOperation,
    WrongNumberOfArguments,
    MissingReturnStatement,
    UnreachableCode,

    // Preprocessor errors
    IncludeNotFound,
    CircularInclude,
    InvalidDirective,

    // General
    IOError,
    UnknownError
};

struct CompilerError {
    ErrorCode code;
    std::string message;
    SourceLocation location;
    std::vector<std::string> notes;

    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] std::string code_string() const;
};

template<typename T>
using Result = std::expected<T, CompilerError>;

} // namespace compiler
