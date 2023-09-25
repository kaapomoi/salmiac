/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#include "orchestrator.h"

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
Orchestrator<Board_w, Board_h, N_colors, N_players>::Orchestrator(
    std::size_t const n_games) noexcept
    : m_thread_pool{std::thread::hardware_concurrency()}, m_n_games{n_games}
{
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
    for (auto& game : m_games) {
        game->reset_board();
    }

    play_games_for_current_artisan();
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
void Orchestrator<Board_w, Board_h, N_colors, N_players>::stop() noexcept
{
    m_thread_pool.cancel_all();
}


template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
void Orchestrator<Board_w, Board_h, N_colors, N_players>::start() noexcept
{
    for (std::size_t i{0}; i < m_n_games; i++) {
        m_games.push_back(std::make_unique<Game<Board_w, Board_h, N_colors, N_players>>());
    }

    for (std::size_t i{0}; i < 10; i++) {
        m_artisans.emplace_back();
    }

    play_games_for_current_artisan();
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
std::size_t
Orchestrator<Board_w, Board_h, N_colors, N_players>::play_games_for_current_artisan() noexcept
{
    for (auto& game : m_games) {
        m_thread_pool.insert(std::move([this, &game]() -> void {
            while (!game->done()) {
                auto avail_moves = game->available_moves();
                auto move = m_artisans.at(m_current_artisan_index).play(*game);
                if (!game->execute_turn(0, move)) {
                    sal::Log::error("Artisan move failed {} {}", 0, move);
                }

                avail_moves = game->available_moves();
                move =
                    avail_moves.at(game->rand_engine().get(std::size_t{0}, avail_moves.size() - 1));
                if (!game->execute_turn(1, move)) {
                    sal::Log::error("Randomizer move failed {} {}", 1, move);
                }
            }
        }));
    }

    return 0;
}


template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
void Orchestrator<Board_w, Board_h, N_colors, N_players>::update() noexcept
{
    /// Note: Wait for all games to finish
    std::size_t total_owned_cells{0};
    for (auto& game : m_games) {
        if (!game->done()) {
            return;
        }
        total_owned_cells += game->players().at(0).owned_cells;
    }

    m_artisans.at(m_current_artisan_index)
        .set_fitness(static_cast<float>(total_owned_cells) / static_cast<float>(m_n_games));

    /// Reset all games, pick next agent.
    m_current_artisan_index++;

    if (m_current_artisan_index >= m_artisans.size()) {
        m_current_artisan_index = 0;

        /// Generation done. Commence genetic algorithm
        std::sort(m_artisans.begin(), m_artisans.end(),
                  [](Artisan<Board_w, Board_h, N_colors, N_players>& lhs,
                     Artisan<Board_w, Board_h, N_colors, N_players>& rhs) -> bool {
                      return lhs.fitness() > rhs.fitness();
                  });

        m_artisans.erase(m_artisans.begin() + (m_artisans.size() / 2), m_artisans.end());

        std::for_each(m_artisans.begin(), m_artisans.end(), [](auto& artisan) -> void {
            artisan.mutate_random(0.05f);
            artisan.mutate_by_delta(0.05f, 0.1f);
        });

        /// TODO: Cross-breed artisans
        for (std::size_t i{m_artisans.size()}; i < 10; i++) {
            m_artisans.emplace_back(m_artisans.at(0), m_artisans.at(1), 0.6f);
        }
    }

    restart();
}


template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
std::pair<std::size_t, float>
Orchestrator<Board_w, Board_h, N_colors, N_players>::current_artisan() noexcept
{
    return {m_current_artisan_index, m_artisans.at(m_current_artisan_index).fitness()};
}
