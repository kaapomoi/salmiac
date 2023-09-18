/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "game.h"

#include <algorithm>
#include <numeric>
#include <queue>

Game::Game(std::size_t const board_w,
           std::size_t const board_h,
           std::size_t const n_colors,
           std::size_t const n_players) noexcept
    : m_board_width{board_w}, m_board_height{board_h}, m_n_colors{n_colors}, m_n_players{n_players}
{
    m_rand_engine.seed(time(NULL));

    m_starting_positions.push_back({0, 0});
    m_starting_positions.push_back({m_board_width - 1, m_board_height - 1});

    initialize_board();
}

std::vector<std::vector<Cell>> const& Game::cells() noexcept
{
    return m_cells;
}

std::vector<std::size_t> Game::available_moves() noexcept
{
    std::vector<size_t> available_moves;
    available_moves.resize(m_n_colors);
    std::iota(available_moves.begin(), available_moves.end(), 0);

    for (auto const& player : m_players) {
        std::erase(available_moves, player.current_color);
    }
    return available_moves;
}

void Game::initialize_board() noexcept
{
    m_cells.clear();

    for (std::size_t i{0}; i < m_n_players; i++) {
        m_players.emplace_back(Player{3, i, i});
    }

    /// Initialize board with a vector of vector of Cells
    for (std::size_t y{0}; y < m_board_height; y++) {
        m_cells.emplace_back(std::vector<Cell>(m_board_width));
    }

    std::uniform_int_distribution<std::size_t> cell_random_dist(0, m_n_colors - 1);
    for_each_cell([this, &cell_random_dist](std::size_t const x, std::size_t const y) {
        m_cells.at(y).at(x).color = cell_random_dist(m_rand_engine);
    });

    auto set_starting_cell = [this](glm::vec2 const& pos, std::size_t const new_owner) {
        if (in_bounds(pos)) {
            cell_at(pos).owner = new_owner;
            cell_at(pos).color = new_owner;
        }
    };

    auto force_foreign_cell = [this, &cell_random_dist](glm::vec2 const& pos,
                                                        std::size_t const new_owner) {
        if (in_bounds(pos)) {
            std::size_t color{cell_random_dist(m_rand_engine)};
            while (color == new_owner) {
                color = cell_random_dist(m_rand_engine);
            }
            cell_at(pos).color = color;
        }
    };

    for (std::size_t i{0}; i < m_starting_positions.size(); i++) {
        set_starting_cell(m_starting_positions.at(i), i);

        set_starting_cell(m_starting_positions.at(i) + glm::vec2{0, 1}, i);
        set_starting_cell(m_starting_positions.at(i) + glm::vec2{0, -1}, i);
        set_starting_cell(m_starting_positions.at(i) + glm::vec2{1, 0}, i);
        set_starting_cell(m_starting_positions.at(i) + glm::vec2{-1, 0}, i);

        force_foreign_cell(m_starting_positions.at(i) + glm::vec2{-1, -1}, i);
        force_foreign_cell(m_starting_positions.at(i) + glm::vec2{-1, 1}, i);
        force_foreign_cell(m_starting_positions.at(i) + glm::vec2{1, 1}, i);
        force_foreign_cell(m_starting_positions.at(i) + glm::vec2{1, -1}, i);

        force_foreign_cell(m_starting_positions.at(i) + glm::vec2{2, 0}, i);
        force_foreign_cell(m_starting_positions.at(i) + glm::vec2{-2, 0}, i);
        force_foreign_cell(m_starting_positions.at(i) + glm::vec2{0, 2}, i);
        force_foreign_cell(m_starting_positions.at(i) + glm::vec2{0, -2}, i);
    }
}

bool Game::execute_turn(std::size_t const player_index, std::size_t const color_index) noexcept
{
    /// Check turn.
    if (player_index != m_turn) {
        return false;
    }

    /// Check color validity
    if (color_index < 0 || color_index >= m_n_colors) {
        return false;
    }

    /// Disallow selecting any colors that are already selected.
    for (auto const& player : m_players) {
        if (player.current_color == color_index) {
            return false;
        }
    }

    // Transfer the ownership of the new color
    m_players.at(player_index).current_color = color_index;

    /// Flood fill to the new color
    flood_fill_to_color(m_starting_positions.at(player_index), player_index, color_index);

    /// Update the board state by running a bfs algorithm and
    /// changing the connected cells to `color_index`
    static_cast<void>(bfs(player_index, color_index,
                          [player_index](Cell& cell) -> void { cell.owner = player_index; }));

    m_turn++;
    m_turn = m_turn % m_n_players;

    return true;
}

void Game::flood_fill_to_color(glm::vec2 const pos,
                               size_t const owner,
                               size_t const new_color) noexcept
{
    if (!in_bounds(pos)) {
        return;
    }
    if (cell_at(pos).owner != owner) {
        return;
    }
    if (cell_at(pos).color == new_color) {
        return;
    }
    cell_at(pos).color = new_color;

    flood_fill_to_color(pos + glm::vec2{1, 0}, owner, new_color);
    flood_fill_to_color(pos + glm::vec2{-1, 0}, owner, new_color);
    flood_fill_to_color(pos + glm::vec2{0, 1}, owner, new_color);
    flood_fill_to_color(pos + glm::vec2{0, -1}, owner, new_color);
}

template<typename F>
std::size_t Game::bfs(std::size_t const player_index,
                      std::size_t const color_index,
                      F&& callback) noexcept
{
    std::size_t num_visited{0};
    std::size_t const old_color{m_players.at(player_index).current_color};

    std::array<std::array<bool, 100>, 100> visited{};

    std::queue<glm::vec2> search_queue{{m_starting_positions.at(player_index)}};

    auto check_pos = [&visited, this, &old_color, &player_index, &color_index,
                      &search_queue](glm::vec2 const pos) -> void {
        if (in_bounds(pos) && !visited.at(pos.y).at(pos.x) && cell_at(pos).color == color_index) {
            search_queue.push(pos);
            visited.at(pos.y).at(pos.x) = true;
        }
    };

    while (!search_queue.empty()) {
        glm::vec2 const pos = search_queue.front();

        callback(cell_at(pos));

        num_visited++;
        search_queue.pop();

        check_pos(pos + glm::vec2{0, 1});
        check_pos(pos + glm::vec2{0, -1});
        check_pos(pos + glm::vec2{1, 0});
        check_pos(pos + glm::vec2{-1, 0});
    }

    return num_visited;
}

bool Game::in_bounds(glm::vec2 const& pos) noexcept
{
    if (pos.x < 0 || pos.y < 0 || pos.x >= m_board_width || pos.y >= m_board_height) {
        return false;
    }
    return true;
}

void Game::reset_board() noexcept
{
    initialize_board();
}

Cell& Game::cell_at(glm::vec2 const& pos) noexcept
{
    return m_cells.at(pos.y).at(pos.x);
}
