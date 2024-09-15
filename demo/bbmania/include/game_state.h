/*
 * Copyright (c) https://github.com/kaapomoi 2024.
 */

#ifndef SALMIAC_GAME_STATE_H
#define SALMIAC_GAME_STATE_H

#include "board.h"
#include "bomb_state.h"
#include "explosion_state.h"
#include "pickup_state.h"
#include "player_state.h"

#include <map>
#include <vector>

namespace bbm {

struct Game_state {
    Board m_board;

    /// The string is the unique id of each client in the server side.
    std::map<std::string, Player_state> m_player_states;
    std::map<std::string, Player_state> m_dead_player_states{};

    std::vector<Bomb_state> m_bomb_states{};
    std::vector<Explosion_state> m_explosion_states{};
    std::vector<Pickup_state> m_pickup_states{};

    bool showdown_started{false};

    void reset() noexcept
    {
        m_board = Board{};

        m_player_states.clear();
        m_dead_player_states.clear();
        m_bomb_states.clear();

        m_explosion_states.clear();
        m_pickup_states.clear();

        showdown_started = false;
    }
};

} // namespace bbm

#endif //SALMIAC_GAME_STATE_H
