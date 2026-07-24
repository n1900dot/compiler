#pragma once
#include <string>
#include "ast.hpp"

namespace compiler {

class ASTPrinter {
public:
    [[nodiscard]] std::string print(const Program& program) const;
    [[nodiscard]] std::string print_expr(const Expr& expr, int indent = 0) const;
    [[nodiscard]] std::string print_stmt(const Stmt& stmt, int indent = 0) const;

private:
    [[nodiscard]] static std::string indent_str(int indent);
};

} // namespace compiler
