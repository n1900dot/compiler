#include "error.hpp"
#include <format>

namespace compiler {

std::string CompilerError::to_string() const {
    auto result = std::format("[{}] Error at {}: {}",
                              code_string(),
                              location.to_string(),
                              message);
    for (const auto& note : notes) {
        result += std::format("\n  note: {}", note);
    }
    return result;
}

std::string CompilerError::code_string() const {
    switch (code) {
        case ErrorCode::UnexpectedCharacter:    return "E0001";
        case ErrorCode::UnterminatedString:     return "E0002";
        case ErrorCode::UnterminatedComment:    return "E0003";
        case ErrorCode::InvalidNumberLiteral:   return "E0004";
        case ErrorCode::UnexpectedToken:        return "E0101";
        case ErrorCode::ExpectedToken:          return "E0102";
        case ErrorCode::MissingSemicolon:       return "E0103";
        case ErrorCode::MissingClosingParen:    return "E0104";
        case ErrorCode::MissingClosingBrace:    return "E0105";
        case ErrorCode::InvalidExpression:      return "E0106";
        case ErrorCode::UndefinedSymbol:        return "E0201";
        case ErrorCode::RedefinedSymbol:        return "E0202";
        case ErrorCode::TypeMismatch:           return "E0203";
        case ErrorCode::InvalidOperation:       return "E0204";
        case ErrorCode::WrongNumberOfArguments: return "E0205";
        case ErrorCode::MissingReturnStatement: return "E0206";
        case ErrorCode::UnreachableCode:        return "E0207";
        case ErrorCode::IncludeNotFound:        return "E0301";
        case ErrorCode::CircularInclude:        return "E0302";
        case ErrorCode::InvalidDirective:       return "E0303";
        case ErrorCode::IOError:                return "E0401";
        case ErrorCode::UnknownError:           return "E9999";
    }
    return "E????";
}

} // namespace compiler
