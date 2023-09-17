/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_ARTISAN_H
#define SALMIAC_ARTISAN_H

#include "game.h"

class Artisan {
public:
    Artisan() noexcept;

    std::size_t play(std::vector<std::size_t> const& available_moves) noexcept;

private:
    std::mt19937 m_rand_engine;
};

#endif //SALMIAC_ARTISAN_H
