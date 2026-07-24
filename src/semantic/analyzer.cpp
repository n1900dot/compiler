#include "analyzer.hpp"
#include <format>

namespace compiler {

Result<void> SemanticAnalyzer::analyze(const Program& program) {
    symbols_.enter_scope(); // Global scope

    // Register basic built-in and STL types
    symbols_.declare("string", SymbolInfo{.type = "string", .is_initialized = true});
    symbols_.declare("vector", SymbolInfo{.type = "vector", .is_initialized = true});
    symbols_.declare("map", SymbolInfo{.type = "map", .is_initialized = true});
    symbols_.declare("stack", SymbolInfo{.type = "stack", .is_initialized = true});

    for (const auto& decl : program.declarations) {
        auto result = analyze_decl(decl);
        if (!result) return std::unexpected(result.error());
    }

    symbols_.exit_scope();
    return {};
}

Result<void> SemanticAnalyzer::analyze_decl(const Stmt& decl) {
    return std::visit([&](const auto& node) -> Result<void> {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, std::unique_ptr<VarDecl>>) {
            auto info = SymbolInfo{
                .type = node->type,
                .declared_at = SourceLocation{},
                .is_initialized = node->initializer.has_value(),
                .is_function = false,
                .array_size = std::nullopt
            };

            if (node->initializer) {
                auto init_type = infer_type(*node->initializer);
                if (!init_type) return std::unexpected(init_type.error());
                auto check = check_types(node->type, *init_type, SourceLocation{});
                if (!check) return std::unexpected(check.error());
            }

            return symbols_.declare(node->name, info);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<StructDecl>>) {
            // Register struct type
            symbols_.declare(node->name, SymbolInfo{.type = "struct:" + std::string(node->name), .is_initialized = true});
            return {};
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<FuncDecl>>) {
            auto info = SymbolInfo{
                .type = node->return_type,
                .declared_at = SourceLocation{},
                .is_initialized = true,
                .is_function = true,
                .array_size = std::nullopt
            };

            auto decl_result = symbols_.declare(node->name, info);
            if (!decl_result) return std::unexpected(decl_result.error());

            if (node->body) {
                symbols_.enter_scope();
                current_function_return_type_ = node->return_type;
                has_return_ = false;

                for (const auto& param : node->parameters) {
                    auto param_info = SymbolInfo{
                        .type = param.type,
                        .declared_at = SourceLocation{},
                        .is_initialized = true,
                        .array_size = std::nullopt
                    };
                    symbols_.declare(param.name, param_info);
                }

                for (const auto& stmt : *node->body) {
                    auto s = analyze_stmt(stmt);
                    if (!s) return std::unexpected(s.error());
                }

                if (node->return_type != "void" && !has_return_) {
                    return std::unexpected(CompilerError{
                        .code = ErrorCode::MissingReturnStatement,
                        .message = std::format("Function '{}' may not return a value", node->name),
                        .location = SourceLocation{},
                        .notes = {}
                    });
                }

                symbols_.exit_scope();
            }

            return {};
        }
        return {};
    }, decl);
}

Result<void> SemanticAnalyzer::analyze_stmt(const Stmt& stmt) {
    return std::visit([&](const auto& node) -> Result<void> {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, std::unique_ptr<BlockStmt>>) {
            symbols_.enter_scope();
            for (const auto& s : node->statements) {
                auto result = analyze_stmt(s);
                if (!result) return std::unexpected(result.error());
            }
            symbols_.exit_scope();
            return {};
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<ExprStmt>>) {
            return analyze_expr(node->expression);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<ReturnStmt>>) {
            has_return_ = true;
            if (node->value) {
                auto val_type = infer_type(*node->value);
                if (!val_type) return std::unexpected(val_type.error());
                return check_types(current_function_return_type_, *val_type, SourceLocation{});
            }
            return {};
        }
        // ... other statements
        return {};
    }, stmt);
}

Result<void> SemanticAnalyzer::analyze_expr(const Expr& expr) {
    auto type = infer_type(expr);
    if (!type) return std::unexpected(type.error());
    return {};
}

Result<std::string_view> SemanticAnalyzer::infer_type(const Expr& expr) {
    return std::visit([&](const auto& node) -> Result<std::string_view> {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, std::unique_ptr<MemberAccessExpr>>) {
            auto obj_type = infer_type(node->object);
            if (!obj_type) return std::unexpected(obj_type.error());
            // Simple member type inference (expand for real struct lookup)
            if (node->member == "x" || node->member == "y" || node->member == "a" || node->member == "c") {
                return "int";
            } else if (node->member == "s") {
                return "string";
            } else if (node->member == "f") {
                return "float";
            } else if (node->member == "d") {
                return "double";
            }
            return "unknown";
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<IdentifierExpr>>) {
            auto info = symbols_.lookup(node->name);
            if (!info) {
                return std::unexpected(CompilerError{
                    .code = ErrorCode::UndefinedSymbol,
                    .message = std::format("Undefined identifier '{}'", node->name),
                    .location = SourceLocation{},
                    .notes = {}
                });
            }
            return info->type;
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<LiteralExpr>>) {
            return std::visit([](auto val) -> Result<std::string_view> {
                using VT = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<VT, int64_t>) return "int";
                else if constexpr (std::is_same_v<VT, double>) return "double";
                else if constexpr (std::is_same_v<VT, std::string_view>) return "string";
                else if constexpr (std::is_same_v<VT, char>) return "char";
                else if constexpr (std::is_same_v<VT, bool>) return "bool";
                return "unknown";
            }, node->value);
        }
        // ... other cases (BinaryExpr, CallExpr, etc.)
        return "unknown";
    }, expr);
}

Result<void> SemanticAnalyzer::check_types(std::string_view expected, std::string_view actual, const SourceLocation& loc) {
    if (!are_types_compatible(expected, actual)) {
        return std::unexpected(CompilerError{
            .code = ErrorCode::TypeMismatch,
            .message = std::format("Expected type '{}', got '{}'", expected, actual),
            .location = loc,
            .notes = {}
        });
    }
    return {};
}

bool SemanticAnalyzer::are_types_compatible(std::string_view a, std::string_view b) {
    if (a == b) return true;
    if ((a == "int" || a == "char") && b == "int") return true; // char to int
    if (a == "double" && (b == "int" || b == "float")) return true;
    return false;
}

} // namespace compiler
