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
    Artisan() noexcept;

    std::size_t play(Game<Board_w, Board_h, N_colors, N_players>& game) noexcept;

private:
    Neural_net m_neural_net{{Board_w * Board_h * N_colors, 800, 6}};
};

#endif //SALMIAC_ARTISAN_H
