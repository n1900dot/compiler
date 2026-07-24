#pragma once
#include <expected>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <optional>
#include "../common/error.hpp"
#include "../common/source_location.hpp"

namespace compiler {

struct SymbolInfo {
    std::string_view type;
    SourceLocation declared_at;
    bool is_initialized = false;
    bool is_function = false;
    bool is_constant = false;
    std::optional<int> array_size;
};

class SymbolTable {
public:
    void enter_scope();
    void exit_scope();

    [[nodiscard]] Result<void> declare(std::string_view name, SymbolInfo info);
    [[nodiscard]] std::optional<SymbolInfo> lookup(std::string_view name) const;
    [[nodiscard]] std::optional<SymbolInfo> lookup_current_scope(std::string_view name) const;
    [[nodiscard]] bool is_declared_in_current_scope(std::string_view name) const;
    [[nodiscard]] size_t scope_depth() const noexcept;

private:
    std::vector<std::unordered_map<std::string_view, SymbolInfo>> scopes_;
};

} // namespace compiler
