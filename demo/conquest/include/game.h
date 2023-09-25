/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_GAME_H
#define SALMIAC_GAME_H

#include "application.h"
#include "cell_position.h"

#include "effolkronium/random.hpp"

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

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
class Game {
public:
    using Random_engine = effolkronium::random_local;
    using Board_state = std::array<std::array<Cell, Board_w>, Board_h>;
    static constexpr std::size_t max_turns{200};

    Game() noexcept;
    ~Game() noexcept = default;

    std::vector<Player> const& players() noexcept;

    std::optional<Board_state> cells() noexcept;

    std::vector<std::size_t> available_moves() noexcept;

    void reset_board() noexcept;

    bool execute_turn(std::size_t const player_index, std::size_t const color_index) noexcept;

    bool done() noexcept;

    template<typename F>
    std::size_t bfs(std::size_t const player_index,
                    std::size_t const color_index,
                    F&& callback) noexcept;

    Random_engine& rand_engine() noexcept;

    Game(const Game& other) = delete;

    Game& operator=(const Game& other) = delete;


private:
    typedef glm::vec<2, std::int32_t> v2;

    void initialize_board() noexcept;

    void flood_fill_to_color(v2 const pos, size_t const owner, size_t const new_color) noexcept;


    bool in_bounds(v2 const& pos) noexcept;

    Cell& cell_at(v2 const& pos) noexcept;

    template<typename F>
    void for_each_cell(F&& func) noexcept
    {
        for (std::size_t y{0}; y < Board_h; y++) {
            for (std::size_t x{0}; x < Board_w; x++) {
                func(x, y);
            }
        }
    }


    std::mutex m_cell_mutex;

    //std::vector<std::vector<Cell>> m_cells;
    std::array<std::array<Cell, Board_w>, Board_h> m_cells;

    Random_engine m_rand_engine{};

    std::size_t m_turns_played{0};
    std::size_t m_turn{0};
    std::vector<Player> m_players;
    std::vector<v2> m_starting_positions;
    std::atomic_bool m_should_not_report;
};

#endif //SALMIAC_GAME_H
