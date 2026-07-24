#pragma once
#include <string_view>
#include <cctype>

namespace compiler::utils {

[[nodiscard]] constexpr bool is_alpha(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

[[nodiscard]] constexpr bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
}

[[nodiscard]] constexpr bool is_alphanumeric(char c) noexcept {
    return is_alpha(c) || is_digit(c);
}

[[nodiscard]] constexpr bool is_whitespace(char c) noexcept {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

} // namespace compiler::utils
