#pragma once
#include <compare>
#include <string>

namespace compiler {

struct SourceLocation {
    int line = 1;
    int column = 1;
    int offset = 0;

    auto operator<=>(const SourceLocation&) const = default;
    bool operator==(const SourceLocation&) const = default;

    [[nodiscard]] std::string to_string() const;
};

} // namespace compiler
