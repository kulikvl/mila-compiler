#include "Token.hpp"

std::ostream& operator<<(std::ostream& os, TokenType tokenType) noexcept {
    switch (tokenType) {
        case TokenType::EOI:
            return os << "EOI";
        case TokenType::IDENTIFIER:
            return os << "IDENTIFIER";
        case TokenType::INTEGER_LITERAL:
            return os << "INTEGER_LITERAL";
        case TokenType::REAL_LITERAL:
            return os << "REAL_LITERAL";
        case TokenType::PLUS:
            return os << "PLUS";
        case TokenType::MINUS:
            return os << "MINUS";
        case TokenType::MULTIPLY:
            return os << "MULTIPLY";
        case TokenType::DIVIDE:
            return os << "DIVIDE";
        case TokenType::EQUAL:
            return os << "EQUAL";
        case TokenType::LESS:
            return os << "LESS";
        case TokenType::GREATER:
            return os << "GREATER";
        case TokenType::NOT_EQUAL:
            return os << "NOT_EQUAL";
        case TokenType::LESS_EQUAL:
            return os << "LESS_EQUAL";
        case TokenType::GREATER_EQUAL:
            return os << "GREATER_EQUAL";
        case TokenType::ASSIGN:
            return os << "ASSIGN";
        case TokenType::SEMICOLON:
            return os << "SEMICOLON";
        case TokenType::COLON:
            return os << "COLON";
        case TokenType::COMMA:
            return os << "COMMA";
        case TokenType::DOT:
            return os << "DOT";
        case TokenType::DOUBLE_DOT:
            return os << "DOUBLE_DOT";
        case TokenType::LEFT_PAREN:
            return os << "LEFT_PAREN";
        case TokenType::RIGHT_PAREN:
            return os << "RIGHT_PAREN";
        case TokenType::LEFT_BRACKET:
            return os << "LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET:
            return os << "RIGHT_BRACKET";
        case TokenType::INTEGER:
            return os << "INTEGER";
        case TokenType::REAL:
            return os << "REAL";
        case TokenType::ARRAY:
            return os << "ARRAY";
        case TokenType::OF:
            return os << "OF";
        case TokenType::PROGRAM:
            return os << "PROGRAM";
        case TokenType::VAR:
            return os << "VAR";
        case TokenType::CONST:
            return os << "CONST";
        case TokenType::BEGIN:
            return os << "BEGIN";
        case TokenType::END:
            return os << "END";
        case TokenType::FUNCTION:
            return os << "FUNCTION";
        case TokenType::PROCEDURE:
            return os << "PROCEDURE";
        case TokenType::IF:
            return os << "IF";
        case TokenType::THEN:
            return os << "THEN";
        case TokenType::ELSE:
            return os << "ELSE";
        case TokenType::WHILE:
            return os << "WHILE";
        case TokenType::FOR:
            return os << "FOR";
        case TokenType::DO:
            return os << "DO";
        case TokenType::TO:
            return os << "TO";
        case TokenType::DOWNTO:
            return os << "DOWNTO";
        case TokenType::EXIT:
            return os << "EXIT";
        case TokenType::BREAK:
            return os << "BREAK";
        case TokenType::FORWARD:
            return os << "FORWARD";
        case TokenType::OR:
            return os << "OR";
        case TokenType::NOT:
            return os << "NOT";
        case TokenType::AND:
            return os << "AND";
        case TokenType::MOD:
            return os << "MOD";
        case TokenType::DIV:
            return os << "DIV";
        default:
            return os << "UNKNOWN_TOKEN_TYPE" << std::endl;
    }
}

Token::Token(TokenType tokenType, const Position& position, const std::optional<TokenValue>& value)
    : type(tokenType), position(position), value(value) {}

Position Token::getPosition() const {
    return position;
}

TokenType Token::getType() const {
    return type;
}

std::optional<TokenValue> Token::getValue() const {
    return value;
}

struct PrintVisitor {
    std::ostream& os;

    explicit PrintVisitor(std::ostream& os) : os(os) {}

    void operator()(int i) const { os << "int: " << i; }

    void operator()(double f) const { os << "double: " << f; }

    void operator()(const std::string& s) const { os << "string: " << s; }
};

std::ostream& operator<<(std::ostream& os, const TokenValue& tokVal) noexcept {
    std::visit(PrintVisitor(os), tokVal);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Token& token) noexcept {
    os << "(Token " << token.getType();
    if (token.getValue()) {
        os << " ";
        os << *token.getValue();
    }
    return os << ")";
}
