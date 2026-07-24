#pragma once
#include <string_view>
#include <span>
#include <vector>
#include <expected>
#include <string>
#include "token.hpp"
#include "../common/error.hpp"

namespace compiler {

class Lexer {
public:
    explicit Lexer(std::string_view source);

    [[nodiscard]] Result<std::vector<Token>> tokenize();

private:
    std::span<const char> source_;
    size_t current_ = 0;
    SourceLocation loc_{.line = 1, .column = 1, .offset = 0};

    [[nodiscard]] char peek() const noexcept;
    [[nodiscard]] char peek_next() const noexcept;
    char advance() noexcept;
    [[nodiscard]] bool match(char expected) noexcept;
    [[nodiscard]] bool is_at_end() const noexcept;

    [[nodiscard]] Result<Token> identifier_or_keyword();
    [[nodiscard]] Result<Token> number();
    [[nodiscard]] Result<Token> string();
    [[nodiscard]] Result<Token> character();
    [[nodiscard]] Result<Token> preprocessor_directive();

    void skip_whitespace();
    void skip_line_comment();
    void skip_block_comment();

    void advance_location(char c) noexcept;

    [[nodiscard]] Token make_token(TokenType type, std::string_view value) const;
    [[nodiscard]] CompilerError make_error(ErrorCode code, std::string_view message) const;

    [[nodiscard]] static TokenType check_keyword(std::string_view text) noexcept;
};

} // namespace compiler
