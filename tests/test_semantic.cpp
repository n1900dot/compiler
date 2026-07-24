#include <catch2/catch_test_macros.hpp>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/analyzer.hpp"

using namespace compiler;

TEST_CASE("Semantic analyzer accepts valid program", "[semantic]") {
    Lexer lexer("int main() { return 0; }");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);

    SemanticAnalyzer analyzer;
    auto result = analyzer.analyze(*ast);
    REQUIRE(result);
}

TEST_CASE("Semantic analyzer detects undefined variable", "[semantic]") {
    Lexer lexer("int main() { return x; }");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);

    SemanticAnalyzer analyzer;
    auto result = analyzer.analyze(*ast);
    REQUIRE(!result);
    CHECK(result.error().code == ErrorCode::UndefinedSymbol);
}

TEST_CASE("Semantic analyzer detects type mismatch", "[semantic]") {
    Lexer lexer("int main() { int x = 3.14; return 0; }");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);

    SemanticAnalyzer analyzer;
    auto result = analyzer.analyze(*ast);
    REQUIRE(!result);
    CHECK(result.error().code == ErrorCode::TypeMismatch);
}

TEST_CASE("Semantic analyzer detects break outside loop", "[semantic]") {
    Lexer lexer("int main() { break; return 0; }");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);

    SemanticAnalyzer analyzer;
    auto result = analyzer.analyze(*ast);
    REQUIRE(!result);
    CHECK(result.error().code == ErrorCode::InvalidOperation);
}
