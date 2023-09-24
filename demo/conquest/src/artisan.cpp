/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "artisan.h"

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
Artisan<Board_w, Board_h, N_colors, N_players>::Artisan(std::size_t const player_index) noexcept
    : m_player_index{player_index}
{
}

template<std::size_t Board_w, std::size_t Board_h, std::size_t N_colors, std::size_t N_players>
std::size_t Artisan<Board_w, Board_h, N_colors, N_players>::play(
    Game<Board_w, Board_h, N_colors, N_players>& game) noexcept
{
    std::vector<std::size_t> available_moves = game.available_moves();
    auto cells = game.cells();

    std::vector<double> input_values(Board_w * Board_h * N_colors);

    //std::generate(input_values.begin(), input_values.end(),
    //             [this]() { return m_neural_net.rand_engine.get(0.f, 1.f); });

    if (cells) {
        auto cells_unwrapped = cells.value();
        std::size_t input_value_index{0};
        std::size_t cell_index{0};
        for (std::size_t y{0}; y < cells_unwrapped.size(); y++) {
            for (std::size_t x{0}; x < cells_unwrapped.at(y).size(); x++) {

                input_value_index += cells_unwrapped.at(y).at(x).color;

                input_values.at(input_value_index) = 1;

                cell_index++;

                /// Reset index position to the next cell
                input_value_index = cell_index * N_colors;
            }
        }
    }


    /// TODO: Genetic algo.

    std::vector<std::pair<std::size_t, double>> res{
        m_neural_net.process(input_values, [](double d) -> double { return 1 / (1 + fabs(d)); })};

    std::erase_if(res, [&available_moves](auto const& index_value_pair) -> bool {
        /// Do not remove if the move is found
        return !std::any_of(
            available_moves.begin(), available_moves.end(),
            [&index = index_value_pair.first](auto const& move) { return move == index; });
    });

    std::sort(res.begin(), res.end(), [](auto &a, auto&b) -> bool{
            return a.second < b.second;
            });

    return res.front().first;
}

