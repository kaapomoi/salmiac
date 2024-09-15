/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_BOARD_H
#define SALMIAC_BOARD_H

#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

namespace bbm {

struct Cell {
    /// empty is ground, box is breakable, walls are indestructible.
    enum class Type : std::uint8_t { empty, box, wall };
    Type type{Type::empty};
};

class Board {
public:
    Board() noexcept;

    explicit Board(std::string const& filename) noexcept;

    Board(std::size_t board_h, std::size_t board_w) noexcept;

    void print_board() const noexcept;

    [[nodiscard]] std::string serialize_for_network() const noexcept;

    void deserialize_from_network(std::string const& data) noexcept;

    void reset() noexcept;

    [[nodiscard]] std::optional<std::pair<std::size_t, std::size_t>> size() const noexcept;

    [[nodiscard]] Cell::Type at(std::size_t const x, std::size_t const y) const noexcept;

    [[nodiscard]] Cell& at_mut(std::size_t const x, std::size_t const y) noexcept;

    [[nodiscard]] std::vector<std::pair<std::size_t, std::size_t>> spawn_positions() noexcept;

private:
    std::vector<std::vector<Cell>> m_board_state;

    std::vector<std::pair<std::size_t, std::size_t>> m_spawn_positions;
};

} // namespace bbm

#endif //SALMIAC_BOARD_H