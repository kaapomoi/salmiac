#include "bbmania_server.h"

#include "fmt/format.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <queue>
#include <set>

namespace bbm {

void Bbmania_server::run() noexcept
{
    std::cout << "Server says hello.\n";

    auto current_time = std::chrono::high_resolution_clock::now();
    auto goal_time = current_time + std::chrono::milliseconds{10};
    auto goal_2_sec = current_time + m_wall_spawn_time;

    auto does_at_least_one_box_exist = [&]() -> bool {
        auto board_size_maybe = m_game_state.m_board.size();
        if (board_size_maybe) {
            auto [board_w, board_h] = board_size_maybe.value();
            for (int i = 0; i < board_h; ++i) {
                for (int j = 0; j < board_w; ++j) {
                    if (m_game_state.m_board.at(j, i) == Cell::Type::box) {
                        return true;
                    }
                }
            }
        }

        return false;
    };


    auto prev_update = std::chrono::high_resolution_clock::now();

    /// Listen for messages indefinitely
    while (m_should_continue) {
        /// Sleep for some time to keep 10 millisecond intervals.
        current_time = std::chrono::high_resolution_clock::now();
        if (current_time > goal_time) {
            auto time_since_previous_update = current_time - prev_update;

            m_delta_time = std::chrono::duration_cast<std::chrono::duration<double>>(
                               time_since_previous_update)
                               .count();

            prev_update = current_time;
            goal_time = current_time + std::chrono::microseconds{1'000'000 / m_tick_rate};

            auto maybe_packet = m_server_socket.receive();
            if (maybe_packet) {
                std::cout << "Message from " << maybe_packet->addr_to_str() << ":"
                          << maybe_packet->address.sin_port << " = " << maybe_packet->message
                          << "\n";

                /// Check the message type
                auto maybe_message = Network_protocol::parse_message(maybe_packet->message);
                if (maybe_message) {
                    /// Based on the message type, do an action, e.g. CONNECT, MOVE, PLACE, etc.
                    handle_message(maybe_message.value(), maybe_packet->address);
                }
            }

            /// Game not started yet, check if all the players are ready
            if (!m_game_started) {
                if (!m_client_connections.empty()
                    && !std::any_of(
                        m_client_connections.begin(), m_client_connections.end(),
                        [&](const Client_connection& cc) { return !cc.ready_to_start_game; })) {

                    for (auto const& client_connection : m_client_connections) {
                        m_server_socket.send({client_connection.address,
                                              Network_protocol::create_restart_game_message()});
                    }

                    m_game_state.reset();
                    generate_board_from_file(m_current_board_file);

                    std::cout << "Board generated, state:\n";
                    m_game_state.m_board.print_board();

                    // Start the game if all the players are ready.
                    m_game_started = true;

                    std::uint32_t player_index{0};

                    // Create player objects based on client connection names
                    for (auto const& client_connection : m_client_connections) {
                        auto pos = m_game_state.m_board.spawn_positions().at(player_index);

                        m_game_state.m_player_states.insert(
                            {client_connection.unique_client_id,
                             {{pos.first, pos.second},
                              ++player_index,
                              1,
                              1,
                              0,
                              {0b00000000},
                              {0b0000},
                              m_unique_client_id_to_display_name.at(
                                  client_connection.unique_client_id),
                              true,
                              {std::chrono::system_clock::now() + m_player_invincibility_period}}});
                    }

                    // Send init game state to all players
                    for (auto const& client_connection : m_client_connections) {
                        m_server_socket.send(
                            {client_connection.address,
                             Network_protocol::create_init_game_state_message(m_game_state)});

                        m_server_socket.send(
                            {client_connection.address,
                             Network_protocol::create_init_player_state_message(m_game_state)});
                    }
                }
            }


            /// See if this is too early to start. Maybe another iteration could be good.
            if (m_game_started) {
                if (current_time > goal_2_sec) {
                    if (!m_game_state.showdown_started) {
                        if (!does_at_least_one_box_exist()) {
                            m_game_state.showdown_started = true;
                        }
                    }
                    else {
                        handle_showdown();

                        if (m_game_state.m_player_states.size() <= 1
                            && !m_game_state.m_dead_player_states.empty()) {
                            /// Game is over, announce the winner to each client
                            m_game_state.showdown_started = false;
                            m_game_started = false;

                            m_current_board_file = m_board_files.at(m_game_counter++);

                            if (!m_game_state.m_player_states.empty()) {
                                /// Move the player away from the screen
                                m_game_state.m_dead_player_states.insert(
                                    m_game_state.m_player_states.begin(),
                                    m_game_state.m_player_states.end());
                                m_game_state.m_player_states.clear();
                            }

                            std::string winner_name{"no one"};
                            std::int32_t max_score{-999};
                            for (auto const& [unique_client_id, player_state] :
                                 m_game_state.m_dead_player_states) {
                                if (player_state.score > max_score) {
                                    winner_name = player_state.display_name;
                                    max_score = player_state.score;
                                }
                            }

                            for (auto& client_connection : m_client_connections) {
                                client_connection.ready_to_start_game = false;

                                m_server_socket.send(
                                    {{client_connection.address},
                                     Network_protocol::create_announce_winner_message(
                                         winner_name)});
                            }
                        }
                    }
                    goal_2_sec = current_time + m_wall_spawn_time;
                }


                /// Start player movements
                /// Adjust movement to time step
                auto movement_chart_this_tick = m_movement_chart;
                for (auto& [direction, movement] : movement_chart_this_tick) {
                    movement *= m_delta_time;
                }

                /// Move all players according to their movement inputs
                for (auto& [name, player_state] : m_game_state.m_player_states) {
                    for (auto const& [direction, movement] : movement_chart_this_tick) {
                        if (player_state.current_movement.test(
                                static_cast<std::uint8_t>(direction))) {
                            player_state.position += movement;
                            if (handle_collision(player_state, direction)) {
                                player_state.position -= movement;
                            }
                        }
                    }
                }
                /// End player movements

                /// Explode bombs when their fuses run out.
                handle_bombs();

                /// Check explosion - player hits
                check_explosion_player_hits();

                /// Check pickup - player hits
                check_player_pickup_collision();

                /// Delete bomb objects from existence.
                std::erase_if(m_game_state.m_bomb_states, [&](Bomb_state const& bs) -> bool {
                    return bs.time_placed + m_bomb_fuse_time < std::chrono::system_clock::now()
                           || bs.has_exploded;
                });

                std::erase_if(
                    m_game_state.m_explosion_states, [&](Explosion_state const& es) -> bool {
                        return es.time_placed + es.life_time < std::chrono::system_clock::now();
                    });

                std::erase_if(m_game_state.m_pickup_states,
                              [&](Pickup_state const& ps) -> bool { return ps.picked_up; });

                std::string const player_positions_for_all{
                    Network_protocol::create_player_position_message(m_game_state)};

                std::string const explosion_spawns_for_all{
                    Network_protocol::create_spawn_explosion_message(
                        m_explosions_created_this_tick)};

                std::string const bomb_explosions_for_all{
                    Network_protocol::create_explode_message(m_bombs_exploded_this_tick)};

                std::string const box_breaks_for_all{
                    Network_protocol::create_destroy_box_message(m_boxes_broken_this_tick)};

                std::string const spawned_pickups_for_all{
                    Network_protocol::create_spawn_pickup_message(m_pickups_spawned_this_tick)};

                // Update the player states via PLAYER_STATE message
                for (auto const& client_connection : m_client_connections) {
                    /// Send player positions to each client.
                    m_server_socket.send({{client_connection.address}, player_positions_for_all});

                    if (!m_explosions_created_this_tick.empty()) {
                        // Inform clients about explosion objects
                        m_server_socket.send(
                            {{client_connection.address}, explosion_spawns_for_all});
                    }

                    if (!m_bombs_exploded_this_tick.empty()) {
                        /// Inform clients about explosions
                        m_server_socket.send(
                            {{client_connection.address}, bomb_explosions_for_all});

                        if (m_game_state.m_player_states.contains(
                                client_connection.unique_client_id)) {

                            m_server_socket.send(
                                {{client_connection.address},
                                 Network_protocol::create_update_scores_message(m_game_state)});

                            /// Send bomb count to each player
                            m_server_socket.send(
                                {client_connection.address,
                                 Network_protocol::create_place_ack_message(
                                     true, m_game_state.m_player_states
                                               .at(client_connection.unique_client_id)
                                               .bombs)});
                        }
                    }

                    if (!m_boxes_broken_this_tick.empty()) {
                        // Inform clients about broken boxes
                        m_server_socket.send({{client_connection.address}, box_breaks_for_all});
                    }

                    if (!m_pickups_spawned_this_tick.empty()) {
                        m_server_socket.send(
                            {{client_connection.address}, spawned_pickups_for_all});
                    }
                }

                m_bombs_exploded_this_tick.clear();
                m_boxes_broken_this_tick.clear();
                m_explosions_created_this_tick.clear();
                m_pickups_spawned_this_tick.clear();
            }
        }
    }
}

bool Bbmania_server::handle_collision(Player_state& player_state,
                                      Network_protocol::Direction const direction) const noexcept
{
    struct Rect {
        float left{0.f};
        float right{0.f};
        float top{0.f};
        float bot{0.f};

        explicit Rect(glm::vec2 const& mid, glm::vec2 const& size)
        {
            left = mid.x - size.x / 2;
            right = mid.x + size.x / 2;
            top = mid.y + size.y / 2;
            bot = mid.y - size.y / 2;
        }
    };

    auto check_rect_collision = [](Rect const& a, Rect const& b) -> bool {
        return !(a.left > b.right    // AisToTheRightOfB
                 || a.right < b.left // AisToTheLeftOfB
                 || a.bot > b.top    // AisAboveB
                 || a.top < b.bot);  // AisBelowB
    };

    static constexpr float epsilon{0.00f};
    float const player_wiggle_room{(1.f - m_player_size) * 0.5f};
    float const p_x = player_state.position.x;
    float const p_y = player_state.position.y;
    std::size_t const grid_x{static_cast<size_t>(std::round(p_x))};
    std::size_t const grid_y{static_cast<size_t>(std::round(p_y))};
    float const p_x_delta = p_x - grid_x;
    float const p_y_delta = p_y - grid_y;
    glm::vec2 const wall_size{1.f};

    std::vector<std::pair<std::size_t, std::size_t>> adjacent_positions{
        {grid_x - 1, grid_y - 1}, {grid_x, grid_y - 1},    {grid_x + 1, grid_y - 1},
        {grid_x - 1, grid_y},     {grid_x + 1, grid_y},    {grid_x - 1, grid_y + 1},
        {grid_x, grid_y + 1},     {grid_x + 1, grid_y + 1}};

    std::vector<Rect> adjacent_walls{
        Rect{{grid_x - 1, grid_y - 1}, {wall_size}}, Rect{{grid_x, grid_y - 1}, {wall_size}},
        Rect{{grid_x + 1, grid_y - 1}, {wall_size}}, Rect{{grid_x - 1, grid_y}, {wall_size}},
        Rect{{grid_x + 1, grid_y}, {wall_size}},     Rect{{grid_x - 1, grid_y + 1}, {wall_size}},
        Rect{{grid_x, grid_y + 1}, {wall_size}},     Rect{{grid_x + 1, grid_y + 1}, {wall_size}}};

    std::set<std::pair<std::size_t, std::size_t>> bomb_positions{};
    for (auto const& bomb_state : m_game_state.m_bomb_states) {
        bomb_positions.insert(bomb_state.grid_position);
    }

    /// Handle wall collisions
    for (auto const& wall_pos : adjacent_positions) {
        if (m_game_state.m_board.at(wall_pos.first, wall_pos.second) != Cell::Type::empty) {
            Rect wall_rect{{wall_pos.first, wall_pos.second}, {wall_size}};
            Rect player_rect{glm::vec2{p_x, p_y}, glm::vec2{m_player_size}};
            if (check_rect_collision(player_rect, wall_rect)) {
                return true;
            }
        }
    }

    // Handle bomb collisions
    if (direction == Network_protocol::Direction::UP) {
        if (p_y_delta > 0.0f) {
            /// up is nearest
            /// Straight above
            if (bomb_positions.contains({grid_x, grid_y + 1})) {
                if (abs(p_y_delta) + epsilon >= player_wiggle_room) {
                    // Collision.
                    return true;
                }
            }
        }
    }
    else if (direction == Network_protocol::Direction::DOWN) {
        if (p_y_delta > 0.0f) {
            /// up is nearest
            return false;
        }

        /// Straight below
        if (bomb_positions.contains({grid_x, grid_y - 1})) {
            if (abs(p_y_delta) + epsilon >= player_wiggle_room) {
                // Collision.
                return true;
            }
        }
    }
    else if (direction == Network_protocol::Direction::LEFT) {
        if (p_x_delta > 0.0f) {
            /// right is nearest
            return false;
        }

        /// Straight left
        if (bomb_positions.contains({grid_x - 1, grid_y})) {
            if (abs(p_x_delta) + epsilon >= player_wiggle_room) {
                // Collision.
                return true;
            }
        }
    }
    else {
        if (p_x_delta > 0.0f) {
            /// Right is nearest
            /// Straight right
            if (bomb_positions.contains({grid_x + 1, grid_y})) {
                if (abs(p_x_delta) + epsilon >= player_wiggle_room) {
                    // Collision.
                    return true;
                }
            }
        }
    }

    return false;
}

void Bbmania_server::handle_message(Network_protocol::Message const& message,
                                    sockaddr_in const& sender_addr) noexcept
{
    switch (message.type) {
    case Network_protocol::Message::Type::CONNECT: {
        /// Check existing client connections for this address, if not present, add the new client.
        std::string const client_display_name{message.data};

        if (m_unique_client_id_to_display_name.contains(client_display_name)) {
            /// Player already connected
            std::cout << client_display_name << " is already connected.\n";

            m_server_socket.send({sender_addr, "Client already connected."});
        }
        else {
            std::cout << client_display_name << " added to the players list.\n";
            std::string const unique_client_id{
                generate_connection_uid(client_display_name, sender_addr)};

            m_client_connections.emplace_back(sender_addr, unique_client_id);
            m_unique_client_id_to_display_name.insert({unique_client_id, client_display_name});

            m_server_socket.send(
                {sender_addr, Network_protocol::create_connect_ack_message(unique_client_id)});
        }
    } break;


    case Network_protocol::Message::Type::VOTE_START: {
        std::string const ready_or_unready{message.data.front()};
        std::string const unique_client_id{message.data.substr(1)};
        if (!m_game_started) {
            if (find_connection_by_uid(unique_client_id) == m_client_connections.end()) {
                /// Player not found, early return.
                return;
            }

            if (ready_or_unready == "R") {
                find_connection_by_uid(unique_client_id)->ready_to_start_game = true;
                m_server_socket.send({sender_addr, Network_protocol::create_vote_start_ack_message(
                                                       unique_client_id, true)});
            }
            else if (ready_or_unready == "U") {
                find_connection_by_uid(unique_client_id)->ready_to_start_game = false;
                m_server_socket.send({sender_addr, Network_protocol::create_vote_start_ack_message(
                                                       unique_client_id, false)});
            }
        }
    } break;


    case Network_protocol::Message::Type::DISCONNECT: {
        /// Check existing client connections for this address, if not present, add the new client.
        std::string const unique_client_id{message.data};
        if (find_connection_by_uid(unique_client_id) == m_client_connections.end()) {
            std::cout << unique_client_id << " is not connected.\n";

            m_server_socket.send({sender_addr, "Connection didn't exist."});
            /// Early return
            return;
        }

        std::cout << unique_client_id << " wants to disconnect.\n";
        /// Remove matching Player from the player list.
        m_client_connections.erase(
            std::remove_if(m_client_connections.begin(), m_client_connections.end(),
                           [unique_client_id, sender_addr](Client_connection const& c) -> bool {
                               return unique_client_id == c.unique_client_id
                                      && sender_addr.sin_addr.S_un.S_addr
                                             == c.address.sin_addr.S_un.S_addr
                                      && sender_addr.sin_port == c.address.sin_port;
                           }),
            m_client_connections.end());

        std::cout << unique_client_id << " has been disconnected.\n";

        m_server_socket.send(
            {sender_addr, Network_protocol::create_disconnect_ack_message(unique_client_id)});

    } break;


    case Network_protocol::Message::Type::MOVE: {
        Network_protocol::Direction const direction{
            static_cast<std::uint8_t>(message.data.front() - '0')};
        bool const start_moving{message.data.at(1) == 'B'};
        std::string const unique_client_id{message.data.substr(2)};

        if (find_connection_by_uid(unique_client_id) != m_client_connections.end()) {
            if (m_game_state.m_player_states.contains(unique_client_id)) {
                if (start_moving) {
                    m_game_state.m_player_states.at(unique_client_id)
                        .current_movement[static_cast<std::uint8_t>(direction)] = true;
                }
                else {
                    m_game_state.m_player_states.at(unique_client_id)
                        .current_movement[static_cast<std::uint8_t>(direction)] = false;
                }
            }
        }

    } break;
    case Network_protocol::Message::Type::PLACE: {
        std::string const unique_client_id{message.data};

        if (find_connection_by_uid(unique_client_id) != m_client_connections.end()) {
            if (m_game_state.m_player_states.contains(unique_client_id)) {
                Player_state& p{m_game_state.m_player_states.at(unique_client_id)};
                if (p.bombs > 0) {
                    /// Place could be possible
                    std::size_t const grid_x{static_cast<size_t>(std::round(p.position.x))};
                    std::size_t const grid_y{static_cast<size_t>(std::round(p.position.y))};

                    /// Check if the cell is empty
                    if (m_game_state.m_board.at(grid_x, grid_y) == Cell::Type::empty) {
                        /// It's empty, is there another bomb already there?
                        if (std::any_of(
                                m_game_state.m_bomb_states.begin(),
                                m_game_state.m_bomb_states.end(),
                                [&](Bomb_state const& bs) -> bool {
                                    return bs.grid_position
                                               == std::pair<std::size_t, std::size_t>{grid_x,
                                                                                      grid_y}
                                           && !bs.has_exploded;
                                })) {
                            /// Can't place, send a message back with failure
                            /// Early return
                            m_server_socket.send(
                                {sender_addr,
                                 Network_protocol::create_place_ack_message(false, p.bombs)});
                            return;
                        }

                        /// Can place. Place it.
                        m_game_state.m_bomb_states.push_back({{grid_x, grid_y},
                                                              p.range,
                                                              std::chrono::system_clock::now(),
                                                              unique_client_id});

                        p.bombs--;

                        m_server_socket.send(
                            {sender_addr,
                             Network_protocol::create_place_ack_message(true, p.bombs)});

                        /// Send message to all clients about the new bomb
                        for (auto const& client_connection : m_client_connections) {
                            m_server_socket.send({client_connection.address,
                                                  Network_protocol::create_bomb_spawn_message(
                                                      m_game_state.m_bomb_states.back())});
                        }
                    }
                }
            }
        }

    } break;
    case Network_protocol::Message::Type::INIT_GAME_STATE:
        break;
    case Network_protocol::Message::Type::DESTROY_BOX:
        break;
    case Network_protocol::Message::Type::SPAWN_PICKUP:
        break;
    case Network_protocol::Message::Type::PLAYER_POSITION:
        break;
    case Network_protocol::Message::Type::INIT_PLAYER_STATE:
        break;
    case Network_protocol::Message::Type::BOMB_SPAWN:
        break;
    case Network_protocol::Message::Type::EXPLODE:
        break;
    case Network_protocol::Message::Type::SPAWN_EXPL:
        break;
    }
}

void Bbmania_server::explode_bombs(Bomb_state& initial_bomb) noexcept
{
    auto cell_at = [&](Bomb_state& b, std::size_t x, std::size_t y) -> Cell::Type {
        return m_game_state.m_board.at(b.grid_position.first + x, b.grid_position.second + y);
    };

    std::vector<std::pair<std::size_t, glm::ivec2>> const directions{
        {0, {0, 1}}, {1, {0, -1}}, {2, {-1, 0}}, {3, {1, 0}}};

    std::queue<std::pair<std::size_t, std::size_t>> bomb_positions_to_explode{
        {initial_bomb.grid_position}};

    auto time_now = std::chrono::system_clock::now();

    while (!bomb_positions_to_explode.empty()) {
        auto position = bomb_positions_to_explode.front();
        bomb_positions_to_explode.pop();
        auto find_result =
            std::find_if(m_game_state.m_bomb_states.begin(), m_game_state.m_bomb_states.end(),
                         [&](Bomb_state& bs) -> bool { return bs.grid_position == position; });

        if (find_result != m_game_state.m_bomb_states.end()) {
            Bomb_state& bs = (*find_result);

            if (!bs.has_exploded) {
                m_game_state.m_player_states.at(bs.placer_unique_id).bombs++;

                /// Increment their bomb counter
                bs.has_exploded = true;
                m_bombs_exploded_this_tick.push_back(bs.grid_position);
            }
            else {
                continue;
            }

            Explosion_state explosion_state{
                bs.grid_position, {}, time_now, m_explosion_life_time, bs.placer_unique_id};

            for (auto const& [dir_identifier, direction] : directions) {
                for (int i = 1; i <= bs.explosion_range; ++i) {
                    auto x = bs.grid_position.first + (i * direction.x);
                    auto y = bs.grid_position.second + (i * direction.y);

                    /// Guard for invalid accesses.
                    if (x < 0 || y < 0 || x >= m_game_state.m_board.size()->first
                        || y >= m_game_state.m_board.size()->second) {
                        explosion_state.range_up_down_left_right.at(dir_identifier) = i - 1;
                        continue;
                    }

                    if (cell_at(bs, i * direction.x, i * direction.y) == Cell::Type::wall) {
                        explosion_state.range_up_down_left_right.at(dir_identifier) = i - 1;
                        break;
                    }
                    else if (cell_at(bs, i * direction.x, i * direction.y) == Cell::Type::box) {
                        /// Destroy the box
                        m_game_state.m_board.at_mut(x, y).type = Cell::Type::empty;

                        m_boxes_broken_this_tick.emplace_back(x, y);

                        /// Bomb count++ or explosion range++
                        auto spawn_pickup = m_rand_engine.get(0.f, 1.f);
                        if (spawn_pickup <= m_pickup_spawn_chance) {
                            auto which_pickup = m_rand_engine.get(0.f, 1.f);
                            if (which_pickup <= 0.60f) {
                                /// Spawn bomb++
                                m_game_state.m_pickup_states.push_back(
                                    {{x, y}, Pickup_state::Type::BOMB_COUNT_PLUS});
                                m_pickups_spawned_this_tick.push_back(
                                    {{x, y}, Pickup_state::Type::BOMB_COUNT_PLUS});
                            }
                            else {
                                /// Spawn range++
                                m_game_state.m_pickup_states.push_back(
                                    {{x, y}, Pickup_state::Type::RANGE_PLUS});
                                m_pickups_spawned_this_tick.push_back(
                                    {{x, y}, Pickup_state::Type::RANGE_PLUS});
                            }
                        }

                        explosion_state.range_up_down_left_right.at(dir_identifier) = i;

                        break;
                    }
                    else if (cell_at(bs, i * direction.x, i * direction.y) == Cell::Type::empty) {
                        explosion_state.range_up_down_left_right.at(dir_identifier) = i;

                        bool found_bomb{false};
                        for (auto const& bomb_state : m_game_state.m_bomb_states) {
                            if (bomb_state.grid_position
                                    == std::pair<std::size_t, std::size_t>{x, y}
                                && !bomb_state.has_exploded) {

                                /// This new bomb should also explode.
                                bomb_positions_to_explode.emplace(x, y);
                                found_bomb = true;


                                break;
                            }
                        }
                        if (found_bomb) {
                            break;
                        }
                    }
                }
            }

            m_game_state.m_explosion_states.push_back(explosion_state);
            m_explosions_created_this_tick.push_back(explosion_state);
        }
    }
}

void Bbmania_server::handle_bombs() noexcept
{
    /// This function check for each bomb that should explode, destroys the boxes within reach
    std::for_each(m_game_state.m_bomb_states.begin(), m_game_state.m_bomb_states.end(),
                  [&](Bomb_state& bs) {
                      if (bs.time_placed + m_bomb_fuse_time < std::chrono::system_clock::now()) {
                          if (m_game_state.m_player_states.contains(bs.placer_unique_id)) {

                              /// Destroy boxes that are in range
                              explode_bombs(bs);

                              /// Create Explosion object??
                          }
                      }
                  });
}

void Bbmania_server::check_explosion_player_hits() noexcept
{
    std::vector<std::pair<std::size_t, glm::ivec2>> const directions{
        {0, {0, 1}}, {1, {0, -1}}, {2, {-1, 0}}, {3, {1, 0}}};

    auto time_now = std::chrono::system_clock::now();

    for (auto& [unique_player_id, player_state] : m_game_state.m_player_states) {
        bool player_hit{false};

        if (player_state.invincible_until_this_point_point > time_now) {
            continue;
        }

        for (auto const& es : m_game_state.m_explosion_states) {
            std::size_t const player_grid_x{
                static_cast<size_t>(std::round(player_state.position.x))};
            std::size_t const player_grid_y{
                static_cast<size_t>(std::round(player_state.position.y))};

            if (es.grid_middle_point
                == std::pair<std::size_t, std::size_t>{player_grid_x, player_grid_y}) {
                /// Player hit.
                player_hit = true;
            }
            else {
                for (auto const& [dir_id, direction] : directions) {
                    /// Up down left right
                    for (int i = 0; i <= es.range_up_down_left_right.at(dir_id); ++i) {
                        std::size_t const check_pos_x{es.grid_middle_point.first
                                                      + (direction.x * i)};
                        std::size_t const check_pos_y{es.grid_middle_point.second
                                                      + (direction.y * i)};

                        if (check_pos_x == player_grid_x && check_pos_y == player_grid_y) {
                            // Player hit.
                            player_hit = true;
                            break;
                        }
                    }
                    if (player_hit) {
                        break;
                    }
                }
            }
            if (player_hit) {
                /// Give the explosion owner a point
                if (m_game_state.m_player_states.contains(es.placer_unique_id)) {
                    if (es.placer_unique_id != unique_player_id) {
                        /// Don't give a player points for exploding themselves.
                        m_game_state.m_player_states.at(es.placer_unique_id).score++;
                    }
                    player_state.score--;

                    if (m_game_state.showdown_started) {
                        /// Remove the player from this round
                        player_state.alive = false;
                        /// Move the player away from the screen
                        player_state.position.x = 999;
                        player_state.position.y = 999;

                        m_game_state.m_dead_player_states.insert({unique_player_id, player_state});
                    }
                    else {
                        player_state.position.x = m_game_state.m_board.spawn_positions()
                                                      .at(player_state.skin_id - 1)
                                                      .first;
                        player_state.position.y = m_game_state.m_board.spawn_positions()
                                                      .at(player_state.skin_id - 1)
                                                      .second;

                        player_state.invincible_until_this_point_point =
                            time_now + m_player_invincibility_period;

                        for (auto const& client_connection : m_client_connections) {
                            m_server_socket.send({{client_connection.address},
                                                  Network_protocol::create_respawn_player_message(
                                                      player_state.display_name,
                                                      m_player_invincibility_period.count())});
                        }
                    }
                }
                break;
            }
        }
    }

    /// Remove the player from this round
    std::erase_if(m_game_state.m_player_states, [](auto& ps) -> bool { return !ps.second.alive; });
}

void Bbmania_server::check_player_pickup_collision() noexcept
{
    for (auto& [unique_id, player_state] : m_game_state.m_player_states) {
        for (auto& ps : m_game_state.m_pickup_states) {
            if (!ps.picked_up) {
                std::size_t const player_grid_x{
                    static_cast<size_t>(std::round(player_state.position.x))};
                std::size_t const player_grid_y{
                    static_cast<size_t>(std::round(player_state.position.y))};

                if (ps.grid_position
                    == std::pair<std::size_t, std::size_t>{player_grid_x, player_grid_y}) {
                    /// Player got pickup.
                    if (ps.type == Pickup_state::Type::BOMB_COUNT_PLUS) {
                        player_state.bombs++;
                    }
                    else if (ps.type == Pickup_state::Type::RANGE_PLUS) {
                        player_state.range++;
                    }

                    std::string const pickup_picked_up_notification_for_all_clients{
                        Network_protocol::create_pick_up_pickup_message(player_state.display_name,
                                                                        ps)};

                    ps.picked_up = true;

                    for (auto const& client_connection : m_client_connections) {
                        m_server_socket.send({{client_connection.address},
                                              pickup_picked_up_notification_for_all_clients});
                    }
                }
            }
        }
    }
}

void Bbmania_server::handle_showdown() noexcept
{
    auto find_next_empty_space = [&]() -> std::optional<std::pair<std::size_t, std::size_t>> {
        auto board_size_maybe = m_game_state.m_board.size();
        if (board_size_maybe) {
            auto [board_w, board_h] = board_size_maybe.value();
            for (int i = 0; i < board_h; ++i) {
                for (int j = 0; j < board_w; ++j) {
                    if (m_game_state.m_board.at(j, i) == Cell::Type::empty) {
                        return {{j, i}};
                    }
                }
            }
        }

        /// Nullopt
        return {};
    };

    auto maybe_empty_grid_pos = find_next_empty_space();
    if (maybe_empty_grid_pos) {
        auto empty_grid_x = maybe_empty_grid_pos->first;
        auto empty_grid_y = maybe_empty_grid_pos->second;

        bool wall_can_be_placed{true};
        for (auto const& bs : m_game_state.m_bomb_states) {
            if (bs.grid_position == maybe_empty_grid_pos) {
                wall_can_be_placed = false;
            }
        }

        /// Check player collision
        if (wall_can_be_placed) {
            for (auto const& ps : m_game_state.m_player_states) {
                auto px = static_cast<std::size_t>(std::round(ps.second.position.x));
                auto py = static_cast<std::size_t>(std::round(ps.second.position.y));

                if (px == empty_grid_x && py == empty_grid_y) {
                    /// Player hit, can't place.
                    wall_can_be_placed = false;
                }
            }
        }
        if (wall_can_be_placed) {
            m_game_state.m_board.at_mut(empty_grid_x, empty_grid_y).type = Cell::Type::wall;

            std::string const spawn_wall_message_for_all_clients{
                Network_protocol::create_spawn_wall_message(maybe_empty_grid_pos.value())};

            for (auto const& client_connection : m_client_connections) {
                m_server_socket.send(
                    {{client_connection.address}, spawn_wall_message_for_all_clients});
            }
        }
    }
}


std::vector<Client_connection>::iterator
Bbmania_server::find_connection_by_uid(std::string const& unique_client_id) noexcept
{
    return std::find_if(m_client_connections.begin(), m_client_connections.end(),
                        [unique_client_id](Client_connection const& c) -> bool {
                            return unique_client_id == c.unique_client_id;
                        });
}

std::string Bbmania_server::generate_connection_uid(std::string const& display_name,
                                                    sockaddr_in const& addr) noexcept
{
    auto name_hash = std::hash<std::string>{}(display_name);
    auto addr_hash = std::hash<std::uint32_t>{}(addr.sin_addr.S_un.S_addr);
    auto time_point_hash = std::hash<std::size_t>{}(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());

    auto final_hash = name_hash ^ (addr_hash ^ (time_point_hash << 1));
    auto final_string = fmt::format("{:x}", final_hash);
    final_string = final_string.substr(0, 8);

    return final_string;
}

void Bbmania_server::generate_board_from_file(std::string const& filepath) noexcept
{
    m_game_state.m_board = Board(filepath);
}


} // namespace bbm
