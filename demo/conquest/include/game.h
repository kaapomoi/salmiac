/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_GAME_H
#define SALMIAC_GAME_H

#include "application.h"
#include "cell_position.h"

#include <mutex>
#include <random>

struct Cell {
    std::size_t color{0};
    /// Default needs to be non-zero, since the first player is index 0
    std::size_t owner{255};
};

struct Player {
    std::size_t owned_cells{0};
    std::size_t current_color{0};
    std::size_t index{0};
};

class Game {
public:
    Game(std::size_t const board_w,
         std::size_t const board_h,
         std::size_t const n_colors,
         std::size_t const n_players) noexcept;
    ~Game() noexcept = default;

    std::vector<std::vector<Cell>> const& cells() noexcept;

    std::vector<std::size_t> available_moves() noexcept;

    void reset_board() noexcept;

    bool execute_turn(std::size_t const player_index, std::size_t const color_index) noexcept;

    bool done() noexcept;

    template<typename F>
    std::size_t bfs(std::size_t const player_index,
                    std::size_t const color_index,
                    F&& callback) noexcept;

    Game(const Game& other) = delete;

    Game& operator=(const Game& other) = delete;

private:
    void initialize_board() noexcept;

    void flood_fill_to_color(glm::vec2 const pos,
                             size_t const owner,
                             size_t const new_color) noexcept;


    bool in_bounds(glm::vec2 const& pos) noexcept;

    Cell& cell_at(glm::vec2 const& pos) noexcept;

    template<typename F>
    void for_each_cell(F&& func) noexcept
    {
        for (std::size_t y{0}; y < m_board_height; y++) {
            for (std::size_t x{0}; x < m_board_width; x++) {
                func(x, y);
            }
        }
    }

    std::mutex m_cell_mutex;
    std::vector<std::vector<Cell>> m_cells;
    std::mt19937 m_rand_engine;

    std::size_t m_board_width{0};
    std::size_t m_board_height{0};
    std::size_t m_n_colors{0};
    std::size_t m_n_players{0};

    std::size_t m_turn{0};
    std::vector<Player> m_players;
    std::vector<glm::vec2> m_starting_positions;
};

#endif //SALMIAC_GAME_H
