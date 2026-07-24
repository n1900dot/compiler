#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <expected>
#include "../common/error.hpp"

namespace compiler {

class Preprocessor {
public:
    [[nodiscard]] Result<std::string> process(std::string_view source, std::string_view filename);

    void define_macro(std::string_view name, std::string_view replacement);
    void undef_macro(std::string_view name);

private:
    std::unordered_map<std::string, std::string> defines_;
    std::vector<std::string> include_paths_;
    std::vector<std::string> included_files_;

    [[nodiscard]] Result<std::string> handle_include(std::string_view line, std::string_view parent_file = "", int parent_line = 0);
    [[nodiscard]] Result<std::string> handle_define(std::string_view line);
    [[nodiscard]] std::string expand_macros(std::string_view line);
    [[nodiscard]] bool is_defined(std::string_view name) const;
};

} // namespace compiler
