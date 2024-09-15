/*
 * Copyright (c) https://github.com/kaapomoi 2024.
 */

#ifndef BBMANIA_SERVER_PICKUP_STATE_H
#define BBMANIA_SERVER_PICKUP_STATE_H


#include "glm/glm.hpp"

struct Pickup_state {
    enum class Type : std::uint8_t { BOMB_COUNT_PLUS, RANGE_PLUS };

    std::pair<std::size_t, std::size_t> grid_position{0, 0};
    Type type{Type::BOMB_COUNT_PLUS};
    bool picked_up{false};
};


#endif //BBMANIA_SERVER_PICKUP_STATE_H
