#include "Position.hpp"

void Position::advance() {
    col++;
}

void Position::nextLine() {
    line++;
    col = 1;
}

unsigned Position::getLine() const {
    return line;
}

unsigned Position::getCol() const {
    return col;
}

std::ostream& operator<<(std::ostream& os, const Position& position) noexcept {
    return os << position.getLine() << ':' << position.getCol();
}
