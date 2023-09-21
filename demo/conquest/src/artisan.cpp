/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "artisan.h"

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
Artisan<Board_w, Board_h, N_colors, N_players>::Artisan() noexcept
{
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
std::size_t Artisan<Board_w, Board_h, N_colors, N_players>::play(
    Game<Board_w, Board_h, N_colors, N_players>& game) noexcept
{
    std::vector<std::size_t> available_moves = game.available_moves();
    auto cells = game.cells();

    std::vector<double> input_values(Board_w * Board_h * N_colors);

    std::generate(input_values.begin(), input_values.end(),
                  [this]() { return m_neural_net.rand_engine.get(0.f, 1.f); });

    //std::transform(cells.begin(), cells.end(), std::back_inserter(input_values),
    //              [](Neuron& n) -> double { return n.output; });

    /// TODO: Genetic algo. Proper inputs. NColors as template param? Neural net as std::array.

    auto res =
        m_neural_net.process(input_values, [](double d) -> double { return 1 / (1 + fabs(d)); });

    return available_moves.at(
        m_neural_net.rand_engine.get(std::size_t{0}, available_moves.size() - 1));
}

