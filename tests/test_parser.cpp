#include <catch2/catch_test_macros.hpp>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

using namespace compiler;

TEST_CASE("Parser parses variable declaration", "[parser]") {
    Lexer lexer("int x = 42;");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);
    REQUIRE(ast->declarations.size() == 1);
}

TEST_CASE("Parser parses function declaration", "[parser]") {
    Lexer lexer("int add(int a, int b) { return a + b; }");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);
    REQUIRE(ast->declarations.size() == 1);
}

TEST_CASE("Parser parses if statement", "[parser]") {
    Lexer lexer("int main() { if (1) { return 0; } else { return 1; } }");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);
}

TEST_CASE("Parser parses while loop", "[parser]") {
    Lexer lexer("int main() { while (1) { break; } }");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);
}

TEST_CASE("Parser detects missing semicolon", "[parser]") {
    Lexer lexer("int x");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(!ast);
}

TEST_CASE("Parser parses binary expressions with precedence", "[parser]") {
    Lexer lexer("int main() { return 1 + 2 * 3; }");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens);

    Parser parser(*tokens);
    auto ast = parser.parse();
    REQUIRE(ast);
}
