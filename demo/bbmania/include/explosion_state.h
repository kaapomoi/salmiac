/*
 * Copyright (c) https://github.com/kaapomoi 2024.
 */

#ifndef BBMANIA_SERVER_EXPLOSION_STATE_H
#define BBMANIA_SERVER_EXPLOSION_STATE_H

#include <array>
#include <bitset>
#include <chrono>

/// This structure is created after a bomb explodes
/// An explosion will create a graphical representation and handle hitting players
struct Explosion_state {
    std::pair<std::size_t, std::size_t> grid_middle_point{0, 0};
    std::array<std::size_t, 4> range_up_down_left_right{0, 0, 0, 0};
    std::chrono::system_clock::time_point time_placed{};
    std::chrono::milliseconds life_time{};

    std::string placer_unique_id;
    //bool has_exploded{false};
};


#endif //BBMANIA_SERVER_EXPLOSION_STATE_H
