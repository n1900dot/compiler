#pragma once
#include <expected>
#include <string_view>
#include "symbol_table.hpp"
#include "../parser/ast.hpp"
#include "../common/error.hpp"

namespace compiler {

class SemanticAnalyzer {
public:
    [[nodiscard]] Result<void> analyze(const Program& program);

private:
    SymbolTable symbols_;
    std::string_view current_function_return_type_;
    bool has_return_ = false;
    bool in_loop_ = false;

    [[nodiscard]] Result<void> analyze_stmt(const Stmt& stmt);
    [[nodiscard]] Result<void> analyze_expr(const Expr& expr);
    [[nodiscard]] Result<std::string_view> infer_type(const Expr& expr);
    [[nodiscard]] Result<void> analyze_decl(const Stmt& decl);

    [[nodiscard]] Result<void> check_types(
        std::string_view expected, 
        std::string_view actual, 
        const SourceLocation& loc
    );

    [[nodiscard]] static bool are_types_compatible(std::string_view a, std::string_view b);
};

} // namespace compiler
