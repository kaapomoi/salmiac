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

    Artisan(Artisan const& a, Artisan const& b, float const a_bias) noexcept;

    std::size_t play(Game<Board_w, Board_h, N_colors, N_players>& game) noexcept;

    float fitness() noexcept;

    void set_fitness(float const fitness) noexcept;

    void mutate_random(float likelyness) noexcept;

    void mutate_by_delta(float likelyness, float delta) noexcept;

private:
    float m_fitness{0.f};

    Neural_net m_neural_net;
};

#endif //SALMIAC_ARTISAN_H
