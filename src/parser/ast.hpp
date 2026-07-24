#pragma once
#include <memory>
#include <variant>
#include <vector>
#include <optional>
#include <string_view>
#include "../lexer/token.hpp"

namespace compiler {

// Forward declarations
struct BinaryExpr;
struct UnaryExpr;
struct LiteralExpr;
struct IdentifierExpr;
struct CallExpr;
struct AssignmentExpr;
struct TernaryExpr;
struct CastExpr;
struct ArrayAccessExpr;
struct MemberAccessExpr;

struct VarDecl;
struct FuncDecl;
struct ParamDecl;
struct StructDecl;
struct EnumDecl;
struct TypedefDecl;

struct BlockStmt;
struct ExprStmt;
struct ReturnStmt;
struct IfStmt;
struct WhileStmt;
struct ForStmt;
struct DoWhileStmt;
struct BreakStmt;
struct ContinueStmt;
struct SwitchStmt;
struct CaseStmt;
struct GotoStmt;
struct LabelStmt;

// Expression variant
using Expr = std::variant<
    std::unique_ptr<BinaryExpr>,
    std::unique_ptr<UnaryExpr>,
    std::unique_ptr<LiteralExpr>,
    std::unique_ptr<IdentifierExpr>,
    std::unique_ptr<CallExpr>,
    std::unique_ptr<AssignmentExpr>,
    std::unique_ptr<TernaryExpr>,
    std::unique_ptr<CastExpr>,
    std::unique_ptr<ArrayAccessExpr>,
    std::unique_ptr<MemberAccessExpr>
>;

// Statement variant
using Stmt = std::variant<
    std::unique_ptr<VarDecl>,
    std::unique_ptr<FuncDecl>,
    std::unique_ptr<StructDecl>,
    std::unique_ptr<EnumDecl>,
    std::unique_ptr<TypedefDecl>,
    std::unique_ptr<BlockStmt>,
    std::unique_ptr<ExprStmt>,
    std::unique_ptr<ReturnStmt>,
    std::unique_ptr<IfStmt>,
    std::unique_ptr<WhileStmt>,
    std::unique_ptr<ForStmt>,
    std::unique_ptr<DoWhileStmt>,
    std::unique_ptr<BreakStmt>,
    std::unique_ptr<ContinueStmt>,
    std::unique_ptr<SwitchStmt>,
    std::unique_ptr<GotoStmt>,
    std::unique_ptr<LabelStmt>
>;

// ========== EXPRESSIONS ==========

struct BinaryExpr {
    SourceLocation location;
    Expr left;
    Token op;
    Expr right;
};

struct UnaryExpr {
    SourceLocation location;
    Token op;
    Expr operand;
};

struct LiteralExpr {
    SourceLocation location;
    std::variant<int64_t, double, std::string_view, char, bool> value;
};

struct IdentifierExpr {
    SourceLocation location;
    std::string_view name;
};

struct CallExpr {
    SourceLocation location;
    Expr callee;
    std::vector<Expr> arguments;
};

struct AssignmentExpr {
    SourceLocation location;
    Expr target;
    Token op;
    Expr value;
};

struct TernaryExpr {
    SourceLocation location;
    Expr condition;
    Expr true_branch;
    Expr false_branch;
};

struct CastExpr {
    SourceLocation location;
    std::string_view target_type;
    Expr expression;
};

struct ArrayAccessExpr {
    SourceLocation location;
    Expr array;
    Expr index;
};

struct MemberAccessExpr {
    SourceLocation location;
    Expr object;
    std::string_view member;
    bool is_pointer;
};

// ========== DECLARATIONS ==========

struct ParamDecl {
    std::string_view type;
    std::string_view name;
    bool is_pointer = false;
    std::optional<int> array_size;
};

struct VarDecl {
    SourceLocation location;
    std::string_view type;
    std::string_view name;
    bool is_pointer = false;
    std::optional<int> array_size;
    std::optional<Expr> initializer;
    bool is_const = false;
    bool is_static = false;
    bool is_extern = false;
};

struct FuncDecl {
    SourceLocation location;
    std::string_view return_type;
    std::string_view name;
    std::vector<ParamDecl> parameters;
    bool is_variadic = false;
    std::optional<std::vector<Stmt>> body;
    bool is_static = false;
    bool is_extern = false;
    bool is_inline = false;
};

struct StructDecl {
    SourceLocation location;
    std::string_view name;
    std::vector<VarDecl> members;
};

struct EnumDecl {
    SourceLocation location;
    std::string_view name;
    std::vector<std::pair<std::string_view, std::optional<int64_t>>> members;
};

struct TypedefDecl {
    SourceLocation location;
    std::string_view alias;
    std::string_view original_type;
};

// ========== STATEMENTS ==========

struct BlockStmt {
    SourceLocation location;
    std::vector<Stmt> statements;
};

struct ExprStmt {
    SourceLocation location;
    Expr expression;
};

struct ReturnStmt {
    SourceLocation location;
    std::optional<Expr> value;
};

struct IfStmt {
    SourceLocation location;
    Expr condition;
    Stmt then_branch;
    std::optional<Stmt> else_branch;
};

struct WhileStmt {
    SourceLocation location;
    Expr condition;
    Stmt body;
};

struct ForStmt {
    SourceLocation location;
    std::optional<Stmt> initializer;
    std::optional<Expr> condition;
    std::optional<Expr> increment;
    Stmt body;
};

struct DoWhileStmt {
    SourceLocation location;
    Stmt body;
    Expr condition;
};

struct BreakStmt {
    SourceLocation location;
};
struct ContinueStmt {
    SourceLocation location;
};

struct SwitchStmt {
    SourceLocation location;
    Expr expression;
    std::vector<Stmt> cases;
};

struct CaseStmt {
    SourceLocation location;
    std::variant<int64_t, char> value;
    std::vector<Stmt> body;
};

struct GotoStmt {
    SourceLocation location;
    std::string_view label;
};

struct LabelStmt {
    SourceLocation location;
    std::string_view name;
    Stmt statement;
};

// ========== PROGRAM ==========

struct Program {
    std::vector<Stmt> declarations;
};

} // namespace compiler
