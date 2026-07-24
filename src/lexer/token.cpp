#include "token.hpp"

namespace compiler {

bool Token::is_keyword() const noexcept {
    switch (type) {
        case TokenType::KwAuto: case TokenType::KwBool: case TokenType::KwBreak:
        case TokenType::KwCase:
        case TokenType::KwChar: case TokenType::KwConst: case TokenType::KwContinue:
        case TokenType::KwDefault: case TokenType::KwDo: case TokenType::KwDouble:
        case TokenType::KwElse: case TokenType::KwEnum: case TokenType::KwExtern:
        case TokenType::KwFloat: case TokenType::KwFor: case TokenType::KwGoto:
        case TokenType::KwIf: case TokenType::KwInline: case TokenType::KwInt:
        case TokenType::KwLong: case TokenType::KwRegister: case TokenType::KwReturn:
        case TokenType::KwShort: case TokenType::KwSigned: case TokenType::KwSizeof:
        case TokenType::KwStatic: case TokenType::KwStruct: case TokenType::KwSwitch:
        case TokenType::KwTypedef: case TokenType::KwUnion: case TokenType::KwUnsigned:
        case TokenType::KwVoid: case TokenType::KwVolatile: case TokenType::KwWhile:
        case TokenType::KwTrue: case TokenType::KwFalse:
        case TokenType::KwClass:
        case TokenType::KwPublic:
        case TokenType::KwPrivate:
        case TokenType::KwProtected:
            return true;
        default:
            return false;
    }
}

bool Token::is_literal() const noexcept {
    switch (type) {
        case TokenType::IntegerLiteral:
        case TokenType::FloatLiteral:
        case TokenType::StringLiteral:
        case TokenType::CharLiteral:
        case TokenType::BoolLiteral:
            return true;
        default:
            return false;
    }
}

bool Token::is_type() const noexcept {
    switch (type) {
        case TokenType::KwVoid: case TokenType::KwChar: case TokenType::KwShort:
        case TokenType::KwInt: case TokenType::KwLong: case TokenType::KwFloat:
        case TokenType::KwDouble: case TokenType::KwBool: case TokenType::KwSigned: case TokenType::KwUnsigned:
            return true;
        default:
            return false;
    }
}

bool Token::is_arithmetic_op() const noexcept {
    switch (type) {
        case TokenType::Plus: case TokenType::Minus: case TokenType::Star:
        case TokenType::Slash: case TokenType::Percent:
            return true;
        default:
            return false;
    }
}

bool Token::is_comparison_op() const noexcept {
    switch (type) {
        case TokenType::EqualEqual: case TokenType::BangEqual:
        case TokenType::Less: case TokenType::LessEqual:
        case TokenType::Greater: case TokenType::GreaterEqual:
            return true;
        default:
            return false;
    }
}

bool Token::is_assignment_op() const noexcept {
    switch (type) {
        case TokenType::Equal: case TokenType::PlusEqual: case TokenType::MinusEqual:
        case TokenType::StarEqual: case TokenType::SlashEqual: case TokenType::PercentEqual:
            return true;
        default:
            return false;
    }
}
bool Token::is_user_type() const noexcept {
    // User-defined types are identifiers that aren't keywords
    // The parser tracks these separately via user_defined_types_
    return type == TokenType::Identifier;
}


std::string_view Token::type_name() const noexcept {
    switch (type) {
        case TokenType::EndOfFile: return "EOF";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::IntegerLiteral: return "INTEGER";
        case TokenType::FloatLiteral: return "FLOAT";
        case TokenType::StringLiteral: return "STRING";
        case TokenType::CharLiteral: return "CHAR";
        case TokenType::BoolLiteral: return "BOOL";
        case TokenType::Plus: return "+";
        case TokenType::Minus: return "-";
        case TokenType::Star: return "*";
        case TokenType::Slash: return "/";
        case TokenType::Percent: return "%";
        case TokenType::Bang: return "!";
        case TokenType::Equal: return "=";
        case TokenType::Less: return "<";
        case TokenType::Greater: return ">";
        case TokenType::Ampersand: return "&";
        case TokenType::Pipe: return "|";
        case TokenType::Caret: return "^";
        case TokenType::Tilde: return "~";
        case TokenType::PlusEqual: return "+=";
        case TokenType::MinusEqual: return "-=";
        case TokenType::StarEqual: return "*=";
        case TokenType::SlashEqual: return "/=";
        case TokenType::PercentEqual: return "%=";
        case TokenType::EqualEqual: return "==";
        case TokenType::BangEqual: return "!=";
        case TokenType::LessEqual: return "<=";
        case TokenType::GreaterEqual: return ">=";
        case TokenType::AmpersandAmpersand: return "&&";
        case TokenType::PipePipe: return "||";
        case TokenType::PlusPlus: return "++";
        case TokenType::MinusMinus: return "--";
        case TokenType::Arrow: return "->";
        case TokenType::LParen: return "(";
        case TokenType::RParen: return ")";
        case TokenType::LBrace: return "{";
        case TokenType::RBrace: return "}";
        case TokenType::LBracket: return "[";
        case TokenType::RBracket: return "]";
        case TokenType::Semicolon: return ";";
        case TokenType::Comma: return ",";
        case TokenType::Dot: return ".";
        case TokenType::Colon: return ":";
        case TokenType::Question: return "?";
        case TokenType::KwBool: return "bool";
        case TokenType::KwAuto: return "auto";
        case TokenType::KwClass: return "class";
        case TokenType::KwBreak: return "break";
        case TokenType::KwCase: return "case";
        case TokenType::KwChar: return "char";
        case TokenType::KwConst: return "const";
        case TokenType::KwContinue: return "continue";
        case TokenType::KwDefault: return "default";
        case TokenType::KwDo: return "do";
        case TokenType::KwDouble: return "double";
        case TokenType::KwElse: return "else";
        case TokenType::KwEnum: return "enum";
        case TokenType::KwExtern: return "extern";
        case TokenType::KwFloat: return "float";
        case TokenType::KwFor: return "for";
        case TokenType::KwGoto: return "goto";
        case TokenType::KwIf: return "if";
        case TokenType::KwInline: return "inline";
        case TokenType::KwInt: return "int";
        case TokenType::KwLong: return "long";
        case TokenType::KwRegister: return "register";
        case TokenType::KwReturn: return "return";
        case TokenType::KwShort: return "short";
        case TokenType::KwSigned: return "signed";
        case TokenType::KwSizeof: return "sizeof";
        case TokenType::KwStatic: return "static";
        case TokenType::KwStruct: return "struct";
        case TokenType::KwSwitch: return "switch";
        case TokenType::KwTypedef: return "typedef";
        case TokenType::KwUnion: return "union";
        case TokenType::KwUnsigned: return "unsigned";
        case TokenType::KwVoid: return "void";
        case TokenType::KwVolatile: return "volatile";
        case TokenType::KwWhile: return "while";
        case TokenType::KwTrue: return "true";
        case TokenType::KwFalse: return "false";
        case TokenType::KwPublic: return "public";
        case TokenType::KwPrivate: return "private";
        case TokenType::KwProtected: return "protected";
        case TokenType::Hash: return "#";
        case TokenType::HashInclude: return "#include";
        case TokenType::HashDefine: return "#define";
        case TokenType::HashIf: return "#if";
        case TokenType::HashIfdef: return "#ifdef";
        case TokenType::HashIfndef: return "#ifndef";
        case TokenType::HashElse: return "#else";
        case TokenType::HashElif: return "#elif";
        case TokenType::HashEndif: return "#endif";
        case TokenType::HashPragma: return "#pragma";
    }
    return "UNKNOWN";
}

} // namespace compiler
