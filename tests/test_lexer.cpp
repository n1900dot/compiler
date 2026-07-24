#include <catch2/catch_test_macros.hpp>
#include "lexer/lexer.hpp"

using namespace compiler;

TEST_CASE("Lexer tokenizes simple identifiers", "[lexer]") {
    Lexer lexer("int main");
    auto result = lexer.tokenize();
    REQUIRE(result);
    REQUIRE(result->size() == 3);

    CHECK((*result)[0].type == TokenType::KwInt);
    CHECK((*result)[1].type == TokenType::Identifier);
    CHECK((*result)[2].type == TokenType::EndOfFile);
}

TEST_CASE("Lexer tokenizes arithmetic operators", "[lexer]") {
    Lexer lexer("a + b * c - d / e");
    auto result = lexer.tokenize();
    REQUIRE(result);

    auto& tokens = *result;
    CHECK(tokens[0].type == TokenType::Identifier);
    CHECK(tokens[1].type == TokenType::Plus);
    CHECK(tokens[3].type == TokenType::Star);
    CHECK(tokens[5].type == TokenType::Minus);
    CHECK(tokens[7].type == TokenType::Slash);
}

TEST_CASE("Lexer tokenizes numbers", "[lexer]") {
    Lexer lexer("42 3.14 1e10");
    auto result = lexer.tokenize();
    REQUIRE(result);

    CHECK((*result)[0].type == TokenType::IntegerLiteral);
    CHECK((*result)[1].type == TokenType::FloatLiteral);
    CHECK((*result)[2].type == TokenType::FloatLiteral);
}

TEST_CASE("Lexer tokenizes strings", "[lexer]") {
    Lexer lexer(R"("hello world")");
    auto result = lexer.tokenize();
    REQUIRE(result);
    CHECK((*result)[0].type == TokenType::StringLiteral);
}

TEST_CASE("Lexer handles comments", "[lexer]") {
    Lexer lexer("int x; // comment\nint y;");
    auto result = lexer.tokenize();
    REQUIRE(result);

    auto& tokens = *result;
    CHECK(tokens[0].type == TokenType::KwInt);
    CHECK(tokens[1].type == TokenType::Identifier);
    CHECK(tokens[2].type == TokenType::Semicolon);
    CHECK(tokens[3].type == TokenType::KwInt);
}

TEST_CASE("Lexer reports unterminated string", "[lexer]") {
    Lexer lexer("\"unterminated");
    auto result = lexer.tokenize();
    REQUIRE(!result);
    CHECK(result.error().code == ErrorCode::UnterminatedString);
}

TEST_CASE("Lexer tracks source locations", "[lexer]") {
    Lexer lexer("int\nx");
    auto result = lexer.tokenize();
    REQUIRE(result);

    CHECK((*result)[0].location.line == 1);
    CHECK((*result)[1].location.line == 2);
}
