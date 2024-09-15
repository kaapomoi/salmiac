/*
 * Copyright (c) https://github.com/kaapomoi 2024.
 */

#ifndef BBMANIA_NETWORK_PROTOCOL_H
#define BBMANIA_NETWORK_PROTOCOL_H

#include "game_state.h"

#include <cstdint>
#include <optional>
#include <string>

namespace bbm {


class Network_protocol {
public:
    /// Note: Some messages may be used for bi-directional communication,
    /// e.g. DISCONNECT ACK <connection name> is sent from server to the client.
    /// It all depends on the message content.
    struct Message {
        enum class Type : std::int32_t {
            CONNECT,
            VOTE_START,
            DISCONNECT,
            MOVE,
            PLACE,
            SPAWN_WALL,
            EXPLODE,
            BOMB_SPAWN,
            SPAWN_EXPL,
            DESTROY_BOX,
            SPAWN_PICKUP,
            PICK_UP_PICKUP,
            RESPAWN_PLAYER,
            INIT_GAME_STATE,
            PLAYER_POSITION,
            INIT_PLAYER_STATE,
            UPDATE_SCORES,
            ANNOUNCE_WINNER,
            RESTART_GAME
        };

        Type type;
        std::string data;
    };

    enum class Direction : std::uint8_t { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };

    static std::optional<Message> parse_message(std::string const& network_packet) noexcept;

    static std::string create_connect_message(std::string const& client_display_name) noexcept;

    static std::string create_connect_ack_message(std::string const& unique_client_id) noexcept;

    static std::string create_vote_start_message(std::string const& client_display_name,
                                                 bool ready) noexcept;

    static std::string create_vote_start_ack_message(std::string const& client_display_name,
                                                     bool ready) noexcept;

    static std::string create_disconnect_message(std::string const& client_display_name) noexcept;

    static std::string
    create_disconnect_ack_message(std::string const& client_display_name) noexcept;

    static std::string create_destroy_box_message(
        std::vector<std::pair<std::size_t, std::size_t>> const& destroyed_box_positions) noexcept;

    static std::string create_init_game_state_message(Game_state const& game_state) noexcept;

    static std::string
    create_spawn_wall_message(std::pair<std::size_t, std::size_t> const wall_pos) noexcept;

    static std::string create_player_position_message(Game_state const& game_state) noexcept;

    static std::string create_init_player_state_message(Game_state const& game_state) noexcept;

    static std::string create_move_player_message(std::string const& client_display_name,
                                                  bool start_or_stop_movement,
                                                  Direction direction) noexcept;

    static std::string create_place_message(std::string const& client_display_name) noexcept;

    static std::string create_place_ack_message(bool const successful_placement,
                                                std::size_t const new_bomb_count) noexcept;

    /// Sent by the server to all the clients once a pickup has been spawned successfully.
    static std::string
    create_spawn_pickup_message(std::vector<Pickup_state> const& pickups) noexcept;

    /// Sent by the server to all the clients once a pickup has been picked up successfully.
    static std::string create_pick_up_pickup_message(std::string const& player_name,
                                                     Pickup_state const& pickup) noexcept;

    static std::string create_respawn_player_message(
        std::string const& player_display_name, std::size_t const invulnerability_millis) noexcept;

    /// Sent by the server to all the clients once a bomb has been placed successfully.
    static std::string create_bomb_spawn_message(Bomb_state const& bomb_state) noexcept;

    /// Sent by the server to all the clients once a bomb has exploded.
    static std::string create_explode_message(
        std::vector<std::pair<std::size_t, std::size_t>> const& exploded_bomb_positions) noexcept;

    static std::string
    create_spawn_explosion_message(std::vector<Explosion_state> const& explosion_states) noexcept;

    static std::string create_update_scores_message(Game_state const& game_state) noexcept;

    static std::string
    create_announce_winner_message(std::string const& winner_display_name) noexcept;

    static std::string create_restart_game_message() noexcept;
};

} // namespace bbm

#endif //BBMANIA_NETWORK_PROTOCOL_H
