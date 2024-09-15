/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_BBMANIA_SERVER_H
#define SALMIAC_BBMANIA_SERVER_H

#include "board.h"
#include "client_connection.h"
#include "file_reader.h"
#include "game_state.h"
#include "network_protocol.h"
#include "server_socket.h"

#include "effolkronium/random.hpp"

#include <atomic>
#include <chrono>

namespace bbm {

class Bbmania_server {
public:
    void run() noexcept;

private:
    void handle_message(Network_protocol::Message const& message,
                        sockaddr_in const& sender_addr) noexcept;

    std::vector<Client_connection>::iterator
    find_connection_by_uid(std::string const& display_name) noexcept;

    bool handle_collision(Player_state& player_state,
                          Network_protocol::Direction const direction) const noexcept;

    static std::string generate_connection_uid(std::string const& display_name,
                                               sockaddr_in const& addr) noexcept;

    void generate_board_from_file(std::string const& filepath) noexcept;

    void handle_bombs() noexcept;

    void check_explosion_player_hits() noexcept;

    void check_player_pickup_collision() noexcept;

    void explode_bombs(Bomb_state& bs) noexcept;

    void handle_showdown() noexcept;

    static std::string set_server_ip_from_file(std::string const& filename) noexcept;

    Game_state m_game_state;

    sal::Server_socket m_server_socket{sal::File_reader::read_file("../res/config/bind_ip")};

    std::atomic_bool m_should_continue{true};
    std::atomic_bool m_game_started{false};

    using Random_engine = effolkronium::random_local;

    Random_engine m_rand_engine{};
    std::vector<Client_connection> m_client_connections;

    std::map<std::string, std::string> m_unique_client_id_to_display_name;

    std::vector<std::pair<std::size_t, std::size_t>> m_bombs_exploded_this_tick{};
    std::vector<std::pair<std::size_t, std::size_t>> m_boxes_broken_this_tick{};
    std::vector<Explosion_state> m_explosions_created_this_tick{};
    std::vector<Pickup_state> m_pickups_spawned_this_tick{};

    std::chrono::milliseconds m_bomb_fuse_time{std::chrono::milliseconds{2000}};
    std::chrono::milliseconds m_wall_spawn_time{std::chrono::milliseconds{1000}};
    std::chrono::milliseconds m_explosion_life_time{std::chrono::milliseconds{350}};
    std::chrono::milliseconds m_player_invincibility_period{std::chrono::milliseconds{1500}};

    //float m_player_movement_speed{0.05f};
    float m_player_movement_speed{5.f};
    float m_player_size{0.75f};

    float m_pickup_spawn_chance{0.35f};
    double m_delta_time{0.01};

    std::size_t m_tick_rate{200};

    std::size_t m_game_counter{0};
    std::vector<std::string> m_board_files{
        "../res/boards/first.bbm",  "../res/boards/castle.bbm", "../res/boards/thing.bbm",
        "../res/boards/spider.bbm", "../res/boards/kela.bbm",
    };

    std::string m_current_board_file{"../res/boards/onebox.bbm"};

    std::map<Network_protocol::Direction, glm::vec2> m_movement_chart{
        {Network_protocol::Direction::UP, glm::vec2{0.f, m_player_movement_speed}},
        {Network_protocol::Direction::LEFT, glm::vec2{-m_player_movement_speed, 0.f}},
        {Network_protocol::Direction::DOWN, glm::vec2{0.f, -m_player_movement_speed}},
        {Network_protocol::Direction::RIGHT, glm::vec2{m_player_movement_speed, 0.f}},
    };
};


} // namespace bbm


#endif //SALMIAC_BBMANIA_SERVER_H
