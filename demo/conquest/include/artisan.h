/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_ARTISAN_H
#define SALMIAC_ARTISAN_H

#include "game.h"
#include "neural_net.h"

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
class Artisan {
public:
    explicit Artisan(std::size_t const player_index) noexcept;

    std::size_t play(Game<Board_w, Board_h, N_colors, N_players>& game) noexcept;

private:
    std::size_t m_player_index{0};

    Neural_net m_neural_net{{Board_w * Board_h * N_colors, 800, 6}};
};

#endif //SALMIAC_ARTISAN_H
