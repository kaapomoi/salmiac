/*
 * Copyright (c) https://github.com/kaapomoi 2024.
 */

#ifndef BBMANIA_SERVER_BOMB_STATE_H
#define BBMANIA_SERVER_BOMB_STATE_H


#include "glm/glm.hpp"

#include <bitset>
#include <chrono>

struct Bomb_state {
    std::pair<std::size_t, std::size_t> grid_position{0, 0};
    std::uint32_t explosion_range{0};
    std::chrono::system_clock::time_point time_placed{};

    std::string placer_unique_id;
    bool has_exploded{false};
};


#endif //BBMANIA_SERVER_BOMB_STATE_H
