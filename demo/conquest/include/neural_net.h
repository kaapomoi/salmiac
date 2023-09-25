/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_NEURAL_NET_H
#define SALMIAC_NEURAL_NET_H

#include "effolkronium/random.hpp"

#include <functional>
#include <vector>

struct Neuron {
    double output{0.f};
    std::vector<double> weights;
};

class Neural_net {
public:
    using Layer = std::vector<Neuron>;
    explicit Neural_net(std::vector<std::size_t> const& topology) noexcept;

    Neural_net(Neural_net const& a, Neural_net const& b, float const a_bias) noexcept;

    std::vector<std::pair<std::size_t, double>>
    process(std::vector<double> inputs, std::function<double(double)> activation_function) noexcept;

    void mutate_random(float likelyness) noexcept;

    void mutate_by_delta(float likelyness, float delta) noexcept;

    using Random_engine = effolkronium::random_local;

    Random_engine rand_engine{};

private:
    void print() noexcept;

    std::vector<Layer> m_net;
};


#endif //SALMIAC_NEURAL_NET_H
