#pragma once
#include <string_view>
#include <cstdint>
#include "../common/source_location.hpp"

namespace compiler {

enum class TokenType : uint8_t {
    // End of file
    EndOfFile,

    // Literals
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    BoolLiteral,

    // Single-character tokens
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    Percent,        // %
    Bang,           // !
    Equal,          // =
    Less,           // <
    Greater,        // >
    Ampersand,      // &
    Pipe,           // |
    Caret,          // ^
    Tilde,          // ~

    // Two-character tokens
    PlusEqual,      // +=
    MinusEqual,     // -=
    StarEqual,      // *=
    SlashEqual,     // /=
    PercentEqual,   // %=
    EqualEqual,     // ==
    BangEqual,      // !=
    LessEqual,      // <=
    GreaterEqual,   // >=
    AmpersandAmpersand, // &&
    PipePipe,       // ||
    PlusPlus,       // ++
    MinusMinus,     // --
    Arrow,          // ->

    // Delimiters
    LParen,         // (
    RParen,         // )
    LBrace,         // {
    RBrace,         // }
    LBracket,       // [
    RBracket,       // ]
    Semicolon,      // ;
    Comma,          // ,
    Dot,            // .
    Colon,          // :
    Question,       // ?

    // Keywords
    KwAuto,
    KwBool,
    KwTrue,      // <-- ADD THIS
    KwFalse,     // <-- ADD THIS
    KwBreak,
    KwCase,
    KwChar,
    KwClass,
    KwConst,
    KwContinue,
    KwDefault,
    KwDo,
    KwDouble,
    KwElse,
    KwEnum,
    KwExtern,
    KwFloat,
    KwFor,
    KwGoto,
    KwIf,
    KwInline,
    KwInt,
    KwLong,
    KwRegister,
    KwReturn,
    KwShort,
    KwSigned,
    KwSizeof,
    KwStatic,
    KwStruct,
    KwSwitch,
    KwTypedef,
    KwUnion,
    KwUnsigned,
    KwVoid,
    KwVolatile,
    KwWhile,

    // Access specifiers
    KwPublic,
    KwPrivate,
    KwProtected,

    // Preprocessor directives
    Hash,           // #
    HashInclude,    // #include
    HashDefine,     // #define
    HashIf,         // #if
    HashIfdef,      // #ifdef
    HashIfndef,     // #ifndef
    HashElse,       // #else
    HashElif,       // #elif
    HashEndif,      // #endif
    HashPragma,     // #pragma
};

struct Token {
    TokenType type;
    std::string_view value;
    SourceLocation location;

    [[nodiscard]] bool is_keyword() const noexcept;
    [[nodiscard]] bool is_literal() const noexcept;
    [[nodiscard]] bool is_type() const noexcept;
    [[nodiscard]] bool is_user_type() const noexcept;
    [[nodiscard]] bool is_arithmetic_op() const noexcept;
    [[nodiscard]] bool is_comparison_op() const noexcept;
    [[nodiscard]] bool is_assignment_op() const noexcept;

    [[nodiscard]] std::string_view type_name() const noexcept;
};

} // namespace compiler
