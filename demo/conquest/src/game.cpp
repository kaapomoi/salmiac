/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "game.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <queue>

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
Game<Board_w, Board_h, N_colors, N_players>::Game() noexcept

{
    m_starting_positions.push_back({0, 0});
    m_starting_positions.push_back({Board_w - 1, Board_h - 1});

    initialize_board();
}

//Game::~Game() noexcept
//{
//    //std::lock_guard<std::mutex> lck(m_cell_mutex);
//}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
std::optional<typename Game<Board_w, Board_h, N_colors, N_players>::Board_state>
Game<Board_w, Board_h, N_colors, N_players>::cells() noexcept
{
    /// TODO: Fix this, add a callback for the main thread, or separate this function for AI and Renderer:
    /*if (m_should_not_report.load()) {
        return {};
    }

    /// This is the last time we report the cells.
    if (done()) {
        m_should_not_report.store(true);
    }
    */

    std::lock_guard<std::mutex> lck{m_cell_mutex};
    return m_cells;
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
std::vector<std::size_t> Game<Board_w, Board_h, N_colors, N_players>::available_moves() noexcept
{
    std::lock_guard<std::mutex> lck{m_cell_mutex};

    std::vector<size_t> available_moves;

    for (std::size_t i{0}; i < N_colors; i++) {
        bool add{true};
        for (auto const& player : m_players) {
            if (i == player.current_color) {
                add = false;
            }
        }
        if (add) {
            available_moves.push_back(i);
        }
    }

    return available_moves;
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
bool Game<Board_w, Board_h, N_colors, N_players>::done() noexcept
{
    std::lock_guard<std::mutex> lck{m_cell_mutex};
    return std::reduce(m_players.begin(), m_players.end(), 0,
                       [](std::size_t sum, Player const& p) { return sum + p.owned_cells; })
           == (Board_w * Board_h);
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
void Game<Board_w, Board_h, N_colors, N_players>::initialize_board() noexcept
{
    std::lock_guard<std::mutex> lck{m_cell_mutex};

    // Fill all the cells with default values:
    m_cells.fill(std::array<Cell, Board_w>{});

    m_players.clear();
    m_should_not_report.store(false);

    for (std::size_t i{0}; i < N_players; i++) {
        m_players.emplace_back(Player{3, i, i});
    }

    /// Initialize board with random Cells
    for_each_cell([this](std::size_t const x, std::size_t const y) {
        m_cells.at(y).at(x).color = m_rand_engine.get(std::size_t{0}, (N_colors - 1));
    });

    auto set_starting_cell = [this](v2 const& pos, std::size_t const new_owner) {
        if (in_bounds(pos)) {
            cell_at(pos).owner = new_owner;
            cell_at(pos).color = new_owner;
        }
    };

    auto force_foreign_cell = [this](v2 const& pos, std::size_t const new_owner) {
        if (in_bounds(pos)) {
            std::size_t color{m_rand_engine.get(std::size_t{0}, (N_colors - 1))};
            while (color == new_owner) {
                color = m_rand_engine.get(std::size_t{0}, N_colors - 1);
            }
            cell_at(pos).color = color;
        }
    };

    for (std::size_t i{0}; i < m_starting_positions.size(); i++) {
        set_starting_cell(m_starting_positions.at(i), i);

        set_starting_cell(m_starting_positions.at(i) + v2{0, 1}, i);
        set_starting_cell(m_starting_positions.at(i) + v2{0, -1}, i);
        set_starting_cell(m_starting_positions.at(i) + v2{1, 0}, i);
        set_starting_cell(m_starting_positions.at(i) + v2{-1, 0}, i);

        force_foreign_cell(m_starting_positions.at(i) + v2{-1, -1}, i);
        force_foreign_cell(m_starting_positions.at(i) + v2{-1, 1}, i);
        force_foreign_cell(m_starting_positions.at(i) + v2{1, 1}, i);
        force_foreign_cell(m_starting_positions.at(i) + v2{1, -1}, i);

        force_foreign_cell(m_starting_positions.at(i) + v2{2, 0}, i);
        force_foreign_cell(m_starting_positions.at(i) + v2{-2, 0}, i);
        force_foreign_cell(m_starting_positions.at(i) + v2{0, 2}, i);
        force_foreign_cell(m_starting_positions.at(i) + v2{0, -2}, i);
    }
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
bool Game<Board_w, Board_h, N_colors, N_players>::execute_turn(
    std::size_t const player_index, std::size_t const color_index) noexcept
{
    std::lock_guard<std::mutex> lck{m_cell_mutex};
    /// Check turn.
    if (player_index != m_turn) {
        sal::Log::error("Wrong turn! t:{} p:{}", m_turn, player_index);
        return false;
    }

    /// Check color validity
    if (color_index < 0 || color_index >= N_colors) {
        sal::Log::error("Color OOB c:{}", color_index);
        return false;
    }

    /// Disallow selecting any colors that are already selected.
    for (auto const& player : m_players) {
        if (player.current_color == color_index) {
            sal::Log::error("Color already owned c:{} {}", color_index, player.current_color);
            return false;
        }
    }

    // Transfer the ownership of the new color
    m_players.at(player_index).current_color = color_index;

    /// Flood fill to the new color
    flood_fill_to_color(m_starting_positions.at(player_index), player_index, color_index);

    /// Update the board state by running a bfs algorithm and
    /// changing the connected cells to `color_index`
    m_players.at(player_index).owned_cells =
        bfs(player_index, color_index,
            [player_index](Cell& cell) -> void { cell.owner = player_index; });

    m_turn++;
    if (m_turn >= N_players) {
        m_turn = 0;
    }

    return true;
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
void Game<Board_w, Board_h, N_colors, N_players>::flood_fill_to_color(
    v2 const pos, size_t const owner, size_t const new_color) noexcept
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

    flood_fill_to_color(pos + v2{1, 0}, owner, new_color);
    flood_fill_to_color(pos + v2{-1, 0}, owner, new_color);
    flood_fill_to_color(pos + v2{0, 1}, owner, new_color);
    flood_fill_to_color(pos + v2{0, -1}, owner, new_color);
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
template<typename F>
std::size_t Game<Board_w, Board_h, N_colors, N_players>::bfs(std::size_t const player_index,
                                                             std::size_t const color_index,
                                                             F&& callback) noexcept
{
    std::size_t num_visited{0};
    std::size_t const old_color{m_players.at(player_index).current_color};

    /// TODO: Make a better visited array.
    std::array<std::array<bool, 50>, 50> visited{};

    std::queue<v2> search_queue{{m_starting_positions.at(player_index)}};
    visited.at(search_queue.front().y).at(search_queue.front().x) = true;

    auto check_pos = [&visited, this, &color_index, &num_visited,
                      &search_queue](v2 const pos) -> void {
        if (in_bounds(pos) && !visited.at(pos.y).at(pos.x) && cell_at(pos).color == color_index) {
            search_queue.push(pos);
            visited.at(pos.y).at(pos.x) = true;
        }
    };

    while (!search_queue.empty()) {
        v2 const pos = search_queue.front();
        num_visited++;

        callback(cell_at(pos));

        search_queue.pop();

        check_pos(pos + v2{0, 1});
        check_pos(pos + v2{0, -1});
        check_pos(pos + v2{1, 0});
        check_pos(pos + v2{-1, 0});
    }

    return num_visited;
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
bool Game<Board_w, Board_h, N_colors, N_players>::in_bounds(v2 const& pos) noexcept
{
    if (pos.x < 0 || pos.y < 0 || pos.x >= Board_w || pos.y >= Board_h) {
        return false;
    }
    return true;
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
void Game<Board_w, Board_h, N_colors, N_players>::reset_board() noexcept
{
    initialize_board();
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
Cell& Game<Board_w, Board_h, N_colors, N_players>::cell_at(v2 const& pos) noexcept
{
    return m_cells.at(pos.y).at(pos.x);
}

