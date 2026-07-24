#include "source_location.hpp"
#include <format>

namespace compiler {

std::string SourceLocation::to_string() const {
    return std::format("{}:{}", line, column);
}

} // namespace compiler
