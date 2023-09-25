

/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "neural_net.h"

#include "log.h"

#include <algorithm>
#include <execution>
#include <numeric>
#include <ranges>
#include <sstream>

Neural_net::Neural_net(std::vector<std::size_t> const& topology) noexcept
{
    for (std::size_t i{0}; i < topology.size() - 1; i++) {
        std::size_t const layer_neuron_count{topology.at(i)};
        std::size_t const next_layer_neuron_count{topology.at(i + 1)};
        Layer l(layer_neuron_count);

        std::ranges::generate(l.begin(), l.end(), [this, &next_layer_neuron_count]() {
            Neuron n;
            n.weights.resize(next_layer_neuron_count);
            /// Randomize weights
            std::generate(n.weights.begin(), n.weights.end(),
                          [this]() { return rand_engine.get(0.f, 1.f); });
            return n;
        });

        m_net.push_back(l);
    }

    Layer last_layer(topology.back());
    std::ranges::generate(last_layer.begin(), last_layer.end(), []() { return Neuron{}; });
    m_net.push_back(last_layer);

    //print();
}


Neural_net::Neural_net(Neural_net const& a, Neural_net const& b, float const a_bias) noexcept
{
    m_net = a.m_net;

    for (std::size_t i{0}; i < m_net.size() - 1; i++) {
        for (std::size_t j{0}; j < m_net.at(i).size(); j++) {
            for (std::size_t k{0}; k < m_net.at(i).at(j).weights.size(); k++) {
                if (rand_engine.get(0.f, 1.f) >= a_bias) {
                    m_net.at(i).at(j).weights.at(k) = b.m_net.at(i).at(j).weights.at(k);
                }
            }
        }
    }
}


/// TODO: Add biases
std::vector<std::pair<std::size_t, double>> Neural_net::process(
    std::vector<double> inputs, std::function<double(double)> activation_function) noexcept
{
    Layer& first_layer{m_net.front()};
    for (std::size_t i{0}; i < inputs.size(); i++) {
        first_layer.at(i).output = inputs.at(i);
    }

    auto feed_forward = [&activation_function](Layer& current_layer, Layer& previous_layer) {
        for (std::size_t neuron_index{0}; neuron_index < current_layer.size(); neuron_index++) {
            double sum{0.0};

            std::for_each(std::execution::unseq, previous_layer.begin(), previous_layer.end(),
                          [&](Neuron const& prev_layer_neuron) -> void {
                              sum += prev_layer_neuron.output
                                     * prev_layer_neuron.weights.at(neuron_index);
                          });

            current_layer.at(neuron_index).output = activation_function(sum);
        }
    };

    for (std::size_t layer_index{1}; layer_index < m_net.size(); layer_index++) {
        Layer& layer{m_net.at(layer_index)};
        Layer& prev_layer{m_net.at(layer_index - 1)};
        feed_forward(layer, prev_layer);
    }

    std::vector<std::pair<std::size_t, double>> output_values;
    std::size_t color_index{0};
    std::transform(m_net.back().begin(), m_net.back().end(), std::back_inserter(output_values),
                   [&color_index](Neuron& n) -> std::pair<std::size_t, double> {
                       return {color_index++, n.output};
                   });

    return output_values;
}

void Neural_net::mutate_random(float likelyness) noexcept
{
    std::for_each(std::execution::unseq, m_net.begin(), m_net.end(), [&](Layer& layer) -> void {
        std::for_each(std::execution::unseq, layer.begin(), layer.end(),
                      [&](Neuron& neuron) -> void {
                          std::for_each(std::execution::unseq, neuron.weights.begin(),
                                        neuron.weights.end(), [&](double& weight) -> void {
                                            if (rand_engine.get(0.f, 1.f) <= likelyness) {
                                                weight = rand_engine.get(0.f, 1.f);
                                            }
                                        });
                      });
    });
}

void Neural_net::mutate_by_delta(float likelyness, float delta) noexcept
{
    std::for_each(std::execution::unseq, m_net.begin(), m_net.end(), [&](Layer& layer) -> void {
        std::for_each(std::execution::unseq, layer.begin(), layer.end(),
                      [&](Neuron& neuron) -> void {
                          std::for_each(std::execution::unseq, neuron.weights.begin(),
                                        neuron.weights.end(), [&](double& weight) -> void {
                                            if (rand_engine.get(0.f, 1.f) <= likelyness) {
                                                weight += rand_engine.get(-delta / 2, delta / 2);
                                            }
                                        });
                      });
    });
}


void Neural_net::print() noexcept
{
    for (auto const& layer : m_net) {
        for (auto const& neuron : layer) {
            std::stringstream s;
            for (auto const& weight : neuron.weights) {
                s << weight << " ";
            }

            sal::Log::info("{}, {}", neuron.output, s.str());
        }
    }
}
