/*
 * Copyright (c) https://github.com/kaapomoi 2024.
 */

#ifndef BBMANIA_SERVER_PLAYER_STATE_H
#define BBMANIA_SERVER_PLAYER_STATE_H

#include "glm/glm.hpp"

#include <bitset>

struct Player_state {
    glm::vec2 position{0.f};
    std::uint32_t skin_id{0};
    std::uint32_t bombs{0};
    std::uint32_t range{0};
    std::int32_t score{0};
    std::bitset<8> specials{0b00000000};
    std::bitset<4> current_movement{0b0000};

    std::string display_name;

    bool alive{true};
    std::chrono::system_clock::time_point invincible_until_this_point_point{};
};


#endif //BBMANIA_SERVER_PLAYER_STATE_H
