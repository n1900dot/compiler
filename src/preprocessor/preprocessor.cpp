#include "preprocessor.hpp"
#include <format>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace compiler {

Result<std::string> Preprocessor::process(std::string_view source, [[maybe_unused]] std::string_view filename) {
    std::istringstream stream{std::string(source)};
    std::ostringstream output;
    std::string line;

    while (std::getline(stream, line)) {

        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            output << "\n";
            continue;
        }

        if (line[start] == '#') {
            std::string_view directive(line.c_str() + start);

            if (directive.starts_with("#include")) {
                auto result = handle_include(directive);
                if (!result) return std::unexpected(result.error());
                output << *result << "\n";
            }
            else if (directive.starts_with("#define")) {
                auto result = handle_define(directive);
                if (!result) return std::unexpected(result.error());
            }
            else if (directive.starts_with("#undef")) {
                // TODO: Implement undef
            }
            else if (directive.starts_with("#ifdef") || directive.starts_with("#ifndef")) {
                // TODO: Implement conditional compilation
            }
            else if (directive.starts_with("#pragma")) {
                // Ignore pragmas for now
            }
            else if (directive.starts_with("#if") || directive.starts_with("#elif") ||
                     directive.starts_with("#else") || directive.starts_with("#endif")) {
                // TODO: Implement conditional compilation
            }
        } else {
            output << expand_macros(line) << "\n";
        }
    }

    return output.str();
}

void Preprocessor::define_macro(std::string_view name, std::string_view replacement) {
    defines_[std::string(name)] = std::string(replacement);
}

void Preprocessor::undef_macro(std::string_view name) {
    defines_.erase(std::string(name));
}

Result<std::string> Preprocessor::handle_include(std::string_view line, std::string_view parent_file, int parent_line) {
    size_t start = line.find('"');
    size_t end = line.rfind('"');
    bool is_quoted = true;

    if (start == std::string_view::npos || end == std::string_view::npos || start == end) {
        start = line.find('<');
        end = line.find('>');
        is_quoted = false;
        if (start == std::string_view::npos || end == std::string_view::npos) {
            return std::unexpected(CompilerError{
                .code = ErrorCode::InvalidDirective,
                .message = "Invalid #include syntax",
                .location = SourceLocation{},
                .notes = {}
            });
        }
    }

    std::string filename(line.substr(start + 1, end - start - 1));

    // Skip common standard library includes (mock them)
    if (!is_quoted || filename.find("string") != std::string::npos || 
        filename.find("iostream") != std::string::npos ||
        filename.find("vector") != std::string::npos ||
        filename.find("map") != std::string::npos ||
        filename.find("stack") != std::string::npos) {
        return "#line 1 \"<stdlib>\"\n";  // Mock successful include
    }

    if (std::find(included_files_.begin(), included_files_.end(), filename) != included_files_.end()) {
        return std::unexpected(CompilerError{
            .code = ErrorCode::CircularInclude,
            .message = std::format("Circular include detected: {}", filename),
            .location = SourceLocation{},
            .notes = {}
        });
    }

    std::ifstream file{filename};
    if (!file) {
        return std::unexpected(CompilerError{
            .code = ErrorCode::IncludeNotFound,
            .message = std::format("Could not open include file: {}", filename),
            .location = SourceLocation{},
            .notes = {}
        });
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    included_files_.push_back(filename);
    auto result = process(buffer.str(), filename);
    included_files_.pop_back();

    if (!result) return std::unexpected(result.error());
    return *result + "\n#line " + std::to_string(parent_line + 1) + " \"" + std::string(parent_file) + "\"\n";
}

Result<std::string> Preprocessor::handle_define(std::string_view line) {
    size_t name_start = line.find(' ', 7) + 1;
    if (name_start == std::string_view::npos) {
        return std::unexpected(CompilerError{
            .code = ErrorCode::InvalidDirective,
            .message = "Invalid #define syntax",
            .location = SourceLocation{},
            .notes = {}
        });
    }

    size_t name_end = line.find(' ', name_start);
    std::string_view name = line.substr(name_start, name_end - name_start);
    std::string_view replacement;

    if (name_end != std::string_view::npos) {
        replacement = line.substr(name_end + 1);
    }

    defines_[std::string(name)] = std::string(replacement);
    return "";
}

std::string Preprocessor::expand_macros(std::string_view line) {
    std::string result(line);

    for (const auto& [name, replacement] : defines_) {
        size_t pos = 0;
        while ((pos = result.find(name, pos)) != std::string::npos) {
            bool valid_start = (pos == 0) || !std::isalnum(result[pos - 1]);
            bool valid_end = (pos + name.length() >= result.length()) 
                          || !std::isalnum(result[pos + name.length()]);

            if (valid_start && valid_end) {
                result.replace(pos, name.length(), replacement);
                pos += replacement.length();
            } else {
                pos += name.length();
            }
        }
    }

    return result;
}

bool Preprocessor::is_defined(std::string_view name) const {
    return defines_.contains(std::string(name));
}

} // namespace compiler
