#pragma once
#include <vector>
#include <span>
#include <expected>
#include <unordered_set>
#include <string>
#include "ast.hpp"
#include "../lexer/token.hpp"
#include "../common/error.hpp"

namespace compiler {

    class Parser {
    public:
        explicit Parser(std::span<const Token> tokens);
        [[nodiscard]] Result<Program> parse();

    private:
        std::span<const Token> tokens_;
        size_t current_ = 0;
        std::unordered_set<std::string> user_defined_types_;

        // Token navigation
        [[nodiscard]] const Token& peek() const;
        [[nodiscard]] const Token& previous() const;
        [[nodiscard]] bool is_at_end() const noexcept;
        const Token& advance();
        [[nodiscard]] bool check(TokenType type) const noexcept;
        [[nodiscard]] bool match(TokenType type);
        [[nodiscard]] bool match_any(std::initializer_list<TokenType> types);

        [[nodiscard]] Result<Token> consume(TokenType type, std::string_view message);
        [[nodiscard]] Result<Token> consume_type(std::string_view message);
        [[nodiscard]] Result<Token> consume_any_type(std::string_view message);

        // Grammar: Program
        [[nodiscard]] Result<Program> program();

        // Grammar: Declarations (NO DUPLICATES)
        [[nodiscard]] Result<Stmt> declaration();
        [[nodiscard]] Result<Stmt> var_declaration();
        [[nodiscard]] Result<Stmt> func_declaration();
        [[nodiscard]] Result<Stmt> struct_declaration();
        [[nodiscard]] Result<Stmt> enum_declaration();
        [[nodiscard]] Result<Stmt> typedef_declaration();
        [[nodiscard]] Result<Stmt> class_declaration();

        // Grammar: Statements
        [[nodiscard]] Result<Stmt> statement();
        [[nodiscard]] Result<Stmt> block_statement();
        [[nodiscard]] Result<Stmt> expr_statement();
        [[nodiscard]] Result<Stmt> if_statement();
        [[nodiscard]] Result<Stmt> while_statement();
        [[nodiscard]] Result<Stmt> for_statement();
        [[nodiscard]] Result<Stmt> do_while_statement();
        [[nodiscard]] Result<Stmt> return_statement();
        [[nodiscard]] Result<Stmt> break_statement();
        [[nodiscard]] Result<Stmt> continue_statement();
        [[nodiscard]] Result<Stmt> switch_statement();
        [[nodiscard]] Result<Stmt> goto_statement();
        [[nodiscard]] Result<Stmt> label_statement();

        // Grammar: Expressions
        [[nodiscard]] Result<Expr> expression();
        [[nodiscard]] Result<Expr> assignment();
        [[nodiscard]] Result<Expr> ternary();
        [[nodiscard]] Result<Expr> logical_or();
        [[nodiscard]] Result<Expr> logical_and();
        [[nodiscard]] Result<Expr> bitwise_or();
        [[nodiscard]] Result<Expr> bitwise_xor();
        [[nodiscard]] Result<Expr> bitwise_and();
        [[nodiscard]] Result<Expr> equality();
        [[nodiscard]] Result<Expr> comparison();
        [[nodiscard]] Result<Expr> shift();
        [[nodiscard]] Result<Expr> term();
        [[nodiscard]] Result<Expr> factor();
        [[nodiscard]] Result<Expr> unary();
        [[nodiscard]] Result<Expr> postfix();
        [[nodiscard]] Result<Expr> primary();
        [[nodiscard]] Result<Expr> cast();

        // Helpers
        [[nodiscard]] Result<std::vector<ParamDecl>> parameter_list();
        [[nodiscard]] Result<std::vector<Expr>> argument_list();
        [[nodiscard]] Result<std::string_view> parse_type();

        // Error handling
        [[nodiscard]] CompilerError error(const Token& token, std::string_view message) const;
        void synchronize();
    };

} // namespace compiler
