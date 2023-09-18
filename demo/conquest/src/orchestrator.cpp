/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#include "orchestrator.h"

Orchestrator::Orchestrator(std::size_t const n_games,
                           std::size_t const board_w,
                           std::size_t const board_h) noexcept
    : m_thread_pool{std::thread::hardware_concurrency()}
{

    for (std::size_t i{0}; i < n_games; i++) {
        m_games.push_back(std::make_unique<Game>(board_w, board_h, 6, 2));
    }

    m_artisans.emplace_back();
    m_artisans.emplace_back();
}

std::vector<std::vector<std::vector<Cell>>> Orchestrator::cells() noexcept
{
    std::vector<std::vector<std::vector<Cell>>> ret;
    for (auto& game : m_games) {
        ret.push_back(game->cells());
    }
    return ret;
}

void Orchestrator::restart() noexcept
{
    std::jthread j([this]() {
        for (auto& game : m_games) {
            game->reset_board();
        }
    });
}

void Orchestrator::stop() noexcept {}

std::size_t Orchestrator::play_one_game_each() noexcept
{
    for (auto& game : m_games) {
        m_thread_pool.insert(std::move([this, &game]() -> void {
            /// TODO: game not finished
            try {
                while (!game->done()) {
                    for (std::size_t i{0}; i < m_artisans.size(); i++) {
                        auto move = m_artisans.at(i).play(game->available_moves());
                        game->execute_turn(i, move);
                        std::this_thread::sleep_for(std::chrono::milliseconds{1});
                    }
                }
            }
            catch (std::exception const& e) {
                sal::Log::fatal("Exception {}", e.what());
            }
        }));
    }

    return 0;
}
