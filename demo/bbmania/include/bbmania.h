/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef BBMANIA_H
#define BBMANIA_H

#include "application.h"
#include "camera_controller.h"
#include "client_socket.h"
#include "file_reader.h"
#include "network_protocol.h"
#include "primitives.h"
#include "text.h"

#include "effolkronium/random.hpp"

#include <chrono>
#include <map>
#include <random>

namespace bbm {


class Bbmania : public sal::Application {
public:
    sal::Application::Exit_code start() noexcept;


    sal::Application::Exit_code run() noexcept;

    void cleanup() noexcept;

private:
    using Random_engine = effolkronium::random_local;
    void run_user_tasks() noexcept final;

    void set_render_model_uniforms(sal::Shader_program& shader) noexcept final;
    void set_user_uniforms_before_render() noexcept final;

    void handle_input() noexcept;

    void receive_network_data() noexcept;

    void handle_queued_network_data() noexcept;

    void blow_up_bombs() noexcept;

    void expire_explosion_objects() noexcept;

    Camera_controller m_camera_controller{};
    std::map<std::string, sal::Shader_program> m_shaders;
    std::map<std::string, sal::Model> m_models;
    std::vector<sal::Font> m_fonts;
    Random_engine m_rand_engine{};

    bool m_connected_to_the_server{false};
    bool m_ready_to_play{false};
    bool m_in_game{false};

    Game_state m_game_state{};
    std::map<std::pair<std::size_t, std::size_t>, entt::entity> m_bomb_pos_to_entity;
    std::map<std::pair<std::size_t, std::size_t>, entt::entity> m_cell_to_entity;
    std::map<std::pair<std::size_t, std::size_t>, entt::entity> m_pickup_to_entity;
    std::map<std::pair<std::size_t, std::size_t>, std::pair<entt::entity, entt::entity>>
        m_explosion_pos_to_entity;
    std::map<std::string, entt::entity> m_player_to_entity;
    std::map<std::string, entt::entity> m_ui_text_entities;

    std::vector<Network_protocol::Message> m_message_queue{};

    std::vector<glm::vec4> m_skins{glm::vec4{1.f},
                                   glm::vec4{1.f, 0.f, 0.f, 1.f},
                                   glm::vec4{0.f, 1.f, 0.f, 1.f},
                                   glm::vec4{0.f, 0.f, 1.f, 1.f},
                                   glm::vec4{1.f, 1.f, 0.f, 1.f},
                                   glm::vec4{0.f, 1.f, 1.f, 1.f},
                                   glm::vec4{1.f, 0.f, 1.f, 1.f}};


    std::string m_client_display_name{"Client"};

    std::string m_unique_client_id{"xxxxxxxx"};

    sal::Client_socket m_client_socket{sal::File_reader::read_file("../../res/config/ip")};
};

} // namespace bbm

#endif
