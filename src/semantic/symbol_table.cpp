#include "symbol_table.hpp"
#include <format>

namespace compiler {

void SymbolTable::enter_scope() {
    scopes_.emplace_back();
}

void SymbolTable::exit_scope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

Result<void> SymbolTable::declare(std::string_view name, SymbolInfo info) {
    if (scopes_.empty()) enter_scope();

    auto& current = scopes_.back();
    if (current.contains(name)) {
        return std::unexpected(CompilerError{
            .code = ErrorCode::RedefinedSymbol,
            .message = std::format("Redeclaration of '{}'", name),
            .location = info.declared_at,
            .notes = {}
        });
    }

    current[name] = info;
    return {};
}

std::optional<SymbolInfo> SymbolTable::lookup(std::string_view name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (auto found = it->find(name); found != it->end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

std::optional<SymbolInfo> SymbolTable::lookup_current_scope(std::string_view name) const {
    if (scopes_.empty()) return std::nullopt;
    auto& current = scopes_.back();
    if (auto found = current.find(name); found != current.end()) {
        return found->second;
    }
    return std::nullopt;
}

bool SymbolTable::is_declared_in_current_scope(std::string_view name) const {
    return lookup_current_scope(name).has_value();
}

size_t SymbolTable::scope_depth() const noexcept {
    return scopes_.size();
}

} // namespace compiler
