#pragma once
#include <iostream>

/**
 * @brief Represents a position in the source code
 */
class Position {
    unsigned line = 1;
    unsigned col = 1;

   public:
    /**
     * @brief Advances the column by one
     */
    void advance();

    /**
     * @brief Advances the line by one and resets the column to 1
     */
    void nextLine();

    [[nodiscard]] unsigned getLine() const;

    [[nodiscard]] unsigned getCol() const;
};

std::ostream& operator<<(std::ostream& os, const Position& position) noexcept;
