#include <format>
#include <print>
#include <fstream>
#include <sstream>
#include <string>
#include <span>
#include <vector>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/analyzer.hpp"
#include "preprocessor/preprocessor.hpp"
#include "parser/ast_printer.hpp"

namespace compiler {

[[nodiscard]] Result<std::string> read_file(std::string_view filename) {
    std::ifstream file{std::string(filename)};
    if (!file) {
        return std::unexpected(CompilerError{
            .code = ErrorCode::IOError,
            .message = std::format("Could not open file: {}", filename),
            .location = SourceLocation{},
            .notes = {}
        });
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

[[nodiscard]] Result<void> compile(std::string_view source, std::string_view filename) {
    // Phase 1: Preprocessing
    Preprocessor preprocessor;
    auto preprocessed = preprocessor.process(source, filename);
    if (!preprocessed) return std::unexpected(preprocessed.error());

    // Phase 2: Lexing
    Lexer lexer(*preprocessed);
    auto tokens = lexer.tokenize();
    if (!tokens) return std::unexpected(tokens.error());

    // Phase 3: Parsing
    Parser parser(*tokens);
    auto ast = parser.parse();
    if (!ast) return std::unexpected(ast.error());

    // Phase 4: Semantic Analysis
    SemanticAnalyzer analyzer;
    auto sem_result = analyzer.analyze(*ast);
    if (!sem_result) return std::unexpected(sem_result.error());

    // Print AST (for debugging)
    ASTPrinter printer;
    std::println("{}", printer.print(*ast));

    return {};
}

} // namespace compiler

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::println(stderr, "Usage: {} <source_file>", argv[0]);
        return 1;
    }

    auto source = compiler::read_file(argv[1]);
    if (!source) {
        std::println(stderr, "Error: {}", source.error().to_string());
        return 1;
    }

    auto result = compiler::compile(*source, argv[1]);
    if (!result) {
        std::println(stderr, "Error: {}", result.error().to_string());
        return 1;
    }

    std::println("Compilation successful!");
    return 0;
}
