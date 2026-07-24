#include "parser.hpp"
#include <format>
#include <unordered_set>

namespace compiler {

Parser::Parser(std::span<const Token> tokens) : tokens_(tokens) {}

// ========== TOKEN NAVIGATION ==========

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::is_at_end() const noexcept {
    return peek().type == TokenType::EndOfFile;
}

const Token& Parser::advance() {
    if (!is_at_end()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const noexcept {
    if (is_at_end()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match_any(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (match(type)) return true;
    }
    return false;
}

Result<Token> Parser::consume(TokenType type, std::string_view message) {
    if (check(type)) return advance();
    return std::unexpected(error(peek(), message));
}

Result<Token> Parser::consume_type(std::string_view message) {
    if (peek().is_type() || check(TokenType::Identifier)) return advance();
    return std::unexpected(error(peek(), message));
}

// ========== ERROR HANDLING ==========

CompilerError Parser::error(const Token& token, std::string_view message) const {
    return CompilerError{
        .code = ErrorCode::UnexpectedToken,
        .message = std::format("{} (got '{}')", message, token.value),
        .location = token.location,
        .notes = {}
    };
}

void Parser::synchronize() {
    advance();
    while (!is_at_end()) {
        if (previous().type == TokenType::Semicolon) return;
        switch (peek().type) {
            case TokenType::KwStruct:
            case TokenType::KwEnum:
            case TokenType::KwIf:
            case TokenType::KwWhile:
            case TokenType::KwFor:
            case TokenType::KwReturn:
            case TokenType::KwInt:
            case TokenType::KwVoid:
            case TokenType::KwChar:
            case TokenType::KwFloat:
            case TokenType::KwDouble:
                return;
            default:
                break;
        }
        advance();
    }
}

// ========== PARSE ENTRY POINT ==========

Result<Program> Parser::parse() {
    return program();
}

// ========== PROGRAM ==========

Result<Program> Parser::program() {
    Program program;
    while (!is_at_end()) {
        if (auto decl = declaration(); decl) {
            program.declarations.push_back(std::move(*decl));
        } else {
            return std::unexpected(decl.error());
        }
    }
    return program;
}

// ========== DECLARATIONS ==========

Result<Stmt> Parser::declaration() {
    if (check(TokenType::KwStruct)) return struct_declaration();
    if (check(TokenType::KwClass)) return class_declaration();
    if (check(TokenType::KwEnum)) return enum_declaration();
    if (check(TokenType::KwTypedef)) return typedef_declaration();

    // Lookahead: type identifier ( -> function, type identifier ;/=/[ -> variable
    if (peek().is_type() || check(TokenType::Identifier)) {
        size_t lookahead = current_;

        // Skip type keyword or identifier (for typedef'd types)
        lookahead++;

        // Skip pointer stars before name
        while (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Star) {
            lookahead++;
        }

        // Next should be identifier (name)
        if (lookahead >= tokens_.size() || tokens_[lookahead].type != TokenType::Identifier) {
            return var_declaration(); // Try anyway, let it fail with proper error
        }
        lookahead++; // skip name

        // Skip pointer stars after name
        while (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Star) {
            lookahead++;
        }

        // Now check what follows the name
        if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::LParen) {
            return func_declaration();
        }
        return var_declaration();
    }

    return std::unexpected(error(peek(), "Expected declaration"));
}

Result<Stmt> Parser::var_declaration() {
    auto type_tok = consume_any_type("Expected type specifier");
    if (!type_tok) return std::unexpected(type_tok.error());

    auto decl = std::make_unique<VarDecl>();
    decl->location = type_tok->location;
    decl->type = type_tok->value;

    // Parse pointer stars as part of the type (e.g., int** ptr)
    while (match(TokenType::Star)) {
        decl->is_pointer = true;
    }

    auto name_tok = consume(TokenType::Identifier, "Expected variable name");
    if (!name_tok) return std::unexpected(name_tok.error());
    decl->name = name_tok->value;

    // Parse array dimensions after the name (e.g., int arr[10])
    if (match(TokenType::LBracket)) {
        if (check(TokenType::IntegerLiteral)) {
            auto size_tok = advance();
            decl->array_size = std::stoll(std::string(size_tok.value));
        }
        auto rb = consume(TokenType::RBracket, "Expected ']' after array size");
        if (!rb) return std::unexpected(rb.error());
    }

    if (match(TokenType::Equal)) {
        auto init = expression();
        if (!init) return std::unexpected(init.error());
        decl->initializer = std::move(*init);
    }

    auto semi = consume(TokenType::Semicolon, "Expected ';' after variable declaration");
    if (!semi) return std::unexpected(semi.error());
    return decl;
}

Result<Stmt> Parser::func_declaration() {
    auto return_type = consume_any_type("Expected return type");
    if (!return_type) return std::unexpected(return_type.error());
    auto name_tok = consume(TokenType::Identifier, "Expected function name");
    if (!name_tok) return std::unexpected(name_tok.error());

    auto lp = consume(TokenType::LParen, "Expected '(' after function name");
    if (!lp) return std::unexpected(lp.error());

    auto params = parameter_list();
    if (!params) return std::unexpected(params.error());

    auto rp = consume(TokenType::RParen, "Expected ')' after parameters");
    if (!rp) return std::unexpected(rp.error());

    auto decl = std::make_unique<FuncDecl>();
    decl->location = return_type->location;
    decl->return_type = return_type->value;
    decl->name = name_tok->value;
    decl->parameters = std::move(*params);

    if (check(TokenType::LBrace)) {
        auto body_stmt = block_statement();
        if (!body_stmt) return std::unexpected(body_stmt.error());
        if (auto* block = std::get_if<std::unique_ptr<BlockStmt>>(&*body_stmt)) {
            decl->body = std::move((*block)->statements);
        }
    } else {
        auto semi = consume(TokenType::Semicolon, "Expected ';' for forward declaration");
        if (!semi) return std::unexpected(semi.error());
    }

    return decl;
}
Result<Token> Parser::consume_any_type(std::string_view message) {
    if (check(TokenType::KwVoid) || check(TokenType::KwChar) || check(TokenType::KwShort) ||
        check(TokenType::KwInt) || check(TokenType::KwLong) || check(TokenType::KwFloat) ||
        check(TokenType::KwDouble) || check(TokenType::KwBool) || check(TokenType::KwAuto) ||
        check(TokenType::KwSigned) || check(TokenType::KwUnsigned) ||
        check(TokenType::KwStruct) || check(TokenType::KwUnion) || check(TokenType::KwEnum) ||
        check(TokenType::KwClass) || check(TokenType::KwTypedef) ||
        check(TokenType::Identifier)) {  // for user-defined types
            return advance();
        }
        return std::unexpected(error(peek(), message));
}

Result<Stmt> Parser::struct_declaration() {
    auto kw = consume(TokenType::KwStruct, "Expected 'struct'");
    if (!kw) return std::unexpected(kw.error());
    auto name_tok = consume(TokenType::Identifier, "Expected struct name");
    if (!name_tok) return std::unexpected(name_tok.error());

    auto lb = consume(TokenType::LBrace, "Expected '{' before struct body");
    if (!lb) return std::unexpected(lb.error());

    auto decl = std::make_unique<StructDecl>();
    decl->location = name_tok->location;
    decl->name = name_tok->value;

    while (!check(TokenType::RBrace) && !is_at_end()) {
        auto member = var_declaration();
        if (!member) return std::unexpected(member.error());
        if (auto* var = std::get_if<std::unique_ptr<VarDecl>>(&*member)) {
            decl->members.push_back(std::move(**var));
        }
    }

    auto rb = consume(TokenType::RBrace, "Expected '}' after struct body");
    if (!rb) return std::unexpected(rb.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after struct declaration");
    if (!semi) return std::unexpected(semi.error());
    return decl;
}

Result<Stmt> Parser::class_declaration() {
    auto kw = consume(TokenType::KwClass, "Expected 'class'");
    if (!kw) return std::unexpected(kw.error());
    auto name_tok = consume(TokenType::Identifier, "Expected class name");
    if (!name_tok) return std::unexpected(name_tok.error());

    auto lb = consume(TokenType::LBrace, "Expected '{' before class body");
    if (!lb) return std::unexpected(lb.error());

    auto decl = std::make_unique<StructDecl>();
    decl->location = name_tok->location;
    decl->name = name_tok->value;
    user_defined_types_.insert(std::string(name_tok->value));

    while (!check(TokenType::RBrace) && !is_at_end()) {
        // Skip access specifiers: public:, private:, protected:
        if (check(TokenType::KwPublic) || check(TokenType::KwPrivate) || check(TokenType::KwProtected)) {
            advance(); // consume the keyword
            if (!match(TokenType::Colon)) {
                return std::unexpected(error(peek(), "Expected ':' after access specifier"));
            }
            continue;
        }

        // Try to parse as method (function declaration inside class)
        if (peek().is_type() || check(TokenType::Identifier) || check(TokenType::KwVoid)) {
            size_t lookahead = current_;
            lookahead++;
            while (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Star) {
                lookahead++;
            }
            if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Identifier) {
                lookahead++;
                if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::LParen) {
                    // It's a method - parse and discard for now (no method storage in StructDecl)
                    auto method = func_declaration();
                    if (!method) return std::unexpected(method.error());
                    continue;
                }
            }
        }

        // Parse as field (variable declaration)
        auto member = var_declaration();
        if (!member) return std::unexpected(member.error());
        if (auto* var = std::get_if<std::unique_ptr<VarDecl>>(&*member)) {
            decl->members.push_back(std::move(**var));
        }
    }

    auto rb = consume(TokenType::RBrace, "Expected '}' after class body");
    if (!rb) return std::unexpected(rb.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after class declaration");
    if (!semi) return std::unexpected(semi.error());
    return decl;
}

Result<Stmt> Parser::enum_declaration() {
    auto kw = consume(TokenType::KwEnum, "Expected 'enum'");
    if (!kw) return std::unexpected(kw.error());
    auto name_tok = consume(TokenType::Identifier, "Expected enum name");
    if (!name_tok) return std::unexpected(name_tok.error());

    auto lb = consume(TokenType::LBrace, "Expected '{' before enum body");
    if (!lb) return std::unexpected(lb.error());

    auto decl = std::make_unique<EnumDecl>();
    decl->location = name_tok->location;
    decl->name = name_tok->value;

    do {
        auto member_tok = consume(TokenType::Identifier, "Expected enum member");
        if (!member_tok) return std::unexpected(member_tok.error());
        std::optional<int64_t> value;
        if (match(TokenType::Equal)) {
            auto val_tok = consume(TokenType::IntegerLiteral, "Expected integer value");
            if (!val_tok) return std::unexpected(val_tok.error());
            value = std::stoll(std::string(val_tok->value));
        }
        decl->members.emplace_back(member_tok->value, value);
    } while (match(TokenType::Comma) && !check(TokenType::RBrace));

    auto rb = consume(TokenType::RBrace, "Expected '}' after enum body");
    if (!rb) return std::unexpected(rb.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after enum declaration");
    if (!semi) return std::unexpected(semi.error());
    return decl;
}

Result<Stmt> Parser::typedef_declaration() {
    auto kw = consume(TokenType::KwTypedef, "Expected 'typedef'");
    if (!kw) return std::unexpected(kw.error());
    auto type_tok = consume_type("Expected type");
    if (!type_tok) return std::unexpected(type_tok.error());
    auto alias_tok = consume(TokenType::Identifier, "Expected alias name");
    if (!alias_tok) return std::unexpected(alias_tok.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after typedef");
    if (!semi) return std::unexpected(semi.error());

    auto decl = std::make_unique<TypedefDecl>();
    decl->location = previous().location;
    decl->original_type = type_tok->value;
    decl->alias = alias_tok->value;
    user_defined_types_.insert(std::string(alias_tok->value));
    return decl;
}

Result<std::vector<ParamDecl>> Parser::parameter_list() {
    std::vector<ParamDecl> params;

    // Empty parameter list: func() or func(void)
    if (check(TokenType::RParen)) {
        return params;
    }

    // void parameter list: func(void) - C style empty params
    if (check(TokenType::KwVoid)) {
        advance();
        return params;
    }

    do {
        auto type_tok = consume_any_type("Expected parameter type");
        if (!type_tok) return std::unexpected(type_tok.error());
        auto name_tok = consume(TokenType::Identifier, "Expected parameter name");
        if (!name_tok) return std::unexpected(name_tok.error());

        ParamDecl param;
        param.type = type_tok->value;
        param.name = name_tok->value;

        if (match(TokenType::LBracket)) {
            auto rb = consume(TokenType::RBracket, "Expected ']'");
            if (!rb) return std::unexpected(rb.error());
            param.is_pointer = true;
        }

        params.push_back(param);
    } while (match(TokenType::Comma));

    return params;
}

// ========== STATEMENTS ==========

Result<Stmt> Parser::statement() {
    if (check(TokenType::KwIf)) return if_statement();
    if (check(TokenType::KwWhile)) return while_statement();
    if (check(TokenType::KwFor)) return for_statement();
    if (check(TokenType::KwDo)) return do_while_statement();
    if (check(TokenType::KwReturn)) return return_statement();
    if (check(TokenType::KwBreak)) return break_statement();
    if (check(TokenType::KwContinue)) return continue_statement();
    if (check(TokenType::KwSwitch)) return switch_statement();
    if (check(TokenType::KwGoto)) return goto_statement();
    if (check(TokenType::LBrace)) return block_statement();

    // Check for declarations using lookahead (like declaration() does)
    // A declaration starts with: type-keyword, void, or identifier that is followed by
    // another identifier or pointer-star-then-identifier (for user-defined types)
    if (peek().is_type() || check(TokenType::KwVoid) || check(TokenType::KwStruct) || 
        check(TokenType::KwEnum) || check(TokenType::KwTypedef) ||
        check(TokenType::KwClass) || check(TokenType::KwExtern) || 
        check(TokenType::KwStatic) || check(TokenType::KwConst) ||
        check(TokenType::KwInline)) {
        return declaration();
    }

    // Check if identifier-starting line is a declaration (user-defined type)
    // Format: ident [star*] ident ( -> function or ;/=/[ -> variable)
    if (check(TokenType::Identifier)) {
        size_t lookahead = current_;
        lookahead++; // skip first identifier (potential type)
        // Skip pointer stars
        while (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Star) {
            lookahead++;
        }
        // If next is identifier, this looks like a declaration
        if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Identifier) {
            lookahead++; // skip name
            // Skip more pointer stars after name
            while (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Star) {
                lookahead++;
            }
            // Check what follows: ( -> function, ;/=/[/, -> variable
            if (lookahead < tokens_.size()) {
                auto next = tokens_[lookahead].type;
                if (next == TokenType::LParen || next == TokenType::Semicolon ||
                    next == TokenType::Equal || next == TokenType::LBracket ||
                    next == TokenType::Comma) {
                    return declaration();
                }
            }
        }

        // Lookahead for label: identifier :
        lookahead = current_ + 1;
        if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Colon) {
            return label_statement();
        }
    }

    return expr_statement();
}

Result<Stmt> Parser::block_statement() {
    auto lb = consume(TokenType::LBrace, "Expected '{'");
    if (!lb) return std::unexpected(lb.error());

    auto block = std::make_unique<BlockStmt>();
    block->location = previous().location;
    while (!check(TokenType::RBrace) && !is_at_end()) {
        auto stmt = statement();
        if (!stmt) return std::unexpected(stmt.error());
        block->statements.push_back(std::move(*stmt));
    }

    auto rb = consume(TokenType::RBrace, "Expected '}'");
    if (!rb) return std::unexpected(rb.error());
    return block;
}

Result<Stmt> Parser::expr_statement() {
    auto expr = expression();
    if (!expr) return std::unexpected(expr.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after expression");
    if (!semi) return std::unexpected(semi.error());
    auto stmt = std::make_unique<ExprStmt>();
    stmt->location = previous().location;
    stmt->expression = std::move(*expr);
    return stmt;
}

Result<Stmt> Parser::if_statement() {
    auto kw = consume(TokenType::KwIf, "Expected 'if'");
    if (!kw) return std::unexpected(kw.error());
    auto lp = consume(TokenType::LParen, "Expected '(' after 'if'");
    if (!lp) return std::unexpected(lp.error());
    auto condition = expression();
    if (!condition) return std::unexpected(condition.error());
    auto rp = consume(TokenType::RParen, "Expected ')' after condition");
    if (!rp) return std::unexpected(rp.error());

    auto then_branch = statement();
    if (!then_branch) return std::unexpected(then_branch.error());

    auto stmt = std::make_unique<IfStmt>();
    stmt->location = previous().location;
    stmt->condition = std::move(*condition);
    stmt->then_branch = std::move(*then_branch);

    if (match(TokenType::KwElse)) {
        auto else_branch = statement();
        if (!else_branch) return std::unexpected(else_branch.error());
        stmt->else_branch = std::move(*else_branch);
    }

    return stmt;
}

Result<Stmt> Parser::while_statement() {
    auto kw = consume(TokenType::KwWhile, "Expected 'while'");
    if (!kw) return std::unexpected(kw.error());
    auto lp = consume(TokenType::LParen, "Expected '(' after 'while'");
    if (!lp) return std::unexpected(lp.error());
    auto condition = expression();
    if (!condition) return std::unexpected(condition.error());
    auto rp = consume(TokenType::RParen, "Expected ')' after condition");
    if (!rp) return std::unexpected(rp.error());
    auto body = statement();
    if (!body) return std::unexpected(body.error());

    auto stmt = std::make_unique<WhileStmt>();
    stmt->location = previous().location;
    stmt->condition = std::move(*condition);
    stmt->body = std::move(*body);
    return stmt;
}

Result<Stmt> Parser::for_statement() {
    auto kw = consume(TokenType::KwFor, "Expected 'for'");
    if (!kw) return std::unexpected(kw.error());
    auto lp = consume(TokenType::LParen, "Expected '(' after 'for'");
    if (!lp) return std::unexpected(lp.error());

    auto stmt = std::make_unique<ForStmt>();
    stmt->location = previous().location;
    stmt->location = previous().location;

    if (!check(TokenType::Semicolon)) {
        // Check if initializer is a declaration (type keyword, void, or identifier-type)
        bool is_decl = peek().is_type() || check(TokenType::KwVoid);
        if (!is_decl && check(TokenType::Identifier)) {
            // Lookahead: identifier [star*] identifier -> user-defined type declaration
            size_t lookahead = current_ + 1;
            while (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Star) {
                lookahead++;
            }
            if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::Identifier) {
                is_decl = true;
            }
        }
        if (is_decl) {
            auto init = var_declaration();
            if (!init) return std::unexpected(init.error());
            stmt->initializer = std::move(*init);
        } else {
            auto init_expr = expression();
            if (!init_expr) return std::unexpected(init_expr.error());
            auto semi = consume(TokenType::Semicolon, "Expected ';' after for initializer");
            if (!semi) return std::unexpected(semi.error());
            auto expr_stmt = std::make_unique<ExprStmt>();
            expr_stmt->location = previous().location;
            expr_stmt->expression = std::move(*init_expr);
            stmt->initializer = std::move(expr_stmt);
        }
    } else {
        advance();
    }

    if (!check(TokenType::Semicolon)) {
        auto cond = expression();
        if (!cond) return std::unexpected(cond.error());
        stmt->condition = std::move(*cond);
    }
    auto semi2 = consume(TokenType::Semicolon, "Expected ';' after for condition");
    if (!semi2) return std::unexpected(semi2.error());

    if (!check(TokenType::RParen)) {
        auto inc = expression();
        if (!inc) return std::unexpected(inc.error());
        stmt->increment = std::move(*inc);
    }
    auto rp = consume(TokenType::RParen, "Expected ')' after for clauses");
    if (!rp) return std::unexpected(rp.error());

    auto body = statement();
    if (!body) return std::unexpected(body.error());
    stmt->body = std::move(*body);
    return stmt;
}

Result<Stmt> Parser::do_while_statement() {
    auto kw = consume(TokenType::KwDo, "Expected 'do'");
    if (!kw) return std::unexpected(kw.error());
    auto body = statement();
    if (!body) return std::unexpected(body.error());
    auto wkw = consume(TokenType::KwWhile, "Expected 'while'");
    if (!wkw) return std::unexpected(wkw.error());
    auto lp = consume(TokenType::LParen, "Expected '(' after 'while'");
    if (!lp) return std::unexpected(lp.error());
    auto condition = expression();
    if (!condition) return std::unexpected(condition.error());
    auto rp = consume(TokenType::RParen, "Expected ')' after condition");
    if (!rp) return std::unexpected(rp.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after do-while");
    if (!semi) return std::unexpected(semi.error());

    auto stmt = std::make_unique<DoWhileStmt>();
    stmt->location = previous().location;
    stmt->body = std::move(*body);
    stmt->condition = std::move(*condition);
    return stmt;
}

Result<Stmt> Parser::return_statement() {
    auto kw = consume(TokenType::KwReturn, "Expected 'return'");
    if (!kw) return std::unexpected(kw.error());

    auto stmt = std::make_unique<ReturnStmt>();
    stmt->location = previous().location;
    stmt->location = previous().location;
    if (!check(TokenType::Semicolon)) {
        auto val = expression();
        if (!val) return std::unexpected(val.error());
        stmt->value = std::move(*val);
    }
    auto semi = consume(TokenType::Semicolon, "Expected ';' after return");
    if (!semi) return std::unexpected(semi.error());
    return stmt;
}

Result<Stmt> Parser::break_statement() {
    auto kw = consume(TokenType::KwBreak, "Expected 'break'");
    if (!kw) return std::unexpected(kw.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after break");
    if (!semi) return std::unexpected(semi.error());
    auto stmt = std::make_unique<BreakStmt>();
    stmt->location = previous().location;
    return stmt;
}

Result<Stmt> Parser::continue_statement() {
    auto kw = consume(TokenType::KwContinue, "Expected 'continue'");
    if (!kw) return std::unexpected(kw.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after continue");
    if (!semi) return std::unexpected(semi.error());
    auto stmt = std::make_unique<ContinueStmt>();
    stmt->location = previous().location;
    return stmt;
}

Result<Stmt> Parser::switch_statement() {
    auto kw = consume(TokenType::KwSwitch, "Expected 'switch'");
    if (!kw) return std::unexpected(kw.error());
    auto lp = consume(TokenType::LParen, "Expected '(' after 'switch'");
    if (!lp) return std::unexpected(lp.error());
    auto expr = expression();
    if (!expr) return std::unexpected(expr.error());
    auto rp = consume(TokenType::RParen, "Expected ')' after switch expression");
    if (!rp) return std::unexpected(rp.error());
    auto lb = consume(TokenType::LBrace, "Expected '{' before switch body");
    if (!lb) return std::unexpected(lb.error());

    auto stmt = std::make_unique<SwitchStmt>();
    stmt->location = previous().location;
    stmt->expression = std::move(*expr);

    while (!check(TokenType::RBrace) && !is_at_end()) {
        auto s = statement();
        if (!s) return std::unexpected(s.error());
        stmt->cases.push_back(std::move(*s));
    }

    auto rb = consume(TokenType::RBrace, "Expected '}' after switch body");
    if (!rb) return std::unexpected(rb.error());
    return stmt;
}

Result<Stmt> Parser::goto_statement() {
    auto kw = consume(TokenType::KwGoto, "Expected 'goto'");
    if (!kw) return std::unexpected(kw.error());
    auto label = consume(TokenType::Identifier, "Expected label name");
    if (!label) return std::unexpected(label.error());
    auto semi = consume(TokenType::Semicolon, "Expected ';' after goto");
    if (!semi) return std::unexpected(semi.error());

    auto stmt = std::make_unique<GotoStmt>();
    stmt->location = previous().location;
    stmt->label = label->value;
    return stmt;
}

Result<Stmt> Parser::label_statement() {
    auto name = consume(TokenType::Identifier, "Expected label name");
    if (!name) return std::unexpected(name.error());
    auto col = consume(TokenType::Colon, "Expected ':' after label");
    if (!col) return std::unexpected(col.error());
    auto stmt_body = statement();
    if (!stmt_body) return std::unexpected(stmt_body.error());

    auto stmt = std::make_unique<LabelStmt>();
    stmt->location = previous().location;
    stmt->location = previous().location;
    stmt->name = name->value;
    stmt->statement = std::move(*stmt_body);
    return stmt;
}

// ========== EXPRESSIONS ==========

Result<Expr> Parser::expression() {
    return assignment();
}

Result<Expr> Parser::assignment() {
    auto expr = ternary();
    if (!expr) return std::unexpected(expr.error());

    if (match_any({TokenType::Equal, TokenType::PlusEqual, TokenType::MinusEqual,
                   TokenType::StarEqual, TokenType::SlashEqual, TokenType::PercentEqual})) {
        Token op = previous();
        auto value = assignment();
        if (!value) return std::unexpected(value.error());

        auto assign = std::make_unique<AssignmentExpr>();
        assign->location = previous().location;
        assign->target = std::move(*expr);
        assign->op = op;
        assign->value = std::move(*value);
        return assign;
    }

    return expr;
}

Result<Expr> Parser::ternary() {
    auto expr = logical_or();
    if (!expr) return std::unexpected(expr.error());

    if (match(TokenType::Question)) {
        auto true_branch = expression();
        if (!true_branch) return std::unexpected(true_branch.error());
        auto col = consume(TokenType::Colon, "Expected ':' in ternary");
        if (!col) return std::unexpected(col.error());
        auto false_branch = ternary();
        if (!false_branch) return std::unexpected(false_branch.error());

        auto ternary_expr = std::make_unique<TernaryExpr>();
        ternary_expr->location = previous().location;
        ternary_expr->condition = std::move(*expr);
        ternary_expr->true_branch = std::move(*true_branch);
        ternary_expr->false_branch = std::move(*false_branch);
        return ternary_expr;
    }

    return expr;
}

Result<Expr> Parser::logical_or() {
    auto expr = logical_and();
    if (!expr) return std::unexpected(expr.error());

    while (match(TokenType::PipePipe)) {
        Token op = previous();
        auto right = logical_and();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::logical_and() {
    auto expr = bitwise_or();
    if (!expr) return std::unexpected(expr.error());

    while (match(TokenType::AmpersandAmpersand)) {
        Token op = previous();
        auto right = bitwise_or();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::bitwise_or() {
    auto expr = bitwise_xor();
    if (!expr) return std::unexpected(expr.error());

    while (match(TokenType::Pipe)) {
        Token op = previous();
        auto right = bitwise_xor();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::bitwise_xor() {
    auto expr = bitwise_and();
    if (!expr) return std::unexpected(expr.error());

    while (match(TokenType::Caret)) {
        Token op = previous();
        auto right = bitwise_and();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::bitwise_and() {
    auto expr = equality();
    if (!expr) return std::unexpected(expr.error());

    while (match(TokenType::Ampersand)) {
        Token op = previous();
        auto right = equality();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::equality() {
    auto expr = comparison();
    if (!expr) return std::unexpected(expr.error());

    while (match_any({TokenType::BangEqual, TokenType::EqualEqual})) {
        Token op = previous();
        auto right = comparison();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::comparison() {
    auto expr = shift();
    if (!expr) return std::unexpected(expr.error());

    while (match_any({TokenType::Greater, TokenType::GreaterEqual,
                      TokenType::Less, TokenType::LessEqual})) {
        Token op = previous();
        auto right = shift();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::shift() {
    auto expr = term();
    if (!expr) return std::unexpected(expr.error());

    while (match_any({TokenType::Less, TokenType::Greater})) {
        Token op = previous();
        auto right = term();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::term() {
    auto expr = factor();
    if (!expr) return std::unexpected(expr.error());

    while (match_any({TokenType::Minus, TokenType::Plus})) {
        Token op = previous();
        auto right = factor();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::factor() {
    auto expr = unary();
    if (!expr) return std::unexpected(expr.error());

    while (match_any({TokenType::Slash, TokenType::Star, TokenType::Percent})) {
        Token op = previous();
        auto right = unary();
        if (!right) return std::unexpected(right.error());

        auto binary = std::make_unique<BinaryExpr>();
        binary->location = previous().location;
        binary->left = std::move(*expr);
        binary->op = op;
        binary->right = std::move(*right);
        expr = std::move(binary);
    }

    return expr;
}

Result<Expr> Parser::unary() {
    if (match_any({TokenType::Bang, TokenType::Minus, TokenType::Plus,
                   TokenType::Tilde, TokenType::MinusMinus, TokenType::PlusPlus})) {
        Token op = previous();
        auto operand = unary();
        if (!operand) return std::unexpected(operand.error());

        auto unary_expr = std::make_unique<UnaryExpr>();
        unary_expr->location = previous().location;
        unary_expr->op = op;
        unary_expr->operand = std::move(*operand);
        return unary_expr;
    }

    return postfix();
}

Result<Expr> Parser::postfix() {
    auto expr = primary();
    if (!expr) return std::unexpected(expr.error());

    while (true) {
        if (match(TokenType::LParen)) {
            auto args = argument_list();
            if (!args) return std::unexpected(args.error());
            auto rp = consume(TokenType::RParen, "Expected ')' after arguments");
            if (!rp) return std::unexpected(rp.error());

            auto call = std::make_unique<CallExpr>();
            call->location = previous().location;
            call->callee = std::move(*expr);
            call->arguments = std::move(*args);
            expr = std::move(call);
        }
        else if (match(TokenType::LBracket)) {
            auto index = expression();
            if (!index) return std::unexpected(index.error());
            auto rb = consume(TokenType::RBracket, "Expected ']' after index");
            if (!rb) return std::unexpected(rb.error());

            auto access = std::make_unique<ArrayAccessExpr>();
            access->location = previous().location;
            access->array = std::move(*expr);
            access->index = std::move(*index);
            expr = std::move(access);
        }
        else if (match(TokenType::Dot) || match(TokenType::Arrow)) {
            bool is_ptr = previous().type == TokenType::Arrow;
            auto member = consume(TokenType::Identifier, "Expected member name");
            if (!member) return std::unexpected(member.error());

            auto access = std::make_unique<MemberAccessExpr>();
            access->location = previous().location;
            access->object = std::move(*expr);
            access->member = member->value;
            access->is_pointer = is_ptr;
            expr = std::move(access);
        }
        else {
            break;
        }
    }

    return expr;
}

Result<Expr> Parser::primary() {
    if (match(TokenType::KwTrue)) {
        auto lit = std::make_unique<LiteralExpr>();
        lit->location = previous().location;
        lit->value = true;
        return lit;
    }
    if (match(TokenType::KwFalse)) {
        auto lit = std::make_unique<LiteralExpr>();
        lit->location = previous().location;
        lit->value = false;
        return lit;
    }
    if (match(TokenType::IntegerLiteral)) {
        auto lit = std::make_unique<LiteralExpr>();
        lit->location = previous().location;
        lit->value = std::stoll(std::string(previous().value));
        return lit;
    }
    if (match(TokenType::FloatLiteral)) {
        auto lit = std::make_unique<LiteralExpr>();
        lit->location = previous().location;
        lit->value = std::stod(std::string(previous().value));
        return lit;
    }
    if (match(TokenType::StringLiteral)) {
        auto lit = std::make_unique<LiteralExpr>();
        lit->location = previous().location;
        lit->value = previous().value;
        return lit;
    }
    if (match(TokenType::CharLiteral)) {
        auto lit = std::make_unique<LiteralExpr>();
        lit->location = previous().location;
        lit->value = previous().value[1];
        return lit;
    }
    if (match(TokenType::Identifier)) {
        auto id = std::make_unique<IdentifierExpr>();
        id->location = previous().location;
        id->name = previous().value;
        return id;
    }
    if (match(TokenType::LParen)) {
        auto expr = expression();
        if (!expr) return std::unexpected(expr.error());
        auto rp = consume(TokenType::RParen, "Expected ')' after expression");
        if (!rp) return std::unexpected(rp.error());
        return expr;
    }

    return std::unexpected(error(peek(), "Expected expression"));
}

Result<Expr> Parser::cast() {
    // Simplified cast: (type) expr
    return unary();
}

Result<std::vector<Expr>> Parser::argument_list() {
    std::vector<Expr> args;

    if (check(TokenType::RParen)) return args;

    do {
        auto arg = expression();
        if (!arg) return std::unexpected(arg.error());
        args.push_back(std::move(*arg));
    } while (match(TokenType::Comma));

    return args;
}

} // namespace compiler
