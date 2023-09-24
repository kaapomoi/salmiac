/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#include "orchestrator.h"

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
Orchestrator<Board_w, Board_h, N_colors, N_players>::Orchestrator(
    std::size_t const n_games) noexcept
    : m_thread_pool{std::thread::hardware_concurrency()}
{

    for (std::size_t i{0}; i < n_games; i++) {
        m_games.push_back(std::make_unique<Game<Board_w, Board_h, N_colors, N_players>>());
    }

    m_artisans.emplace_back(0);
    m_artisans.emplace_back(1);
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
std::vector<std::optional<typename Game<Board_w, Board_h, N_colors, N_players>::Board_state>>
Orchestrator<Board_w, Board_h, N_colors, N_players>::cells() noexcept
{
    std::vector<std::optional<typename Game<Board_w, Board_h, N_colors, N_players>::Board_state>>
        ret;
    for (auto& game : m_games) {
        ret.push_back(game->cells());
    }
    return ret;
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
void Orchestrator<Board_w, Board_h, N_colors, N_players>::restart() noexcept
{
    std::jthread j([this]() {
        for (auto& game : m_games) {
            game->reset_board();
        }
        play_one_game_each();
    });
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
void Orchestrator<Board_w, Board_h, N_colors, N_players>::stop() noexcept
{
    m_thread_pool.cancel_all();
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
std::size_t Orchestrator<Board_w, Board_h, N_colors, N_players>::play_one_game_each() noexcept
{
    for (auto& game : m_games) {
        m_thread_pool.insert(std::move([this, &game]() -> void {
            while (!game->done()) {
                for (std::size_t i{0}; i < m_artisans.size(); i++) {
                    auto avail_moves = game->available_moves();
                    auto move = m_artisans.at(i).play(*game);
                    if (!game->execute_turn(i, move)) {
                        sal::Log::error("Move failed {} {}", i, move);
                    }
                }
            }
        }));
    }

    return 0;
}

