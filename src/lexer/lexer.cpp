#include "lexer.hpp"
#include "../common/utils.hpp"
#include <format>
#include <unordered_map>

namespace compiler {

Lexer::Lexer(std::string_view source)
    : source_(source.data(), source.size()) {}

Result<std::vector<Token>> Lexer::tokenize() {
    std::vector<Token> tokens;
    tokens.reserve(256);

    while (!is_at_end()) {
        skip_whitespace();
        if (is_at_end()) break;

        char c = peek();

        // Handle line comments: // ...
        if (c == '/' && peek_next() == '/') {
            skip_line_comment();
            continue;
        }

        // Handle block comments: /* ... */
        if (c == '/' && peek_next() == '*') {
            skip_block_comment();
            continue;
        }

        auto result = [this, c]() -> Result<Token> {

            if (utils::is_alpha(c)) return identifier_or_keyword();
            if (utils::is_digit(c)) return number();
            if (c == '\"') return string();
            if (c == '\'') return character();
            if (c == '#') return preprocessor_directive();

            switch (c) {
                case '+':
                    advance();
                    if (match('=')) return make_token(TokenType::PlusEqual, "+=");
                    if (match('+')) return make_token(TokenType::PlusPlus, "++");
                    return make_token(TokenType::Plus, "+");
                case '-':
                    advance();
                    if (match('=')) return make_token(TokenType::MinusEqual, "-=");
                    if (match('-')) return make_token(TokenType::MinusMinus, "--");
                    if (match('>')) return make_token(TokenType::Arrow, "->");
                    return make_token(TokenType::Minus, "-");
                case '*':
                    advance();
                    if (match('=')) return make_token(TokenType::StarEqual, "*=");
                    return make_token(TokenType::Star, "*");
                case '/':
                    advance();
                    if (match('=')) return make_token(TokenType::SlashEqual, "/=");
                    return make_token(TokenType::Slash, "/");
                case '%':
                    advance();
                    if (match('=')) return make_token(TokenType::PercentEqual, "%=");
                    return make_token(TokenType::Percent, "%");
                case '=':
                    advance();
                    if (match('=')) return make_token(TokenType::EqualEqual, "==");
                    return make_token(TokenType::Equal, "=");
                case '!':
                    advance();
                    if (match('=')) return make_token(TokenType::BangEqual, "!=");
                    return make_token(TokenType::Bang, "!");
                case '<':
                    advance();
                    if (match('=')) return make_token(TokenType::LessEqual, "<=");
                    return make_token(TokenType::Less, "<");
                case '>':
                    advance();
                    if (match('=')) return make_token(TokenType::GreaterEqual, ">=");
                    return make_token(TokenType::Greater, ">");
                case '&':
                    advance();
                    if (match('&')) return make_token(TokenType::AmpersandAmpersand, "&&");
                    return make_token(TokenType::Ampersand, "&");
                case '|':
                    advance();
                    if (match('|')) return make_token(TokenType::PipePipe, "||");
                    return make_token(TokenType::Pipe, "|");
                case '^':
                    advance();
                    return make_token(TokenType::Caret, "^");
                case '~':
                    advance();
                    return make_token(TokenType::Tilde, "~");

                case '(': advance(); return make_token(TokenType::LParen, "(");
                case ')': advance(); return make_token(TokenType::RParen, ")");
                case '{': advance(); return make_token(TokenType::LBrace, "{");
                case '}': advance(); return make_token(TokenType::RBrace, "}");
                case '[': advance(); return make_token(TokenType::LBracket, "[");
                case ']': advance(); return make_token(TokenType::RBracket, "]");
                case ';': advance(); return make_token(TokenType::Semicolon, ";");
                case ',': advance(); return make_token(TokenType::Comma, ",");
                case '.': advance(); return make_token(TokenType::Dot, ".");
                case ':': advance(); return make_token(TokenType::Colon, ":");
                case '?': advance(); return make_token(TokenType::Question, "?");

                default:
                    advance();
                    return std::unexpected(make_error(
                        ErrorCode::UnexpectedCharacter,
                        std::format("Unexpected character '{}'", c)
                    ));
            }
        }();

        if (!result) {
            return std::unexpected(result.error());
        }
        tokens.push_back(*result);
    }

    tokens.push_back(make_token(TokenType::EndOfFile, ""));
    return tokens;
}

char Lexer::peek() const noexcept {
    if (is_at_end()) return '\0';
    return source_[current_];
}

char Lexer::peek_next() const noexcept {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

char Lexer::advance() noexcept {
    char c = source_[current_];
    current_++;
    advance_location(c);
    return c;
}

bool Lexer::match(char expected) noexcept {
    if (is_at_end()) return false;
    if (source_[current_] != expected) return false;
    advance();
    return true;
}

bool Lexer::is_at_end() const noexcept {
    return current_ >= source_.size();
}

Result<Token> Lexer::identifier_or_keyword() {
    size_t start = current_;
    SourceLocation start_loc = loc_;

    while (utils::is_alphanumeric(peek())) {
        advance();
    }

    std::string_view text(source_.data() + start, current_ - start);
    TokenType type = check_keyword(text);

    return Token{
        .type = type,
        .value = text,
        .location = start_loc
    };
}

Result<Token> Lexer::number() {
    size_t start = current_;
    SourceLocation start_loc = loc_;
    bool is_float = false;

    while (utils::is_digit(peek())) {
        advance();
    }

    if (peek() == '.' && utils::is_digit(peek_next())) {
        is_float = true;
        advance();
        while (utils::is_digit(peek())) {
            advance();
        }
    }

    if (peek() == 'e' || peek() == 'E') {
        is_float = true;
        advance();
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        if (!utils::is_digit(peek())) {
            return std::unexpected(make_error(
                ErrorCode::InvalidNumberLiteral,
                "Invalid scientific notation"
            ));
        }
        while (utils::is_digit(peek())) {
            advance();
        }
    }

    if (peek() == 'u' || peek() == 'U' || peek() == 'l' || peek() == 'L' || 
        peek() == 'f' || peek() == 'F') {
        if (peek() == 'f' || peek() == 'F') is_float = true;
        advance();
        if ((peek() == 'l' || peek() == 'L') && !is_float) {
            advance();
        }
    }

    std::string_view text(source_.data() + start, current_ - start);
    return Token{
        .type = is_float ? TokenType::FloatLiteral : TokenType::IntegerLiteral,
        .value = text,
        .location = start_loc
    };
}

Result<Token> Lexer::string() {
    size_t start = current_;
    SourceLocation start_loc = loc_;
    advance();

    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') {
            return std::unexpected(make_error(
                ErrorCode::UnterminatedString,
                "Unterminated string literal (newline in string)"
            ));
        }
        if (peek() == '\\') {
            advance();
        }
        advance();
    }

    if (is_at_end()) {
        return std::unexpected(make_error(
            ErrorCode::UnterminatedString,
            "Unterminated string literal"
        ));
    }

    advance();
    std::string_view text(source_.data() + start, current_ - start);

    return Token{
        .type = TokenType::StringLiteral,
        .value = text,
        .location = start_loc
    };
}

Result<Token> Lexer::character() {
    SourceLocation start_loc = loc_;
    advance();

    if (is_at_end() || peek() == '\n') {
        return std::unexpected(make_error(
            ErrorCode::UnexpectedCharacter,
            "Unterminated character literal"
        ));
    }

    if (peek() == '\\') {
        advance();
        if (is_at_end()) {
            return std::unexpected(make_error(
                ErrorCode::UnexpectedCharacter,
                "Unterminated escape sequence"
            ));
        }
    }

    advance();

    if (peek() != '\'') {
        return std::unexpected(make_error(
            ErrorCode::UnexpectedCharacter,
            "Multi-character constant"
        ));
    }

    advance();
    std::string_view text(source_.data() + static_cast<size_t>(start_loc.offset), current_ - static_cast<size_t>(start_loc.offset));

    return Token{
        .type = TokenType::CharLiteral,
        .value = text,
        .location = start_loc
    };
}

Result<Token> Lexer::preprocessor_directive() {
    size_t start = current_;
    SourceLocation start_loc = loc_;
    advance();

    while (peek() == ' ' || peek() == '\t') {
        advance();
    }

    size_t directive_start = current_;
    while (utils::is_alpha(peek())) {
        advance();
    }

    std::string_view directive(source_.data() + directive_start, current_ - directive_start);
    std::string_view full_text(source_.data() + start, current_ - start);

    // Handle #line directive: update line tracking
    if (directive == "line") {
        // Skip whitespace
        while (peek() == ' ' || peek() == '\t') {
            advance();
        }
        // Parse line number
        size_t num_start = current_;
        while (utils::is_digit(peek())) {
            advance();
        }
        if (num_start < current_) {
            std::string_view line_num_str(source_.data() + num_start, current_ - num_start);
            int new_line = std::stoi(std::string(line_num_str));
            loc_.line = new_line;
            loc_.column = 1;
        }
        // Skip rest of line
        while (peek() != '\n' && !is_at_end()) {
            advance();
        }
        // Return as a special token (or skip it)
        return Token{
            .type = TokenType::Hash,
            .value = full_text,
            .location = start_loc
        };
    }

    TokenType type = TokenType::Hash;
    if (directive == "include") type = TokenType::HashInclude;
    else if (directive == "define") type = TokenType::HashDefine;
    else if (directive == "if") type = TokenType::HashIf;
    else if (directive == "ifdef") type = TokenType::HashIfdef;
    else if (directive == "ifndef") type = TokenType::HashIfndef;
    else if (directive == "else") type = TokenType::HashElse;
    else if (directive == "elif") type = TokenType::HashElif;
    else if (directive == "endif") type = TokenType::HashEndif;
    else if (directive == "pragma") type = TokenType::HashPragma;

    return Token{
        .type = type,
        .value = full_text,
        .location = start_loc
    };
}

void Lexer::skip_whitespace() {
    while (utils::is_whitespace(peek())) {
        advance();
    }
}

void Lexer::skip_line_comment() {
    while (peek() != '\n' && !is_at_end()) {
        advance();
    }
    if (peek() == '\n') {
        advance();
    }
}

void Lexer::skip_block_comment() {
    while (!is_at_end()) {
        if (peek() == '*' && peek_next() == '/') {
            advance();
            advance();
            return;
        }
        advance();
    }
}

void Lexer::advance_location(char c) noexcept {
    if (c == '\n') {
        loc_.line++;
        loc_.column = 1;
    } else {
        loc_.column++;
    }
    loc_.offset++;
}

Token Lexer::make_token(TokenType type, std::string_view value) const {
    return Token{
        .type = type,
        .value = value,
        .location = loc_
    };
}

CompilerError Lexer::make_error(ErrorCode code, std::string_view message) const {
    return CompilerError{
        .code = code,
        .message = std::string(message),
        .location = loc_,
        .notes = {}
    };
}

TokenType Lexer::check_keyword(std::string_view text) noexcept {
    static const std::unordered_map<std::string_view, TokenType> keywords = {
        {"auto", TokenType::KwAuto},       {"bool", TokenType::KwBool},
        {"break", TokenType::KwBreak},
        {"case", TokenType::KwCase},       {"char", TokenType::KwChar},
        {"class", TokenType::KwClass},
        {"public", TokenType::KwPublic},
        {"private", TokenType::KwPrivate},
        {"protected", TokenType::KwProtected},
        {"const", TokenType::KwConst},     {"continue", TokenType::KwContinue},
        {"default", TokenType::KwDefault}, {"do", TokenType::KwDo},
        {"double", TokenType::KwDouble},   {"else", TokenType::KwElse},
        {"enum", TokenType::KwEnum},       {"extern", TokenType::KwExtern},
        {"float", TokenType::KwFloat},     {"for", TokenType::KwFor},
        {"goto", TokenType::KwGoto},       {"if", TokenType::KwIf},
        {"inline", TokenType::KwInline},   {"int", TokenType::KwInt},
        {"long", TokenType::KwLong},       {"register", TokenType::KwRegister},
        {"return", TokenType::KwReturn},   {"short", TokenType::KwShort},
        {"signed", TokenType::KwSigned},   {"sizeof", TokenType::KwSizeof},
        {"static", TokenType::KwStatic},   {"struct", TokenType::KwStruct},
        {"switch", TokenType::KwSwitch},   {"typedef", TokenType::KwTypedef},
        {"union", TokenType::KwUnion},     {"unsigned", TokenType::KwUnsigned},
        {"void", TokenType::KwVoid},       {"volatile", TokenType::KwVolatile},
        {"while", TokenType::KwWhile},
        {"true", TokenType::KwTrue},
        {"false", TokenType::KwFalse},
    };

    if (auto it = keywords.find(text); it != keywords.end()) {
        return it->second;
    }
    return TokenType::Identifier;
}

} // namespace compiler
