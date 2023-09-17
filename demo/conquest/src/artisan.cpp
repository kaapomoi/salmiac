/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "artisan.h"

Artisan::Artisan() noexcept {}

std::size_t Artisan::play(std::vector<std::size_t> const& available_moves) noexcept
{
    std::uniform_int_distribution<std::size_t> random_color_dist(0, available_moves.size() - 1);
    return available_moves.at(random_color_dist(m_rand_engine));
}
