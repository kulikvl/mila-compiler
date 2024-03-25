#include "Lexer.hpp"
#include <map>
#include <sstream>

Lexer::Lexer(std::istream& is) : is(is), curPos(), tokenStartPos(), nextToken(readNextToken()) {}

Token Lexer::peek() const {
    return nextToken;
}

std::optional<Token> Lexer::match(TokenType tokenType) {
    if (peek().getType() == tokenType) {
        auto curToken = nextToken;
        nextToken = readNextToken();
        return curToken;
    }

    return std::nullopt;
}

std::map<std::string, TokenType> keywords = {
    {"integer", TokenType::INTEGER},
    {"real", TokenType::REAL},
    {"program", TokenType::PROGRAM},
    {"var", TokenType::VAR},
    {"const", TokenType::CONST},
    {"begin", TokenType::BEGIN},
    {"end", TokenType::END},
    {"array", TokenType::ARRAY},
    {"function", TokenType::FUNCTION},
    {"procedure", TokenType::PROCEDURE},
    {"if", TokenType::IF},
    {"then", TokenType::THEN},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"do", TokenType::DO},
    {"to", TokenType::TO},
    {"downto", TokenType::DOWNTO},
    {"exit", TokenType::EXIT},
    {"break", TokenType::BREAK},
    {"forward", TokenType::FORWARD},
    {"of", TokenType::OF},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
    {"and", TokenType::AND},
    {"mod", TokenType::MOD},
    {"div", TokenType::DIV},
};

std::optional<TokenType> isKeyword(const std::string& identifier) {
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        return it->second;
    }

    return std::nullopt;
}

Token Lexer::readNextToken() {
    std::string identifierBuffer;
    int intNumberBuffer = 0;
    double doubleNumberBuffer = 0.0;
    int dividerBuffer = 10;

qStart:
    if (is.peek() == EOF) {
        return {TokenType::EOI, curPos};
    }

    if (std::isspace(is.peek())) {

        if (is.peek() == '\n') {
            curPos.nextLine();
        } else {
            curPos.advance();
        }
        is.get();

        goto qStart;
    }

    switch (is.peek()) {
        case '{':
            is.get();
            goto qComment;
        case '+':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::PLUS, tokenStartPos};
        case '-':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::MINUS, tokenStartPos};
        case '*':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::MULTIPLY, tokenStartPos};
        case '/':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::DIVIDE, tokenStartPos};
        case '=':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::EQUAL, tokenStartPos};
        case '<':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            goto qLess;
        case '>':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            goto qGreater;
        case ':':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            goto qColon;
        case ';':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::SEMICOLON, tokenStartPos};
        case ',':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::COMMA, tokenStartPos};
        case '.':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            goto qDot;
        case '(':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::LEFT_PAREN, tokenStartPos};
        case ')':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::RIGHT_PAREN, tokenStartPos};
        case '[':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::LEFT_BRACKET, tokenStartPos};
        case ']':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            return {TokenType::RIGHT_BRACKET, tokenStartPos};
        case '&':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            goto qInt8;
        case '$':
            is.get();
            tokenStartPos = curPos;
            curPos.advance();
            goto qInt16;
        default:;
    }

    if (std::isalpha(is.peek()) || is.peek() == '_') {
        identifierBuffer.clear();
        identifierBuffer.push_back((char)is.peek());
        is.get();
        tokenStartPos = curPos;
        curPos.advance();
        goto qIdentifier;
    }

    if (std::isdigit(is.peek())) {
        intNumberBuffer = is.get() - '0';
        tokenStartPos = curPos;
        curPos.advance();
        goto qInt10;
    }

    throw LexerException("Unable to lex next token.", curPos);

qComment:
    switch (is.peek()) {
        case '}':
            is.get();
            goto qStart;
        case EOF:
            throw LexerException("Unexpected end of file in a comment.", curPos);
        default:
            is.get();
            goto qComment;
    }

qLess:
    switch (is.peek()) {
        case '>':
            is.get();
            curPos.advance();
            return {TokenType::NOT_EQUAL, tokenStartPos};
        case '=':
            is.get();
            curPos.advance();
            return {TokenType::LESS_EQUAL, tokenStartPos};
        default:
            return {TokenType::LESS, tokenStartPos};
    }

qGreater:
    switch (is.peek()) {
        case '=':
            is.get();
            curPos.advance();
            return {TokenType::GREATER_EQUAL, tokenStartPos};
        default:
            return {TokenType::GREATER, tokenStartPos};
    }

qColon:
    switch (is.peek()) {
        case '=':
            is.get();
            curPos.advance();
            return {TokenType::ASSIGN, tokenStartPos};
        default:
            return {TokenType::COLON, tokenStartPos};
    }

qDot:
    switch (is.peek()) {
        case '.':
            is.get();
            curPos.advance();
            return {TokenType::DOUBLE_DOT, tokenStartPos};
        default:
            return {TokenType::DOT, tokenStartPos};
    }

qIdentifier:
    if (std::isdigit(is.peek()) || std::isalpha(is.peek()) || is.peek() == '_') {
        identifierBuffer.push_back((char)is.peek());
        is.get();
        curPos.advance();
        goto qIdentifier;
    } else {
        auto maybeKeyword = isKeyword(identifierBuffer);
        if (maybeKeyword.has_value()) {
            return {maybeKeyword.value(), tokenStartPos};
        } else {
            return {TokenType::IDENTIFIER, tokenStartPos, identifierBuffer};
        }
    }

qInt10:
    if (std::isdigit(is.peek())) {
        intNumberBuffer = intNumberBuffer * 10 + is.get() - '0';
        curPos.advance();
        goto qInt10;
    } else if (is.peek() == '.') {
        is.get();
        curPos.advance();

        // there must be at least one digit after the dot
        if (!std::isdigit(is.peek()))
            throw LexerException("Expected a digit after the dot in a real number.", curPos);

        doubleNumberBuffer = intNumberBuffer;
        goto qDouble;
    } else {
        return {TokenType::INTEGER_LITERAL, tokenStartPos, intNumberBuffer};
    }

qInt8:
    if (std::isdigit(is.peek())) {
        if (is.peek() > '7')
            throw LexerException("Invalid octal digit.", curPos);

        intNumberBuffer = intNumberBuffer * 8 + is.get() - '0';
        curPos.advance();
        goto qInt8;
    } else {
        return {TokenType::INTEGER_LITERAL, tokenStartPos, intNumberBuffer};
    }

qInt16:
    if (std::isdigit(is.peek()) || std::isalpha(is.peek())) {
        if (std::isalpha(is.peek()) && !(is.peek() >= 'a' && is.peek() <= 'f'))
            throw LexerException("Invalid hex digit: " + std::to_string(is.peek()), curPos);

        int digit = (std::isalpha(is.peek())) ? (is.get() - 'a' + 10) : (is.get() - '0');
        intNumberBuffer = intNumberBuffer * 16 + digit;
        curPos.advance();
        goto qInt16;
    } else {
        return {TokenType::INTEGER_LITERAL, tokenStartPos, intNumberBuffer};
    }

qDouble:
    if (std::isdigit(is.peek())) {
        auto digit = (double)(is.get() - '0');
        doubleNumberBuffer += digit / dividerBuffer;
        dividerBuffer *= 10;
        curPos.advance();
        goto qDouble;
    } else {
        return {TokenType::REAL_LITERAL, tokenStartPos, doubleNumberBuffer};
    }
}

LexerException::LexerException(const std::string& message, const Position& position) {
    std::ostringstream oss;
    oss << "Lexer Error at [" << position << "] - " << message;
    this->message = oss.str();
}

const char* LexerException::what() const noexcept {
    return message.c_str();
}
