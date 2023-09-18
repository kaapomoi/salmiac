/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_ORCHESTRATOR_H
#define SALMIAC_ORCHESTRATOR_H

#include "game.h"
#include "thread_pool.h"

class Orchestrator {
public:
    Orchestrator() noexcept;

    std::size_t play(std::vector<std::size_t> const& available_moves) noexcept;

private:
    //sal::Thread_pool m_thread_pool;

    std::mt19937 m_rand_engine;
};

#endif //SALMIAC_ARTISAN_H
