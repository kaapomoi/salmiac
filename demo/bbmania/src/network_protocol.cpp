#include "network_protocol.h"

#include <map>
#include <sstream>

namespace bbm {


std::optional<Network_protocol::Message>
Network_protocol::parse_message(std::string const& msg_in) noexcept
{
    static std::map<std::string, Network_protocol::Message::Type> const str_to_msg_type{
        {"CONNECT", Network_protocol::Message::Type::CONNECT},
        {"VOTE_START", Network_protocol::Message::Type::VOTE_START},
        {"DISCONNECT", Network_protocol::Message::Type::DISCONNECT},
        {"MOVE", Network_protocol::Message::Type::MOVE},
        {"PLACE", Network_protocol::Message::Type::PLACE},
        {"SPAWN_WALL", Network_protocol::Message::Type::SPAWN_WALL},
        {"EXPLODE", Network_protocol::Message::Type::EXPLODE},
        {"BOMB_SPAWN", Network_protocol::Message::Type::BOMB_SPAWN},
        {"SPAWN_EXPL", Network_protocol::Message::Type::SPAWN_EXPL},
        {"DESTROY_BOX", Network_protocol::Message::Type::DESTROY_BOX},
        {"SPAWN_PICKUP", Network_protocol::Message::Type::SPAWN_PICKUP},
        {"PICK_UP_PICKUP", Network_protocol::Message::Type::PICK_UP_PICKUP},
        {"RESPAWN_PLAYER", Network_protocol::Message::Type::RESPAWN_PLAYER},
        {"INIT_GAME_STATE", Network_protocol::Message::Type::INIT_GAME_STATE},
        {"PLAYER_POSITION", Network_protocol::Message::Type::PLAYER_POSITION},
        {"INIT_PLAYER_STATE", Network_protocol::Message::Type::INIT_PLAYER_STATE},
        {"UPDATE_SCORES", Network_protocol::Message::Type::UPDATE_SCORES},
        {"ANNOUNCE_WINNER", Network_protocol::Message::Type::ANNOUNCE_WINNER},
        {"RESTART_GAME", Network_protocol::Message::Type::RESTART_GAME}};

    for (auto const& [hdr, type] : str_to_msg_type) {
        if (msg_in.substr(0, hdr.size()) == hdr) {
            return Network_protocol::Message{.type = type, .data = msg_in.substr(hdr.size() + 1)};
        }
    }

    return {};
}

std::string
Network_protocol::create_connect_message(std::string const& client_display_name) noexcept
{
    /// TODO: Should there be a token with the player connection?
    return {"CONNECT " + client_display_name};
}

std::string
Network_protocol::create_connect_ack_message(std::string const& unique_client_id) noexcept
{
    return {"CONNECT ACK " + unique_client_id};
}

std::string Network_protocol::create_vote_start_message(std::string const& client_display_name,
                                                        bool ready) noexcept
{
    std::string const ready_or_not{ready ? "R" : "U"};
    return {"VOTE_START " + ready_or_not + client_display_name};
}

std::string Network_protocol::create_vote_start_ack_message(std::string const& client_display_name,
                                                            bool ready) noexcept
{
    std::string const ready_or_not{ready ? "R" : "U"};
    return {"VOTE_START ACK " + ready_or_not + client_display_name};
}

std::string
Network_protocol::create_disconnect_message(std::string const& client_display_name) noexcept
{
    /// TODO: Should there be a token with the player connection?
    return {"DISCONNECT " + client_display_name};
}

std::string
Network_protocol::create_disconnect_ack_message(std::string const& client_display_name) noexcept
{
    /// TODO: Should there be a token with the player connection?
    return {"DISCONNECT ACK " + client_display_name};
}

std::string Network_protocol::create_init_game_state_message(Game_state const& game_state) noexcept
{
    std::string serialized_game_state{"INIT_GAME_STATE "
                                      + game_state.m_board.serialize_for_network()};

    return serialized_game_state;
}

std::string Network_protocol::create_spawn_wall_message(
    std::pair<std::size_t, std::size_t> const wall_pos) noexcept
{
    return {"SPAWN_WALL " + std::to_string(wall_pos.first) + ":" + std::to_string(wall_pos.second)};
}

std::string Network_protocol::create_player_position_message(Game_state const& game_state) noexcept
{
    std::string serialized_player_state{
        "PLAYER_POSITION "
        + std::to_string(game_state.m_player_states.size() + game_state.m_dead_player_states.size())
        + ";"};

    for (auto const& [unique_client_id, player_state] : game_state.m_player_states) {
        serialized_player_state.append(player_state.display_name);
        serialized_player_state.append(":");

        float const x_pos = player_state.position.x;
        float const y_pos = player_state.position.y;

        std::stringstream strstream;
        strstream << x_pos << ":" << y_pos << ";";
        serialized_player_state.append(strstream.str());
    }

    for (auto const& [unique_client_id, player_state] : game_state.m_dead_player_states) {
        serialized_player_state.append(player_state.display_name);
        serialized_player_state.append(":");

        float x = player_state.position.x;
        float y = player_state.position.y;

        std::stringstream ss;
        ss << x << ":" << y << ";";
        serialized_player_state.append(ss.str());
    }

    return serialized_player_state;
}

std::string
Network_protocol::create_init_player_state_message(Game_state const& game_state) noexcept
{
    ///TODO: Add player states, info about pickups
    std::string result{"INIT_PLAYER_STATE " + std::to_string(game_state.m_player_states.size())
                       + ";"};

    for (auto const& [unique_id, player_state] : game_state.m_player_states) {
        result.append(player_state.display_name);
        result.append(":");
        result.append(std::to_string(player_state.skin_id));
        result.append(":");
        result.append(std::to_string(player_state.bombs));
        result.append(":");
        result.append(std::to_string(player_state.range));
        result.append(":");
        result.append(std::to_string(player_state.score));
        result.append(":");
        result.append(player_state.specials.to_string());
        result.append(";");
    }

    return result;
}


std::string Network_protocol::create_respawn_player_message(
    std::string const& player_display_name, std::size_t const invulnerability_millis) noexcept
{
    return {"RESPAWN_PLAYER " + player_display_name + ":" + std::to_string(invulnerability_millis)};
}

std::string Network_protocol::create_update_scores_message(Game_state const& game_state) noexcept
{
    std::string result{"UPDATE_SCORES " + std::to_string(game_state.m_player_states.size()) + ";"};

    for (auto const& [unique_player_id, player_state] : game_state.m_player_states) {
        result.append(player_state.display_name);
        result.append(":");
        result.append(std::to_string(player_state.score));
        result.append(";");
    }

    return result;
}


auto Network_protocol::create_move_player_message(std::string const& client_display_name,
                                                  bool start_or_stop_movement,
                                                  Network_protocol::Direction direction) noexcept
    -> std::string
{
    /// TODO: Should there be a token with the player connection?
    std::string const start_or_stop{start_or_stop_movement ? "B" : "E"};
    return {"MOVE " + std::to_string(static_cast<std::uint8_t>(direction)) + start_or_stop
            + client_display_name};
}

std::string Network_protocol::create_place_message(std::string const& client_display_name) noexcept
{
    return {"PLACE " + client_display_name};
}

std::string Network_protocol::create_place_ack_message(bool const successful_placement,
                                                       std::size_t const new_bomb_count) noexcept
{
    std::string const success{successful_placement ? "T" : "F"};
    return {"PLACE " + success + std::to_string(new_bomb_count)};
}

std::string Network_protocol::create_bomb_spawn_message(Bomb_state const& bomb_state) noexcept
{
    auto const milliseconds_since_epoch = bomb_state.time_placed.time_since_epoch().count();
    return {"BOMB_SPAWN " + std::to_string(bomb_state.grid_position.first) + ":"
            + std::to_string(bomb_state.grid_position.second) + ":"
            + std::to_string(milliseconds_since_epoch)};
}

std::string Network_protocol::create_explode_message(
    std::vector<std::pair<std::size_t, std::size_t>> const& exploded_bomb_positions) noexcept
{
    std::string result{"EXPLODE " + std::to_string(exploded_bomb_positions.size()) + ";"};

    for (auto const& position : exploded_bomb_positions) {
        result.append(std::to_string(position.first));
        result.append(":");
        result.append(std::to_string(position.second));
        result.append(";");
    }

    return result;
}

std::string Network_protocol::create_destroy_box_message(
    std::vector<std::pair<std::size_t, std::size_t>> const& destroyed_box_positions) noexcept
{
    std::string result{"DESTROY_BOX " + std::to_string(destroyed_box_positions.size()) + ";"};

    for (auto const& position : destroyed_box_positions) {
        result.append(std::to_string(position.first));
        result.append(":");
        result.append(std::to_string(position.second));
        result.append(";");
    }

    return result;
}

std::string Network_protocol::create_spawn_explosion_message(
    std::vector<Explosion_state> const& explosion_states) noexcept
{
    std::string result{"SPAWN_EXPL " + std::to_string(explosion_states.size()) + ";"};

    for (auto const& expl_state : explosion_states) {
        result.append(std::to_string(expl_state.grid_middle_point.first));
        result.append(":");
        result.append(std::to_string(expl_state.grid_middle_point.second));

        /// range
        for (unsigned long long i : expl_state.range_up_down_left_right) {
            result.append(":");
            result.append(std::to_string(i));
        }

        result.append(":");
        result.append(std::to_string(expl_state.life_time.count()));

        result.append(";");
    }


    return result;
}

std::string
Network_protocol::create_spawn_pickup_message(std::vector<Pickup_state> const& pickups) noexcept
{
    std::string result{"SPAWN_PICKUP " + std::to_string(pickups.size()) + ";"};

    for (auto const& pickup : pickups) {
        result.append(std::to_string(pickup.grid_position.first));
        result.append(":");
        result.append(std::to_string(pickup.grid_position.second));
        result.append(":");
        result.append(std::to_string(static_cast<std::uint8_t>(pickup.type)));
        result.append(";");
    }

    return result;
}

std::string Network_protocol::create_pick_up_pickup_message(std::string const& player_name,
                                                            Pickup_state const& pickup) noexcept
{
    return {"PICK_UP_PICKUP " + player_name + ";"
            + std::to_string(static_cast<std::uint8_t>(pickup.type)) + ":"
            + std::to_string(pickup.grid_position.first) + ":"
            + std::to_string(pickup.grid_position.second)};
}

std::string
Network_protocol::create_announce_winner_message(std::string const& winner_display_name) noexcept
{
    return {"ANNOUNCE_WINNER " + winner_display_name};
}

std::string Network_protocol::create_restart_game_message() noexcept
{
    return {"RESTART_GAME "};
}


} // namespace bbm
