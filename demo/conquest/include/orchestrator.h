/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_ORCHESTRATOR_H
#define SALMIAC_ORCHESTRATOR_H

#include "artisan.h"
#include "game.h"
#include "thread_pool.h"

class Orchestrator {
public:
    Orchestrator(std::size_t const n_games,
                 std::size_t const board_w,
                 std::size_t const board_h) noexcept;

    std::size_t play_one_game_each() noexcept;

    void restart() noexcept;
    void stop() noexcept;

    std::vector<std::vector<std::vector<Cell>>> cells() noexcept;

private:
    sal::Thread_pool<std::function<void()>> m_thread_pool;

    std::mt19937 m_rand_engine;
    std::vector<std::unique_ptr<Game>> m_games;
    std::vector<Artisan> m_artisans;
};

#endif //SALMIAC_ARTISAN_H
