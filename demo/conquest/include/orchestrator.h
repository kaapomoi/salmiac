/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_ORCHESTRATOR_H
#define SALMIAC_ORCHESTRATOR_H

#include "artisan.h"
#include "game.h"
#include "thread_pool.h"

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
class Orchestrator {
public:
    Orchestrator(std::size_t const n_games) noexcept;

    std::size_t play_one_game_each() noexcept;

    void restart() noexcept;
    void stop() noexcept;

    std::vector<std::optional<typename Game<Board_w, Board_h, N_colors, N_players>::Board_state>>
    cells() noexcept;

private:
    sal::Thread_pool<std::function<void()>> m_thread_pool;

    std::vector<std::unique_ptr<Game<Board_w, Board_h, N_colors, N_players>>> m_games;
    std::vector<Artisan<Board_w, Board_h, N_colors, N_players>> m_artisans;
};

#endif //SALMIAC_ARTISAN_H
