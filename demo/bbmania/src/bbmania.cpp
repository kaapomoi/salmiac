#include "bbmania.h"

#include "mesh_binder.h"
#include "texture_loader.h"
#include "time_elapsed.h"

#include <numbers>

namespace bbm {

sal::Application::Exit_code Bbmania::start() noexcept
{
    register_keys({GLFW_KEY_W,
                   GLFW_KEY_A,
                   GLFW_KEY_S,
                   GLFW_KEY_D,
                   GLFW_KEY_LEFT_SHIFT,
                   GLFW_KEY_E,
                   GLFW_KEY_Q,
                   GLFW_KEY_R,
                   GLFW_KEY_Z,
                   GLFW_KEY_X,
                   GLFW_KEY_C,
                   GLFW_KEY_V,
                   GLFW_KEY_UP,
                   GLFW_KEY_DOWN,
                   GLFW_KEY_LEFT,
                   GLFW_KEY_RIGHT,
                   GLFW_KEY_B,
                   GLFW_KEY_ESCAPE,
                   GLFW_KEY_LEFT_CONTROL,
                   GLFW_KEY_SPACE,
                   GLFW_KEY_F1},
                  {GLFW_MOUSE_BUTTON_RIGHT});

    return setup(1280, 720);
}


sal::Application::Exit_code Bbmania::run() noexcept
{
    auto v_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
    auto f_str = sal::File_reader::read_file("../res/shaders/bad_lighting_frag.glsl");

    m_shaders.emplace(
        "invert",
        sal::Shader_loader::from_sources(
            v_str, f_str, {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}}, {"material"}));

    auto basic_lighting_str = sal::File_reader::read_file("../res/shaders/basic_lighting.fsh");
    m_shaders.emplace("basic",
                      sal::Shader_loader::from_sources(
                          v_str, basic_lighting_str,
                          {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}}, {"material"}));

    auto blink_lighting_str = sal::File_reader::read_file("../res/shaders/blink_lighting.fsh");
    m_shaders.emplace("blink", sal::Shader_loader::from_sources(
                                   v_str, blink_lighting_str,
                                   {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}},
                                   {"material", "frame"}));

    auto f2_str = sal::File_reader::read_file("../res/shaders/liquid_frag.glsl");
    m_shaders.emplace("liquid",
                      sal::Shader_loader::from_sources(
                          v_str, f2_str, {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}},
                          {"material", "frame"}));

    auto instanced_vert = sal::File_reader::read_file("../res/shaders/instanced_vert.glsl");
    m_shaders.emplace("instanced_basic",
                      sal::Shader_loader::from_sources(instanced_vert, basic_lighting_str,
                                                       {{"in_uv"},
                                                        {"in_normal"},
                                                        {"in_pos"},
                                                        {"in_color"},
                                                        {"in_instance_model_matrix", 4},
                                                        {"in_instance_color"}},
                                                       {"material", "frame"}));

    m_shaders.emplace("instanced_liquid",
                      sal::Shader_loader::from_sources(instanced_vert, f2_str,
                                                       {{"in_uv"},
                                                        {"in_normal"},
                                                        {"in_pos"},
                                                        {"in_color"},
                                                        {"in_instance_model_matrix", 4},
                                                        {"in_instance_color"}},
                                                       {"material", "frame"}));

    std::uint64_t const base_flags = aiProcess_Triangulate | aiProcess_GenNormals
                                     | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes;

    float const scale_factor{1.f};
    /*
    std::string const model_file{"../res/models/bbm_v1/BomberMan_v1.obj"};
    m_models.emplace("bomberman_v1", sal::Model_loader::from_file(model_file, scale_factor,
                                                                  base_flags | aiProcess_FlipUVs));

    std::string const bomb_model_file{"../res/models/bbm_bomb_v1/untitled.obj"};
    m_models.emplace("bomb_v1", sal::Model_loader::from_file(model_file, scale_factor,
                                                             base_flags | aiProcess_FlipUVs));
    */

    sal::Texture disaster_tex{sal::Texture_loader::from_file("../res/textures/disaster.png",
                                                             sal::Texture::Type::diffuse)};
    sal::Mesh const disaster_cube{
        sal::Primitive_factory::cube(glm::vec3{scale_factor}, {disaster_tex})};
    m_models.emplace("disaster_cube", sal::Model{{disaster_cube}});

    sal::Texture wood_tex{
        sal::Texture_loader::from_file("../res/textures/wood.png", sal::Texture::Type::diffuse)};
    sal::Mesh const cube{sal::Primitive_factory::cube(glm::vec3{scale_factor}, {wood_tex})};
    m_models.emplace("wood_cube", sal::Model{{cube}});

    sal::Texture rock_tex{
        sal::Texture_loader::from_file("../res/textures/rock.png", sal::Texture::Type::diffuse)};
    sal::Mesh const rock_cube{sal::Primitive_factory::cube(glm::vec3{scale_factor}, {rock_tex})};
    m_models.emplace("rock_cube", sal::Model{{rock_cube}});

    sal::Texture gravel_tex{
        sal::Texture_loader::from_file("../res/textures/gravel.png", sal::Texture::Type::diffuse)};
    sal::Mesh const gravel_cube{
        sal::Primitive_factory::cube(glm::vec3{scale_factor}, {gravel_tex})};
    m_models.emplace("gravel_cube", sal::Model{{gravel_cube}});

    sal::Texture explosion_tex{
        sal::Texture_loader::from_file("../res/textures/bg.png", sal::Texture::Type::diffuse)};
    sal::Mesh const explosion_cube{
        sal::Primitive_factory::cube(glm::vec3{scale_factor}, {explosion_tex})};
    m_models.emplace("explosion_cube", sal::Model{{explosion_cube}});


    sal::Texture bg_tex{
        sal::Texture_loader::from_file("../res/textures/bg.png", sal::Texture::Type::diffuse)};
    sal::Mesh bg_plane{sal::Primitive_factory::plane(glm::vec3{scale_factor}, {bg_tex})};
    m_models.emplace("bg_plane", sal::Model{{bg_plane}});

    auto text_vert = sal::File_reader::read_file("../res/shaders/basic_text_vert.glsl");
    auto text_frag = sal::File_reader::read_file("../res/shaders/basic_text_frag.glsl");
    m_shaders.emplace("basic_text", sal::Shader_loader::from_sources(
                                        text_vert, text_frag,
                                        {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}},
                                        {"atlas", "color"}));
    m_fonts.emplace_back(m_font_loader.create("../res/fonts/calibri.ttf"));


    /// These are just defaults. The map can be a different size
    auto const board_h{15U};
    auto const board_w{15U};

    sal::Transform const center{glm::vec3{board_w * 0.5f - .5f, board_h * 0.5f - .5f, 0.f},
                                glm::vec3{0.f}, glm::vec3{1.f}};

    /// Text
    /// Connection state UI text
    auto connection_state_entity = m_registry.create();
    sal::Text connection_state_text{"Connected: false", m_fonts.front(), glm::vec2{0},
                                    glm::vec2{0.01f}, glm::vec4{0.8f, 0.8f, 0.8f, 0.5f}};
    m_registry.emplace<sal::Text>(connection_state_entity, connection_state_text);
    m_registry.emplace<sal::Shader_program>(connection_state_entity, m_shaders.at("basic_text"));

    sal::Transform t{glm::vec3{-6.f, 13.5f, 1.5f}, glm::vec3{0.f}, glm::vec3{1.f}};
    m_registry.emplace<sal::Transform>(connection_state_entity, t);
    m_ui_text_entities.insert({"connection_state", connection_state_entity});

    /// Ready status UI text
    auto ready_status_text_entity = m_registry.create();
    sal::Text ready_status_text{"Not ready.", m_fonts.front(), glm::vec2{0}, glm::vec2{0.01f},
                                glm::vec4{0.8f, 0.8f, 0.8f, 0.5f}};
    m_registry.emplace<sal::Text>(ready_status_text_entity, ready_status_text);
    m_registry.emplace<sal::Shader_program>(ready_status_text_entity, m_shaders.at("basic_text"));

    sal::Transform t3{glm::vec3{-6.f, 12.5f, 1.5f}, glm::vec3{0.f}, glm::vec3{1.f}};
    m_registry.emplace<sal::Transform>(ready_status_text_entity, t3);
    m_ui_text_entities.insert({"ready_status", ready_status_text_entity});


    /// Score UI text
    auto score_ui_text_entity = m_registry.create();
    sal::Text score_ui_text{"Score: 0", m_fonts.front(), glm::vec2{0}, glm::vec2{0.01f},
                            glm::vec4{0.8f, 0.8f, 0.8f, 0.5f}};
    m_registry.emplace<sal::Text>(score_ui_text_entity, score_ui_text);
    m_registry.emplace<sal::Shader_program>(score_ui_text_entity, m_shaders.at("basic_text"));

    sal::Transform t4{glm::vec3{-6.f, 11.5f, 1.5f}, glm::vec3{0.f}, glm::vec3{1.f}};
    m_registry.emplace<sal::Transform>(score_ui_text_entity, t4);
    m_ui_text_entities.insert({"score", score_ui_text_entity});


    /// Bomb count UI text
    auto bomb_count_ui_text_entity = m_registry.create();
    sal::Text bomb_count_text{"Bombs: 1", m_fonts.front(), glm::vec2{0}, glm::vec2{0.01f},
                              glm::vec4{0.8f, 0.8f, 0.8f, 0.5f}};
    m_registry.emplace<sal::Text>(bomb_count_ui_text_entity, bomb_count_text);
    m_registry.emplace<sal::Shader_program>(bomb_count_ui_text_entity, m_shaders.at("basic_text"));

    sal::Transform t2{glm::vec3{-6.f, 10.5f, 1.5f}, glm::vec3{0.f}, glm::vec3{1.f}};
    m_registry.emplace<sal::Transform>(bomb_count_ui_text_entity, t2);
    m_ui_text_entities.insert({"bomb_count", bomb_count_ui_text_entity});

    /// Bomb range UI text
    auto bomb_range_ui_text_entity = m_registry.create();
    sal::Text bomb_range_ui_text{"Range: 1", m_fonts.front(), glm::vec2{0}, glm::vec2{0.01f},
                                 glm::vec4{0.8f, 0.8f, 0.8f, 0.5f}};
    m_registry.emplace<sal::Text>(bomb_range_ui_text_entity, bomb_range_ui_text);
    m_registry.emplace<sal::Shader_program>(bomb_range_ui_text_entity, m_shaders.at("basic_text"));

    sal::Transform t5{glm::vec3{-6.f, 9.5f, 1.5f}, glm::vec3{0.f}, glm::vec3{1.f}};
    m_registry.emplace<sal::Transform>(bomb_range_ui_text_entity, t5);
    m_ui_text_entities.insert({"range", bomb_range_ui_text_entity});


    /// Announce winner ui text
    auto announce_winner_text_entity = m_registry.create();
    sal::Text announce_winner_ui_text{"",
                                      m_fonts.front(),
                                      glm::vec2{0},
                                      glm::vec2{0.01f},
                                      glm::vec4{0.8f, 0.8f, 0.8f, 0.5f},
                                      2.f,
                                      true};
    m_registry.emplace<sal::Text>(announce_winner_text_entity, announce_winner_ui_text);
    m_registry.emplace<sal::Shader_program>(announce_winner_text_entity,
                                            m_shaders.at("basic_text"));

    sal::Transform t6{center};
    t6.position.z += 2.f;
    m_registry.emplace<sal::Transform>(announce_winner_text_entity, t6);
    m_ui_text_entities.insert({"announce_winner", announce_winner_text_entity});
    /// End text


    entt::entity camera{m_registry.create()};
    m_registry.emplace<sal::Transform>(camera,
                                       glm::vec3{center.position.x, center.position.y, 20.f},
                                       glm::vec3{0.0f}, glm::vec3{1.0f});
    m_registry.emplace<sal::Camera>(camera, glm::vec3{0.f, 1.f, 0.f}, -90.f, 0.f);

    m_client_display_name.append(std::to_string(
        m_rand_engine.get(std::uint16_t{0}, std::numeric_limits<std::uint16_t>::max())));


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSwapInterval(0);

    m_t_start = std::chrono::high_resolution_clock::now();
    m_t_prev_update = m_t_start;

    while (!m_suggest_close) {
        update();
    }

    return Exit_code::ok;
}

void Bbmania::cleanup() noexcept
{
    m_client_socket.send({{}, Network_protocol::create_disconnect_message(m_client_display_name)});
    for (auto const& shader : m_shaders) {
        glDeleteProgram(shader.second.program_id);
    }
    glfwTerminate();
}


///
/// Private section:
///
void Bbmania::run_user_tasks() noexcept
{
    handle_input();

    bool time_to_draw_frame{false};
    while (!time_to_draw_frame) {
        auto t_now = std::chrono::high_resolution_clock::now();


        /// Update game objects
        receive_network_data();

        if (m_goal_time < t_now) {
            m_goal_time = t_now + std::chrono::microseconds{1000000 / m_target_fps};
            time_to_draw_frame = true;
        }
    }


    glClearColor(0.0f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    handle_queued_network_data();

    /// Update local state
    blow_up_bombs();

    /// Expire explosion graphics
    expire_explosion_objects();

    auto time_now = std::chrono::system_clock::now();

    for (auto const& [player_name, entity] : m_player_to_entity) {
        if (m_game_state.m_player_states.contains(player_name)) {
            if (m_game_state.m_player_states.at(player_name).invincible_until_this_point_point
                > time_now) {
                m_registry.replace<sal::Shader_program>(entity, m_shaders.at("blink"));
            }
            else {
                /// TODO: check if not set, then only set this.
                m_registry.replace<sal::Shader_program>(entity, m_shaders.at("basic"));
            }
        }
    }

    for (auto const& [key, entity] : m_bomb_pos_to_entity) {
        double time_alive{0};
        m_registry.patch<Time_elapsed>(entity, [&](Time_elapsed& time_elapsed_component) {
            time_elapsed_component.value += m_delta_time;
            time_alive = time_elapsed_component.value;
        });

        m_registry.patch<sal::Transform>(entity, [time_alive](sal::Transform& transform) {
            transform.scale =
                glm::vec3{0.75f
                          + static_cast<float>(time_alive) * 0.15f
                                * static_cast<float>(::sin(2.0 * std::numbers::pi * time_alive
                                                           + 0.5 * std::numbers::pi))};
            transform.dirty = true;
        });
    }

    for (auto const& [key, entity] : m_pickup_to_entity) {
        double time_alive{0};

        m_registry.patch<Time_elapsed>(entity, [&](Time_elapsed& time_elapsed_component) {
            time_elapsed_component.value += m_delta_time;
            time_alive = time_elapsed_component.value;
        });

        m_registry.patch<sal::Transform>(entity, [time_alive](sal::Transform& transform) {
            transform.position.z =
                1.f + 0.15f * static_cast<float>(::sin(2.0 * std::numbers::pi * time_alive));
            transform.rotation = glm::vec3{0, time_alive * 50.f, 90.f + time_alive * 50.f};
            transform.dirty = true;
        });
    }

    // Update UI
    for (auto const& [key, entity] : m_ui_text_entities) {
        m_registry.patch<sal::Text>(entity, [this, key](sal::Text& t) {
            if (key == "connection_state") {
                t.set_content(
                    m_client_display_name
                    + std::string{m_connected_to_the_server ? " connected." : " not connected."});
            }
            else if (key == "ready_status") {
                if (m_in_game) {
                    t.set_content("In game");
                }
                else {
                    t.set_content(m_ready_to_play ? "Ready" : "Not ready");
                }
            }

            if (m_game_state.m_player_states.contains(m_client_display_name)) {
                Player_state const& me{m_game_state.m_player_states.at(m_client_display_name)};
                if (key == "bomb_count") {
                    t.set_content("Bombs: " + std::to_string(me.bombs));
                }
                else if (key == "range") {
                    t.set_content("Range: " + std::to_string(me.range));
                }
                else if (key == "score") {
                    t.set_content("Score: " + std::to_string(me.score));
                }
            }
        });
    }
}

void Bbmania::set_user_uniforms_before_render() noexcept
{
    for (auto& name_shader_pair : m_shaders) {
        auto& shader = name_shader_pair.second;
        shader.use();

        auto camera_view = m_registry.view<sal::Transform, sal::Camera>();
        for (auto [entity, transform, camera] : camera_view.each()) {

            if (m_window_width > 0 && m_window_height > 0) {
                glm::mat4 const projection = glm::perspective(
                    glm::radians(camera.zoom()),
                    static_cast<float>(m_window_width) / static_cast<float>(m_window_height), 0.1f,
                    1000.0f);

                glm::mat4 const view = camera.get_view_matrix(transform.position);

                shader.set_uniform("camera_pos", transform.position);
                shader.set_uniform("view", view);
                shader.set_uniform("projection", projection);
            }
        }

        if (shader.has_uniform("frame")) {
            shader.set_uniform<std::int32_t>("frame", static_cast<std::int32_t>(m_frame_counter));
        }

        shader.set_uniform<float>("material.shininess", 64.0f);

        shader.un_use();
    }
}

void Bbmania::set_render_model_uniforms(sal::Shader_program& shader) noexcept {}


void Bbmania::handle_input() noexcept
{
    sal::Mouse_position const m_mouse_delta{m_input_manager.mouse_pos_delta()};
    static constexpr float sensitivity{0.1f};

    double const x_offset{m_mouse_delta.x * sensitivity};
    double const y_offset{m_mouse_delta.y * sensitivity};


    if (m_input_manager.key(GLFW_KEY_LEFT_CONTROL)) {
        auto camera_view = m_registry.view<sal::Transform, sal::Camera>();
        for (auto [entity, transform, camera] : camera_view.each()) {
            m_camera_controller(x_offset, y_offset, m_delta_time, m_window, m_input_manager, camera,
                                transform);
        }
    }

    if (m_input_manager.key(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(m_window.get(), true);
    }
    if (m_input_manager.key(GLFW_KEY_F1)) {
        sal::Log::info("FPS: {}", (1.f / m_delta_time));
    }
    /// Movement
    if (m_input_manager.key_now(GLFW_KEY_UP)) {
        m_client_socket.send({{},
                              Network_protocol::create_move_player_message(
                                  m_unique_client_id, true, Network_protocol::Direction::UP)});
    }
    else if (m_input_manager.key_released_now(GLFW_KEY_UP)) {
        m_client_socket.send({{},
                              Network_protocol::create_move_player_message(
                                  m_unique_client_id, false, Network_protocol::Direction::UP)});
    }

    if (m_input_manager.key_now(GLFW_KEY_LEFT)) {
        m_client_socket.send({{},
                              Network_protocol::create_move_player_message(
                                  m_unique_client_id, true, Network_protocol::Direction::LEFT)});
    }
    else if (m_input_manager.key_released_now(GLFW_KEY_LEFT)) {
        m_client_socket.send({{},
                              Network_protocol::create_move_player_message(
                                  m_unique_client_id, false, Network_protocol::Direction::LEFT)});
    }

    if (m_input_manager.key_now(GLFW_KEY_DOWN)) {
        m_client_socket.send({{},
                              Network_protocol::create_move_player_message(
                                  m_unique_client_id, true, Network_protocol::Direction::DOWN)});
    }
    else if (m_input_manager.key_released_now(GLFW_KEY_DOWN)) {
        m_client_socket.send({{},
                              Network_protocol::create_move_player_message(
                                  m_unique_client_id, false, Network_protocol::Direction::DOWN)});
    }

    if (m_input_manager.key_now(GLFW_KEY_RIGHT)) {
        m_client_socket.send({{},
                              Network_protocol::create_move_player_message(
                                  m_unique_client_id, true, Network_protocol::Direction::RIGHT)});
    }
    else if (m_input_manager.key_released_now(GLFW_KEY_RIGHT)) {
        m_client_socket.send({{},
                              Network_protocol::create_move_player_message(
                                  m_unique_client_id, false, Network_protocol::Direction::RIGHT)});
    }

    if (m_input_manager.key_now(GLFW_KEY_Q)) {
        sal::Log::info("Sending hello");
        m_client_socket.send({{}, "Hello!"});
    }

    /*
    if (m_input_manager.key_now(GLFW_KEY_X)) {
        /// TODO: Make the player input this.
        sal::Log::info("Disconnecting from the server with name {}", m_client_display_name);

        m_client_socket.send(
            {{}, Network_protocol::create_disconnect_message(m_client_display_name)});
    }
    */
    if (m_input_manager.key_now(GLFW_KEY_C)) {
        /// TODO: Make the player input this.
        sal::Log::info("Connecting to the server with name {}", m_client_display_name);

        m_client_socket.send({{}, Network_protocol::create_connect_message(m_client_display_name)});
    }
    if (m_input_manager.key_now(GLFW_KEY_V)) {
        /// TODO: Make the player input this.
        sal::Log::info("Sending READY with name {}", m_unique_client_id);

        m_client_socket.send(
            {{}, Network_protocol::create_vote_start_message(m_unique_client_id, true)});
    }
    if (m_input_manager.key_now(GLFW_KEY_B)) {
        /// TODO: Make the player input this.
        sal::Log::info("Sending UNREADY with name {}", m_unique_client_id);

        m_client_socket.send(
            {{}, Network_protocol::create_vote_start_message(m_unique_client_id, false)});
    }
    if (m_input_manager.key_now(GLFW_KEY_SPACE)) {
        /// TODO: Make the player input this.
        sal::Log::info("Sending PLACE with name {}", m_unique_client_id);

        m_client_socket.send({{}, Network_protocol::create_place_message(m_unique_client_id)});
    }
}

void Bbmania::receive_network_data() noexcept
{
    auto maybe_packet = m_client_socket.receive();
    if (maybe_packet) {
        // sal::Log::info("Message from {}:{} = {}", maybe_packet->addr_to_str(),
        //               maybe_packet->address.sin_port, maybe_packet->message);

        auto message = Network_protocol::parse_message(maybe_packet->message);
        if (message) {
            m_message_queue.push_back(message.value());
        }
    }
}


void Bbmania::handle_queued_network_data() noexcept
{
    for (auto const& message : m_message_queue) {
        if (message.type == Network_protocol::Message::Type::INIT_GAME_STATE) {
            m_game_state.m_board.deserialize_from_network(message.data);
            //b.print_board();

            auto maybe_size = m_game_state.m_board.size();
            if (maybe_size) {
                auto [w, h] = maybe_size.value();

                auto ent = m_registry.create();
                auto model = sal::Model{m_models.at("bg_plane")};

                sal::Transform transform{glm::vec3{static_cast<float>(w) / 2.f - 0.5f,
                                                   static_cast<float>(h) / 2.f - 0.5f, .49f},
                                         glm::vec3{0.f}, glm::vec3{w, h, 1.f}};

                m_registry.emplace<sal::Model>(ent, model);
                m_registry.emplace<sal::Shader_program>(ent, m_shaders.at("liquid"));
                m_registry.emplace<sal::Transform>(ent, transform);


                for (std::size_t y{0}; y < h; y++) {
                    for (std::size_t x{0}; x < w; x++) {
                        if (m_game_state.m_board.at(x, y) == Cell::Type::wall) {
                            auto rock_ent = m_registry.create();
                            auto rock_model = sal::Model{m_models.at("rock_cube")};

                            sal::Transform rock_transform{glm::vec3{x, y, 1.f}, glm::vec3{0.f},
                                                          glm::vec3{1.f}};

                            m_registry.emplace<sal::Model>(rock_ent, rock_model);
                            m_registry.emplace<sal::Shader_program>(rock_ent,
                                                                    m_shaders.at("basic"));
                            m_registry.emplace<sal::Transform>(rock_ent, rock_transform);
                            m_cell_to_entity.insert({{x, y}, rock_ent});
                        }
                        else if (m_game_state.m_board.at(x, y) == Cell::Type::box) {
                            auto wood_ent = m_registry.create();
                            auto wood_model = sal::Model{m_models.at("wood_cube")};

                            sal::Transform wood_transform{glm::vec3{x, y, 1.f}, glm::vec3{0.f},
                                                          glm::vec3{0.9f}};

                            m_registry.emplace<sal::Model>(wood_ent, wood_model);
                            m_registry.emplace<sal::Shader_program>(wood_ent,
                                                                    m_shaders.at("basic"));
                            m_registry.emplace<sal::Transform>(wood_ent, wood_transform);
                            m_cell_to_entity.insert({{x, y}, wood_ent});
                        }
                    }
                }
            }
            m_registry.patch<sal::Text>(m_ui_text_entities.at("announce_winner"),
                                        [](sal::Text& t) -> void { t.set_content(""); });
            m_in_game = true;
        }
        else if (message.type == Network_protocol::Message::Type::DESTROY_BOX) {
            std::istringstream iss{message.data};
            std::size_t amount_of_destroyed_boxes{0};
            iss >> amount_of_destroyed_boxes;
            iss.clear();

            auto pos = message.data.find(';');

            for (int i = 0; i < amount_of_destroyed_boxes; ++i) {
                std::istringstream position_stream{message.data.substr(pos + 1)};
                char delim;
                std::size_t grid_x{0};
                std::size_t grid_y{0};
                position_stream >> grid_x >> delim >> grid_y;

                m_game_state.m_board.at_mut(grid_x, grid_y).type = Cell::Type::empty;
                if (m_cell_to_entity.contains({grid_x, grid_y})) {
                    auto ent = m_cell_to_entity.at({grid_x, grid_y});
                    m_registry.destroy(ent);
                    m_cell_to_entity.erase({grid_x, grid_y});
                }

                pos += message.data.substr(pos + 1).find(';') + 1;
            }
        }
        else if (message.type == Network_protocol::Message::Type::CONNECT) {
            std::string const ack_header{"ACK "};
            if (message.data.find(ack_header) != std::string::npos) {
                m_unique_client_id = message.data.substr(ack_header.size());

                sal::Log::info("Connected to the server. UID: {}", m_unique_client_id);
                m_connected_to_the_server = true;
            }
        }
        else if (message.type == Network_protocol::Message::Type::DISCONNECT) {
            if (message.data == std::string{"ACK " + m_unique_client_id}) {
                for (auto [cell, board_entity] : m_cell_to_entity) {
                    m_registry.destroy(board_entity);
                }
                m_cell_to_entity.clear();
                m_connected_to_the_server = false;
            }
        }
        else if (message.type == Network_protocol::Message::Type::VOTE_START) {
            if (message.data == std::string{"ACK R" + m_unique_client_id}) {
                m_ready_to_play = true;
            }
            else if (message.data == std::string{"ACK U" + m_unique_client_id}) {
                m_ready_to_play = false;
            }
        }
        else if (message.type == Network_protocol::Message::Type::PLAYER_POSITION) {
            std::istringstream iss{message.data};
            std::size_t amount_of_players{0};
            iss >> amount_of_players;
            iss.clear();

            auto pos = message.data.find(';');

            for (int i = 0; i < amount_of_players; ++i) {
                std::istringstream player_namestream{message.data.substr(pos + 1)};
                std::string player_name;
                std::getline(player_namestream, player_name, ':');
                pos += message.data.substr(pos).find(':');

                std::istringstream player_datastream{message.data.substr(pos + 1)};
                float x;
                float y;
                char delim;
                player_datastream >> x >> delim >> y >> delim;

                if (m_game_state.m_player_states.contains(player_name)) {
                    m_game_state.m_player_states.at(player_name).position = {x, y};
                }

                /// TODO: Update actual player states?
                //m_game_state.m_player_states.at(name).position = {x, y};
                if (m_player_to_entity.contains(player_name)) {
                    m_registry.patch<sal::Transform>(m_player_to_entity.at(player_name),
                                                     [x, y](auto& transform) {
                                                         transform.position = glm::vec3{x, y, 1.f};
                                                         transform.dirty = true;
                                                     });
                }

                pos += message.data.substr(pos).find(';');
            }
        }
        else if (message.type == Network_protocol::Message::Type::INIT_PLAYER_STATE) {
            std::istringstream iss{message.data};
            std::size_t amount_of_players{0};
            iss >> amount_of_players;
            iss.clear();

            auto pos = message.data.find(';');

            for (int i = 0; i < amount_of_players; ++i) {
                std::istringstream player_namestream{message.data.substr(pos + 1)};
                std::string player_name;
                std::getline(player_namestream, player_name, ':');
                pos += message.data.substr(pos).find(':');

                std::istringstream player_datastream{message.data.substr(pos + 1)};
                std::uint32_t skin_id{0};
                std::uint32_t bombs{0};
                std::uint32_t range{0};
                std::int32_t score{0};
                std::bitset<8> specials{0x00};
                char delim;
                player_datastream >> skin_id >> delim >> bombs >> delim >> range >> delim >> score
                    >> delim >> specials >> delim;

                if (!m_game_state.m_player_states.contains(player_name)) {
                    m_game_state.m_player_states.insert({player_name, Player_state{{0, 0},
                                                                                   skin_id,
                                                                                   bombs,
                                                                                   range,
                                                                                   score,
                                                                                   specials,
                                                                                   {},
                                                                                   player_name,
                                                                                   true}});
                }

                if (!m_player_to_entity.contains(player_name)) {
                    /// Create player
                    auto player_ent = m_registry.create();
                    sal::Model player_model{m_models.at("wood_cube")};

                    for (auto& mesh : player_model.meshes) {
                        for (auto& vertex : mesh.vertices) {
                            vertex.color =
                                m_skins.at(m_game_state.m_player_states.at(player_name).skin_id);
                        }
                        sal::Mesh_binder::setup(mesh);
                    }

                    sal::Transform player_transform{glm::vec3{0.f, 0.f, 1.f}, glm::vec3{0.f},
                                                    glm::vec3{0.75f}};

                    m_registry.emplace<sal::Model>(player_ent, player_model);
                    m_registry.emplace<sal::Shader_program>(player_ent, m_shaders.at("basic"));
                    m_registry.emplace<sal::Transform>(player_ent, player_transform);
                    m_player_to_entity.insert({player_name, player_ent});
                }

                pos += message.data.substr(pos).find(';');
            }
        }
        else if (message.type == Network_protocol::Message::Type::SPAWN_WALL) {
            std::istringstream iss{message.data};
            std::size_t grid_x{0};
            std::size_t grid_y{0};
            char delim;

            iss >> grid_x >> delim >> grid_y;

            m_game_state.m_board.at_mut(grid_x, grid_y).type = Cell::Type::wall;

            auto rock_ent = m_registry.create();
            auto rock_model = sal::Model{m_models.at("rock_cube")};

            sal::Transform rock_transform{glm::vec3{grid_x, grid_y, 1.f}, glm::vec3{0.f},
                                          glm::vec3{1.f}};

            m_registry.emplace<sal::Model>(rock_ent, rock_model);
            m_registry.emplace<sal::Shader_program>(rock_ent, m_shaders.at("basic"));
            m_registry.emplace<sal::Transform>(rock_ent, rock_transform);
            m_cell_to_entity.insert({{grid_x, grid_y}, rock_ent});
        }
        else if (message.type == Network_protocol::Message::Type::BOMB_SPAWN) {
            std::istringstream iss{message.data};
            std::size_t grid_x{0};
            std::size_t grid_y{0};
            std::size_t count_millis_since_epoch{0};
            char delim;

            iss >> grid_x >> delim >> grid_y >> delim >> count_millis_since_epoch;

            std::chrono::system_clock::time_point const time_point{
                std::chrono::system_clock::duration{count_millis_since_epoch}};

            m_game_state.m_bomb_states.push_back({{grid_x, grid_y}, 0, time_point, ""});

            if (!m_bomb_pos_to_entity.contains({grid_x, grid_y})) {
                /// Create bomb
                auto bomb_ent = m_registry.create();
                sal::Model bomb_model{m_models.at("wood_cube")};

                sal::Transform bomb_transform{glm::vec3{grid_x, grid_y, 1.f}, glm::vec3{0.f},
                                              glm::vec3{0.75f}};

                m_registry.emplace<sal::Model>(bomb_ent, bomb_model);
                m_registry.emplace<sal::Shader_program>(bomb_ent, m_shaders.at("invert"));
                m_registry.emplace<sal::Transform>(bomb_ent, bomb_transform);
                m_registry.emplace<Time_elapsed>(bomb_ent, Time_elapsed{0});
                m_bomb_pos_to_entity.insert({{grid_x, grid_y}, bomb_ent});
            }
        }
        else if (message.type == Network_protocol::Message::Type::PICK_UP_PICKUP) {
            std::istringstream player_namestream{message.data};
            std::string player_name;
            std::getline(player_namestream, player_name, ';');

            auto pos = message.data.find(';');


            std::istringstream iss{message.data.substr(pos + 1)};
            std::uint8_t pickup_type{0};
            std::size_t grid_x{0};
            std::size_t grid_y{0};
            char delim;
            iss >> pickup_type >> delim >> grid_x >> delim >> grid_y;

            Pickup_state::Type type{static_cast<std::uint8_t>(pickup_type - '0')};

            /// Only update UI if this client picked up the pickup.
            if (player_name == m_client_display_name) {
                if (m_game_state.m_player_states.contains(m_client_display_name)) {
                    Player_state& me{m_game_state.m_player_states.at(m_client_display_name)};
                    if (type == Pickup_state::Type::RANGE_PLUS) {
                        me.range++;
                    }
                    else if (type == Pickup_state::Type::BOMB_COUNT_PLUS) {
                        me.bombs++;
                    }
                }
            }

            if (m_pickup_to_entity.contains({grid_x, grid_y})) {
                /// Destroy pickup entity
                auto ent = m_pickup_to_entity.at({grid_x, grid_y});
                m_registry.destroy(ent);
                m_pickup_to_entity.erase({grid_x, grid_y});
            }
        }
        else if (message.type == Network_protocol::Message::Type::PLACE) {
            bool const success{message.data.at(0) == 'T'};
            if (success) {
                if (m_game_state.m_player_states.contains(m_client_display_name)) {
                    std::uint32_t const new_bomb_count{std::stoul(message.data.substr(1))};
                    m_game_state.m_player_states.at(m_client_display_name).bombs = new_bomb_count;
                }
            }
        }
        else if (message.type == Network_protocol::Message::Type::EXPLODE) {
            std::istringstream iss{message.data};
            std::size_t amount_of_exploded_bombs{0};
            iss >> amount_of_exploded_bombs;
            iss.clear();

            auto pos = message.data.find(';');

            for (int i = 0; i < amount_of_exploded_bombs; ++i) {
                std::istringstream position_stream{message.data.substr(pos + 1)};
                char delim;
                std::size_t grid_x{0};
                std::size_t grid_y{0};
                position_stream >> grid_x >> delim >> grid_y;

                std::for_each(m_game_state.m_bomb_states.begin(), m_game_state.m_bomb_states.end(),
                              [grid_x, grid_y](Bomb_state& bs) {
                                  if (bs.grid_position
                                      == std::pair<std::size_t, std::size_t>{grid_x, grid_y}) {
                                      bs.has_exploded = true;
                                  }
                              });

                pos += message.data.substr(pos + 1).find(';') + 1;
            }
        }
        else if (message.type == Network_protocol::Message::Type::RESPAWN_PLAYER) {
            std::istringstream player_namestream{message.data};
            std::string player_name;
            std::getline(player_namestream, player_name, ':');
            std::size_t const pos = message.data.find(':');

            std::istringstream player_datastream{message.data.substr(pos + 1)};
            std::size_t spawn_invulnerability_time{0};
            char delim;
            player_datastream >> spawn_invulnerability_time;

            if (m_game_state.m_player_states.contains(player_name)) {
                std::chrono::system_clock::time_point const time_point{
                    std::chrono::system_clock::now()
                    + std::chrono::milliseconds{spawn_invulnerability_time}};

                m_game_state.m_player_states.at(player_name).invincible_until_this_point_point =
                    time_point;
            }
        }
        else if (message.type == Network_protocol::Message::Type::UPDATE_SCORES) {
            std::istringstream iss{message.data};
            std::size_t amount_of_players{0};
            iss >> amount_of_players;
            iss.clear();

            auto pos = message.data.find(';');

            for (int i = 0; i < amount_of_players; ++i) {
                std::istringstream player_namestream{message.data.substr(pos + 1)};
                std::string player_name;
                std::getline(player_namestream, player_name, ':');
                pos += message.data.substr(pos).find(':');

                std::istringstream player_datastream{message.data.substr(pos + 1)};
                std::int32_t score{0};
                char delim;
                player_datastream >> score;

                if (m_game_state.m_player_states.contains(player_name)) {
                    m_game_state.m_player_states.at(player_name).score = score;
                }

                pos += message.data.substr(pos).find(';');
            }
        }
        else if (message.type == Network_protocol::Message::Type::SPAWN_PICKUP) {
            std::istringstream iss{message.data};
            std::size_t amount_of_spawned_pickups{0};
            iss >> amount_of_spawned_pickups;
            iss.clear();

            auto pos = message.data.find(';');

            for (int i = 0; i < amount_of_spawned_pickups; ++i) {
                std::istringstream data_stream{message.data.substr(pos + 1)};
                char delim;
                std::size_t grid_x{0};
                std::size_t grid_y{0};
                std::uint8_t pickup_type{0};
                data_stream >> grid_x >> delim >> grid_y >> delim >> pickup_type;
                Pickup_state::Type type{static_cast<std::uint8_t>(pickup_type - '0')};

                /// TODO: maybe not needed?
                m_game_state.m_pickup_states.push_back({{grid_x, grid_y}, type});

                /// Create pickup entities
                {
                    auto pickup_ent = m_registry.create();
                    /// TODO: change texture/model/color based on type?

                    if (type == Pickup_state::Type::BOMB_COUNT_PLUS) {
                        sal::Model pickup_model{m_models.at("disaster_cube")};
                        m_registry.emplace<sal::Model>(pickup_ent, pickup_model);
                        m_registry.emplace<sal::Shader_program>(pickup_ent, m_shaders.at("basic"));
                    }
                    else if (type == Pickup_state::Type::RANGE_PLUS) {
                        sal::Model pickup_model{m_models.at("wood_cube")};
                        m_registry.emplace<sal::Model>(pickup_ent, pickup_model);
                        m_registry.emplace<sal::Shader_program>(pickup_ent, m_shaders.at("liquid"));
                    }

                    sal::Transform pickup_transform{glm::vec3{grid_x, grid_y, 1.0f},
                                                    glm::vec3{-90.f, 0.f, 0.f}, glm::vec3{0.5f}};
                    m_registry.emplace<sal::Transform>(pickup_ent, pickup_transform);
                    m_registry.emplace<Time_elapsed>(pickup_ent, Time_elapsed{0});

                    m_pickup_to_entity.insert({{grid_x, grid_y}, pickup_ent});
                }

                pos += message.data.substr(pos + 1).find(';') + 1;
            }
        }
        else if (message.type == Network_protocol::Message::Type::SPAWN_EXPL) {
            std::istringstream iss{message.data};

            std::size_t amount_of_explosion_objects{0};
            iss >> amount_of_explosion_objects;
            iss.clear();

            auto pos = message.data.find(';');

            for (int i = 0; i < amount_of_explosion_objects; ++i) {
                std::istringstream data_stream{message.data.substr(pos + 1)};
                char delim;
                std::size_t grid_x{0};
                std::size_t grid_y{0};
                std::size_t range_up{0};
                std::size_t range_down{0};
                std::size_t range_left{0};
                std::size_t range_right{0};
                std::size_t life_time_millis{0};

                data_stream >> grid_x >> delim >> grid_y >> delim >> range_up >> delim >> range_down
                    >> delim >> range_left >> delim >> range_right >> delim >> life_time_millis;

                std::chrono::system_clock::time_point const time_point{
                    std::chrono::system_clock::now()};

                m_game_state.m_explosion_states.push_back(
                    {{grid_x, grid_y},
                     {range_up, range_down, range_left, range_right},
                     time_point,
                     std::chrono::milliseconds{life_time_millis},
                     ""});

                sal::Log::info(
                    "Got SPAWN_EXPL packet {}, {}", message.data,
                    (time_point + std::chrono::milliseconds{350}).time_since_epoch().count());

                /// Create explosion entities
                {
                    auto explosion_up_down_ent = m_registry.create();
                    sal::Model explosion_model{m_models.at("explosion_cube")};

                    sal::Log::info("Spawned explosion object {}",
                                   static_cast<std::uint32_t>(explosion_up_down_ent));

                    float const explosion_z_pos{1.0f};
                    float const explosion_height{1.0f};

                    auto up_down_length = range_up + range_down + 1;
                    glm::vec3 up_down_mid_point{grid_x,
                                                (grid_y + (range_up * 0.5f) - (range_down * 0.5f)),
                                                explosion_z_pos};
                    sal::Transform explosion_up_down_transform{
                        up_down_mid_point, glm::vec3{0.f},
                        glm::vec3{1, up_down_length, explosion_height}};
                    m_registry.emplace<sal::Model>(explosion_up_down_ent, explosion_model);
                    m_registry.emplace<sal::Shader_program>(explosion_up_down_ent,
                                                            m_shaders.at("invert"));
                    m_registry.emplace<sal::Transform>(explosion_up_down_ent,
                                                       explosion_up_down_transform);


                    auto left_right_length = range_left + range_right + 1;
                    glm::vec3 left_right_mid_point{
                        (grid_x + (range_right * 0.5f) - (range_left * 0.5f)), grid_y,
                        explosion_z_pos};
                    sal::Transform explosion_left_right_transform{
                        left_right_mid_point, glm::vec3{0.f},
                        glm::vec3{left_right_length, 1, explosion_height}};

                    auto explosion_left_right_ent = m_registry.create();
                    m_registry.emplace<sal::Model>(explosion_left_right_ent, explosion_model);
                    m_registry.emplace<sal::Shader_program>(explosion_left_right_ent,
                                                            m_shaders.at("invert"));
                    m_registry.emplace<sal::Transform>(explosion_left_right_ent,
                                                       explosion_left_right_transform);

                    m_explosion_pos_to_entity.insert(
                        {m_game_state.m_explosion_states.back().grid_middle_point,
                         {explosion_up_down_ent, explosion_left_right_ent}});
                }

                pos += message.data.substr(pos + 1).find(';') + 1;
            }
        }
        else if (message.type == Network_protocol::Message::Type::ANNOUNCE_WINNER) {
            std::string const winner_name{message.data};
            auto winner_text_entity = m_ui_text_entities.at("announce_winner");
            m_registry.patch<sal::Text>(winner_text_entity, [winner_name](sal::Text& t) -> void {
                t.set_content("WINNER\n" + winner_name);
            });

            m_ready_to_play = false;
            m_in_game = false;
        }
        else if (message.type == Network_protocol::Message::Type::RESTART_GAME) {
            for (auto const& [cell, entity] : m_cell_to_entity) {
                m_registry.destroy(entity);
            }
            m_cell_to_entity.clear();


            for (auto const& [bomb_pos, entity] : m_bomb_pos_to_entity) {
                m_registry.destroy(entity);
            }
            m_bomb_pos_to_entity.clear();

            for (auto const& [pickup_state, entity] : m_pickup_to_entity) {
                m_registry.destroy(entity);
            }
            m_pickup_to_entity.clear();

            for (auto const& [explosion_pos, entities] : m_explosion_pos_to_entity) {
                m_registry.destroy(entities.first);
                m_registry.destroy(entities.second);
            }
            m_explosion_pos_to_entity.clear();

            for (auto const& [player_display_name, entity] : m_player_to_entity) {
                m_registry.destroy(entity);
            }
            m_player_to_entity.clear();

            m_game_state.reset();
        }
    }

    m_message_queue.clear();
}

void Bbmania::blow_up_bombs() noexcept
{
    for (auto const& bs : m_game_state.m_bomb_states) {
        if (bs.has_exploded) {
            if (m_bomb_pos_to_entity.contains(bs.grid_position)) {
                auto ent = m_bomb_pos_to_entity.at(bs.grid_position);
                m_registry.destroy(ent);
                m_bomb_pos_to_entity.erase(bs.grid_position);
            }
        }
    }

    std::erase_if(m_game_state.m_bomb_states,
                  [&](Bomb_state const& bs) -> bool { return bs.has_exploded; });
}

void Bbmania::expire_explosion_objects() noexcept
{
    auto time_now = std::chrono::system_clock::now();
    for (auto const& es : m_game_state.m_explosion_states) {
        if (es.time_placed + es.life_time < time_now) {
            if (m_explosion_pos_to_entity.contains(es.grid_middle_point)) {
                sal::Log::info("Removed explosion object {}, {}, {}", es.grid_middle_point.first,
                               es.grid_middle_point.second, time_now.time_since_epoch().count());

                auto ent = m_explosion_pos_to_entity.at(es.grid_middle_point);
                m_registry.destroy(ent.first);
                m_registry.destroy(ent.second);
                m_explosion_pos_to_entity.erase(es.grid_middle_point);
            }
        }
    }

    std::erase_if(m_game_state.m_explosion_states, [&](Explosion_state const& es) -> bool {
        return es.time_placed + es.life_time < time_now;
    });
}

} // namespace bbm