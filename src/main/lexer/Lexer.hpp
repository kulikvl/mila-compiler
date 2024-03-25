#pragma once
#include "Token.hpp"

class Lexer {
    /**
     * @brief Input stream with source code
     */
    std::istream& is;

    /**
     * @brief Current position in the source code
     */
    Position curPos;

    /**
     * @brief Position where currently scanning token starts
     */
    Position tokenStartPos;

    Token nextToken;

    /**
     * @brief Eats next token
     */
    Token readNextToken();

   public:
    explicit Lexer(std::istream& is);

    /**
     * @brief Matches the next token with the given token type. If successful, the token is consumed
     * @return The matched token or std::nullopt if the token does not match the given token type.
     */
    std::optional<Token> match(TokenType tokenType);

    /**
     * @return Next token without consuming it
     */
    [[nodiscard]] Token peek() const;
};

class LexerException : public std::exception {
   private:
    std::string message;

   public:
    LexerException(const std::string& message, const Position& position);

    [[nodiscard]] const char* what() const noexcept override;
};
