#include "ast_printer.hpp"
#include <format>
#include <sstream>

namespace compiler {

std::string ASTPrinter::indent_str(int indent) {
    return std::string(static_cast<size_t>(indent * 2), ' ');
}

std::string ASTPrinter::print(const Program& program) const {
    std::ostringstream oss;
    oss << "Program\n";
    for (const auto& decl : program.declarations) {
        oss << print_stmt(decl, 1);
    }
    return oss.str();
}

std::string ASTPrinter::print_expr(const Expr& expr, int indent) const {
    std::ostringstream oss;
    std::string ind = indent_str(indent);

    std::visit([&](const auto& node) {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, std::unique_ptr<BinaryExpr>>) {
            oss << ind << "Binary(" << node->op.type_name() << ")\n";
            oss << print_expr(node->left, indent + 1);
            oss << print_expr(node->right, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<UnaryExpr>>) {
            oss << ind << "Unary(" << node->op.type_name() << ")\n";
            oss << print_expr(node->operand, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<LiteralExpr>>) {
            oss << ind << "Literal(";
            std::visit([&](auto val) {
                using VT = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<VT, std::string_view>) {
                    oss << "\"" << val << "\"";
                } else {
                    oss << val;
                }
            }, node->value);
            oss << ")\n";
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<IdentifierExpr>>) {
            oss << ind << "Identifier(" << node->name << ")\n";
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<CallExpr>>) {
            oss << ind << "Call\n";
            oss << print_expr(node->callee, indent + 1);
            for (const auto& arg : node->arguments) {
                oss << print_expr(arg, indent + 1);
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<AssignmentExpr>>) {
            oss << ind << "Assignment(" << node->op.type_name() << ")\n";
            oss << print_expr(node->target, indent + 1);
            oss << print_expr(node->value, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<TernaryExpr>>) {
            oss << ind << "Ternary\n";
            oss << print_expr(node->condition, indent + 1);
            oss << print_expr(node->true_branch, indent + 1);
            oss << print_expr(node->false_branch, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<CastExpr>>) {
            oss << ind << "Cast(" << node->target_type << ")\n";
            oss << print_expr(node->expression, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<ArrayAccessExpr>>) {
            oss << ind << "ArrayAccess\n";
            oss << print_expr(node->array, indent + 1);
            oss << print_expr(node->index, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<MemberAccessExpr>>) {
            oss << ind << "MemberAccess(" << (node->is_pointer ? "->" : ".") 
                << node->member << ")\n";
            oss << print_expr(node->object, indent + 1);
        }
    }, expr);

    return oss.str();
}

std::string ASTPrinter::print_stmt(const Stmt& stmt, int indent) const {
    std::ostringstream oss;
    std::string ind = indent_str(indent);

    std::visit([&](const auto& node) {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, std::unique_ptr<VarDecl>>) {
            oss << ind << "VarDecl(" << node->type << " " << node->name << ")";
            if (node->initializer) {
                oss << " =\n";
                oss << print_expr(*node->initializer, indent + 1);
            } else {
                oss << "\n";
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<FuncDecl>>) {
            oss << ind << "FuncDecl(" << node->return_type << " " << node->name << ")\n";
            if (node->body) {
                for (const auto& s : *node->body) {
                    oss << print_stmt(s, indent + 1);
                }
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<BlockStmt>>) {
            oss << ind << "Block\n";
            for (const auto& s : node->statements) {
                oss << print_stmt(s, indent + 1);
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<ExprStmt>>) {
            oss << ind << "ExprStmt\n";
            oss << print_expr(node->expression, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<ReturnStmt>>) {
            oss << ind << "Return\n";
            if (node->value) {
                oss << print_expr(*node->value, indent + 1);
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<IfStmt>>) {
            oss << ind << "If\n";
            oss << print_expr(node->condition, indent + 1);
            oss << print_stmt(node->then_branch, indent + 1);
            if (node->else_branch) {
                oss << ind << "Else\n";
                oss << print_stmt(*node->else_branch, indent + 1);
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<WhileStmt>>) {
            oss << ind << "While\n";
            oss << print_expr(node->condition, indent + 1);
            oss << print_stmt(node->body, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<ForStmt>>) {
            oss << ind << "For\n";
            if (node->initializer) {
                oss << ind << "  Init\n";
                oss << print_stmt(*node->initializer, indent + 2);
            }
            if (node->condition) {
                oss << ind << "  Cond\n";
                oss << print_expr(*node->condition, indent + 2);
            }
            if (node->increment) {
                oss << ind << "  Incr\n";
                oss << print_expr(*node->increment, indent + 2);
            }
            oss << print_stmt(node->body, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<DoWhileStmt>>) {
            oss << ind << "DoWhile\n";
            oss << print_stmt(node->body, indent + 1);
            oss << ind << "  Cond\n";
            oss << print_expr(node->condition, indent + 2);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<SwitchStmt>>) {
            oss << ind << "Switch\n";
            oss << print_expr(node->expression, indent + 1);
            for (const auto& c : node->cases) {
                oss << print_stmt(c, indent + 1);
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<CaseStmt>>) {
            oss << ind << "Case(";
            std::visit([&](auto val) { oss << val; }, node->value);
            oss << ")\n";
            for (const auto& s : node->body) {
                oss << print_stmt(s, indent + 1);
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<BreakStmt>>) {
            oss << ind << "Break\n";
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<ContinueStmt>>) {
            oss << ind << "Continue\n";
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<GotoStmt>>) {
            oss << ind << "Goto(" << node->label << ")\n";
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<LabelStmt>>) {
            oss << ind << "Label(" << node->name << ")\n";
            oss << print_stmt(node->statement, indent + 1);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<StructDecl>>) {
            oss << ind << "StructDecl(" << node->name << ")\n";
            for (const auto& member : node->members) {
                oss << ind << "  Member(" << member.type << " " << member.name << ")\n";
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<EnumDecl>>) {
            oss << ind << "EnumDecl(" << node->name << ")\n";
            for (const auto& member : node->members) {
                oss << ind << "  Member(" << member.first;
                if (member.second) oss << " = " << *member.second;
                oss << ")\n";
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<TypedefDecl>>) {
            oss << ind << "Typedef(" << node->original_type << " -> " << node->alias << ")\n";
        }
        else {
            oss << ind << "[UnknownStmt]\n";
        }
    }, stmt);

    return oss.str();
}

} // namespace compiler
