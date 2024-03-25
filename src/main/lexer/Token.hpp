#pragma once
#include <iostream>
#include <optional>
#include <variant>
#include "Position.hpp"

enum class TokenType {
    // end of file
    EOI,  // EOF

    // identifier
    IDENTIFIER,  // my_var

    // literals
    INTEGER_LITERAL,  // 55
    REAL_LITERAL,     // 3.14

    // operators
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    EQUAL,          // =
    LESS,           // <
    GREATER,        // >
    NOT_EQUAL,      // <>
    LESS_EQUAL,     // <=
    GREATER_EQUAL,  // >=
    ASSIGN,         // :=

    // separators (punctuation)
    SEMICOLON,       // ;
    COLON,           // :
    COMMA,           // ,
    DOT,             // .
    DOUBLE_DOT,      // ..
    LEFT_PAREN,      // (
    RIGHT_PAREN,     // )
    LEFT_BRACKET,   // [
    RIGHT_BRACKET,  // ]

    // keywords
    INTEGER,    // integer
    REAL,       // real
    ARRAY,      // array
    OF,         // of
    PROGRAM,    // program
    VAR,        // var
    CONST,      // const
    BEGIN,      // begin
    END,        // end
    FUNCTION,   // function
    PROCEDURE,  // procedure
    FORWARD,    // forward
    IF,         // if
    THEN,       // then
    ELSE,       // else
    WHILE,      // while
    DO,         // do
    FOR,        // for
    TO,         // to
    DOWNTO,     // downto
    EXIT,       // exit
    BREAK,      // break

    // keywords that act as operators
    OR,   // or
    NOT,  // not
    AND,  // and
    MOD,  // mod
    DIV,  // div
};

std::ostream& operator<<(std::ostream& os, TokenType tokenType) noexcept;

using TokenValue = std::variant<int, double, std::string>;
std::ostream& operator<<(std::ostream& os, const TokenValue& tokVal) noexcept;

class Token {
    TokenType type;

    /**
     * @brief Position in the source code where the token starts
     */
    Position position;

    std::optional<TokenValue> value;

   public:
    Token(TokenType type, const Position& position,
          const std::optional<TokenValue>& value = std::nullopt);

    [[nodiscard]] TokenType getType() const;
    [[nodiscard]] Position getPosition() const;
    [[nodiscard]] std::optional<TokenValue> getValue() const;
};

std::ostream& operator<<(std::ostream& os, const Token& token) noexcept;
