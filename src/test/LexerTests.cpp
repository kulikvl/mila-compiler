#include <gtest/gtest.h>
#include <sstream>
#include "lexer/Lexer.hpp"

TEST(LexerTests, HandlesIdentifiers) {
    std::istringstream input("MyVar _MY__VAR_ my_var123");
    Lexer lexer(input);

    auto tk1 = lexer.match(TokenType::IDENTIFIER);
    ASSERT_TRUE(tk1.has_value());
    ASSERT_EQ("MyVar", std::get<std::string>(tk1.value().getValue().value()));

    auto tk2 = lexer.match(TokenType::IDENTIFIER);
    ASSERT_TRUE(tk2.has_value());
    ASSERT_EQ("_MY__VAR_", std::get<std::string>(tk2.value().getValue().value()));

    auto tk3 = lexer.match(TokenType::IDENTIFIER);
    ASSERT_TRUE(tk3.has_value());
    ASSERT_EQ("my_var123", std::get<std::string>(tk3.value().getValue().value()));
}

TEST(LexerTests, HandlesOperators) {
    std::istringstream input("+ - * / = < > <> <= >= :=");
    Lexer lexer(input);

    std::vector<std::optional<Token>> tokens = {
        lexer.match(TokenType::PLUS),          lexer.match(TokenType::MINUS),     lexer.match(TokenType::MULTIPLY),
        lexer.match(TokenType::DIVIDE),        lexer.match(TokenType::EQUAL),     lexer.match(TokenType::LESS),
        lexer.match(TokenType::GREATER),       lexer.match(TokenType::NOT_EQUAL), lexer.match(TokenType::LESS_EQUAL),
        lexer.match(TokenType::GREATER_EQUAL), lexer.match(TokenType::ASSIGN),
    };

    for (const auto& token : tokens) {
        ASSERT_TRUE(token.has_value());
        ASSERT_EQ(std::nullopt, token.value().getValue());
    }
}

TEST(LexerTests, HandlesSeparators) {
    std::istringstream input("; :, ... () []");
    Lexer lexer(input);

    std::vector<std::optional<Token>> tokens = {
        lexer.match(TokenType::SEMICOLON),     lexer.match(TokenType::COLON),
        lexer.match(TokenType::COMMA),         lexer.match(TokenType::DOUBLE_DOT),
        lexer.match(TokenType::DOT),           lexer.match(TokenType::LEFT_PAREN),
        lexer.match(TokenType::RIGHT_PAREN),   lexer.match(TokenType::LEFT_BRACKET),
        lexer.match(TokenType::RIGHT_BRACKET),
    };

    for (const auto& token : tokens) {
        ASSERT_TRUE(token.has_value());
        ASSERT_EQ(std::nullopt, token.value().getValue());
    }
}

TEST(LexerTests, HandlesKeywords) {
    std::istringstream input(
        "integer real program var const begin end array function procedure if then else while for do to downto exit "
        "break "
        "forward of or not and mod div");
    Lexer lexer(input);

    std::vector<std::optional<Token>> tokens = {
        lexer.match(TokenType::INTEGER),   lexer.match(TokenType::REAL),  lexer.match(TokenType::PROGRAM),
        lexer.match(TokenType::VAR),       lexer.match(TokenType::CONST), lexer.match(TokenType::BEGIN),
        lexer.match(TokenType::END),       lexer.match(TokenType::ARRAY), lexer.match(TokenType::FUNCTION),
        lexer.match(TokenType::PROCEDURE), lexer.match(TokenType::IF),    lexer.match(TokenType::THEN),
        lexer.match(TokenType::ELSE),      lexer.match(TokenType::WHILE), lexer.match(TokenType::FOR),
        lexer.match(TokenType::DO),        lexer.match(TokenType::TO),    lexer.match(TokenType::DOWNTO),
        lexer.match(TokenType::EXIT),      lexer.match(TokenType::BREAK), lexer.match(TokenType::FORWARD),
        lexer.match(TokenType::OF),        lexer.match(TokenType::OR),    lexer.match(TokenType::NOT),
        lexer.match(TokenType::AND),       lexer.match(TokenType::MOD),   lexer.match(TokenType::DIV),
    };

    for (const auto& token : tokens) {
        ASSERT_TRUE(token.has_value());
        ASSERT_EQ(std::nullopt, token.value().getValue());
    }
}

TEST(LexerTests, ThrowsOnInvalidInput) {
    std::istringstream input("?");
    ASSERT_THROW(Lexer lexer(input), LexerException);
}

TEST(LexerTests, HandlesComments) {
    std::istringstream input("my_var { this is a comment }=123");
    Lexer lexer(input);

    std::vector<std::optional<Token>> tokens = {
        lexer.match(TokenType::IDENTIFIER),
        lexer.match(TokenType::EQUAL),
        lexer.match(TokenType::INTEGER_LITERAL),
        lexer.match(TokenType::EOI),
    };

    for (const auto& token : tokens) {
        ASSERT_TRUE(token.has_value());
    }
}

TEST(LexerTests, HandlesArithmeticExpression) {
    std::istringstream input("8230 +\n 0099");
    Lexer lexer(input);

    auto tk1 = lexer.match(TokenType::INTEGER_LITERAL);
    ASSERT_TRUE(tk1.has_value());
    ASSERT_EQ(8230, std::get<int>(tk1.value().getValue().value()));
    ASSERT_EQ(TokenType::INTEGER_LITERAL, tk1.value().getType());
    ASSERT_EQ(1, tk1.value().getPosition().getCol());
    ASSERT_EQ(1, tk1.value().getPosition().getLine());

    auto tk2 = lexer.match(TokenType::PLUS);
    ASSERT_TRUE(tk2.has_value());
    ASSERT_EQ(std::nullopt, tk2.value().getValue());
    ASSERT_EQ(TokenType::PLUS, tk2.value().getType());
    ASSERT_EQ(6, tk2.value().getPosition().getCol());
    ASSERT_EQ(1, tk2.value().getPosition().getLine());

    auto tk3 = lexer.match(TokenType::INTEGER_LITERAL);
    ASSERT_TRUE(tk3.has_value());
    ASSERT_EQ(99, std::get<int>(tk3.value().getValue().value()));
    ASSERT_EQ(TokenType::INTEGER_LITERAL, tk3.value().getType());
    ASSERT_EQ(2, tk3.value().getPosition().getCol());
    ASSERT_EQ(2, tk3.value().getPosition().getLine());

    // from now on, the lexer should always return EOI

    auto tk4 = lexer.match(TokenType::EOI);
    ASSERT_TRUE(tk4.has_value());
    ASSERT_EQ(std::nullopt, tk4.value().getValue());
    ASSERT_EQ(TokenType::EOI, tk4.value().getType());
    ASSERT_EQ(6, tk4.value().getPosition().getCol());
    ASSERT_EQ(2, tk4.value().getPosition().getLine());

    auto tk5 = lexer.match(TokenType::EOI);
    ASSERT_TRUE(tk5.has_value());
    ASSERT_EQ(TokenType::EOI, tk5.value().getType());
}

TEST(LexerTests, HandlesNumbers) {
    std::istringstream input("123.456 0.99 &1234 $a9f8e &0000");
    Lexer lexer(input);

    auto tk1 = lexer.match(TokenType::REAL_LITERAL);
    ASSERT_TRUE(tk1.has_value());
    EXPECT_EQ(123.456, std::get<double>(tk1.value().getValue().value()));
    EXPECT_EQ(TokenType::REAL_LITERAL, tk1.value().getType());

    auto tk2 = lexer.match(TokenType::REAL_LITERAL);
    ASSERT_TRUE(tk2.has_value());
    EXPECT_EQ(0.99, std::get<double>(tk2.value().getValue().value()));
    EXPECT_EQ(TokenType::REAL_LITERAL, tk2.value().getType());

    auto tk3 = lexer.match(TokenType::INTEGER_LITERAL);
    ASSERT_TRUE(tk3.has_value());
    EXPECT_EQ(668, std::get<int>(tk3.value().getValue().value()));
    EXPECT_EQ(TokenType::INTEGER_LITERAL, tk3.value().getType());

    auto tk4 = lexer.match(TokenType::INTEGER_LITERAL);
    ASSERT_TRUE(tk4.has_value());
    EXPECT_EQ(696206, std::get<int>(tk4.value().getValue().value()));
    EXPECT_EQ(TokenType::INTEGER_LITERAL, tk4.value().getType());

    auto tk5 = lexer.match(TokenType::INTEGER_LITERAL);
    ASSERT_TRUE(tk5.has_value());
    EXPECT_EQ(0, std::get<int>(tk5.value().getValue().value()));
    EXPECT_EQ(TokenType::INTEGER_LITERAL, tk5.value().getType());
}
