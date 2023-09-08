#include "n_body_sim.h"


sal::Application::Exit_code N_body_sim::start() noexcept
{
    register_keys({GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_SHIFT,
                   GLFW_KEY_ESCAPE, GLFW_KEY_F1},
                  {GLFW_MOUSE_BUTTON_RIGHT});

    return setup(WINDOW_WIDTH, WINDOW_HEIGHT);
}


sal::Application::Exit_code N_body_sim::run() noexcept
{
    auto v_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
    auto f_str = sal::File_reader::read_file("../res/shaders/bad_lighting_frag.glsl");

    m_shaders.push_back(sal::Shader_loader::from_sources(
        v_str, f_str, {"in_uv", "in_normal", "in_pos"}, {"material"}));

    auto basic_lighting_str = sal::File_reader::read_file("../res/shaders/basic_lighting.fsh");
    m_shaders.push_back(sal::Shader_loader::from_sources(
        v_str, basic_lighting_str, {"in_uv", "in_normal", "in_pos"}, {"material"}));

    auto f2_str = sal::File_reader::read_file("../res/shaders/liquid_frag.glsl");
    m_shaders.push_back(sal::Shader_loader::from_sources(
        v_str, f2_str, {"in_uv", "in_normal", "in_pos"}, {"material", "frame"}));

    std::uint64_t const base_flags = aiProcess_Triangulate | aiProcess_GenNormals
                                     | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes;

    std::string const model_file{"../res/models/case/j-case.obj"};
    float const scale_factor{0.5f};
    m_models.push_back(
        sal::Model_loader::from_file(model_file, scale_factor, base_flags | aiProcess_FlipUVs));

    std::string const avo_file{"../res/models/Sponza/Sponza.gltf"};
    float const scale_factor2{0.1f};
    m_models.push_back(
        sal::Model_loader::from_file(avo_file, scale_factor2, base_flags | aiProcess_FlipUVs));
    /*
        entt::entity palace = m_registry.create();
        m_registry.emplace<sal::Transform>(palace, glm::vec3{0.0f}, glm::vec3{0.0f}, glm::vec3{1.0f});
        m_registry.emplace<sal::Model>(palace, m_models.back());
        m_registry.emplace<sal::Shader_program>(palace, m_shaders.at(1));
        auto dist{std::uniform_real_distribution<double>()};

        for (std::size_t i{0}; i < 8; i++) {
            for (std::size_t j{0}; j < 8; j++) {
                entt::entity kao = m_registry.create();
                m_registry.emplace<sal::Transform>(kao,
                                                   glm::vec3{dist(m_rand_engine) * 4,
                                                             static_cast<float>(i) + 5.0f,
                                                             static_cast<float>(j)},
                                                   glm::vec3{90.0f, 0.0f, 90.f}, glm::vec3{1.0f});
                m_registry.emplace<sal::Model>(kao, m_models.front());
                m_registry.emplace<sal::Shader_program>(kao, m_shaders.at((i + j) % 3));
                m_registry.emplace<Rotator>(
                    kao, glm::vec3{0.f, dist(m_rand_engine) * 10.f, dist(m_rand_engine) * 10.f});
                m_registry.emplace<Mover>(
                    kao, glm::vec3{0.f, dist(m_rand_engine) * 0.1f, dist(m_rand_engine) * 0.01f});
            }
        }
    */

    create_nodes(10000);

    entt::entity camera{m_registry.create()};
    m_registry.emplace<sal::Transform>(camera, glm::vec3{0.0f}, glm::vec3{0.0f}, glm::vec3{1.0f});
    m_registry.emplace<sal::Camera>(camera);


    m_t_start = std::chrono::high_resolution_clock::now();
    m_t_prev_update = m_t_start;

    while (!m_suggest_close) {
        update();
    }

    return Exit_code::ok;
}

void N_body_sim::cleanup() noexcept
{
    for (auto const& shader : m_shaders) {
        glDeleteProgram(shader.program_id);
    }
    glfwTerminate();
}


///
/// Private section:
///
void N_body_sim::run_user_tasks() noexcept
{
    handle_input();
    glClearColor(0.0f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    /*
        auto rotator_view = m_registry.view<Rotator, sal::Transform>();
        for (auto [entity, rotator, transform] : rotator_view.each()) {
            transform.rotation += rotator.amount * m_delta_time;
        }

        float time_diff =
            std::chrono::duration_cast<std::chrono::duration<float>>(m_t_prev_update - m_t_start)
                .count();
        auto mover_view = m_registry.view<Mover, sal::Transform>();
        for (auto [entity, mover, transform] : mover_view.each()) {
            Sine_mover{}(mover, transform, time_diff);

        }
        */
    update_nodes();
}


void N_body_sim::set_user_uniforms(sal::Shader_program& shader) noexcept
{
    auto camera_view = m_registry.view<sal::Transform, sal::Camera>();
    for (auto [entity, transform, camera] : camera_view.each()) {

        glm::mat4 const projection = glm::perspective(glm::radians(camera.zoom()),
                                                      static_cast<float>(m_window_width)
                                                          / static_cast<float>(m_window_height),
                                                      0.01f, 10000.0f);

        glm::mat4 const view = camera.get_view_matrix(transform.position);

        shader.set_uniform("camera_pos", transform.position);
        shader.set_uniform("view", view);
        shader.set_uniform("projection", projection);
    }

    if (shader.has_uniform("frame")) {
        shader.set_uniform<std::int32_t>("frame", static_cast<std::int32_t>(m_frame_counter));
    }

    shader.set_uniform<float>("material.shininess", 64.0f);
}


void N_body_sim::handle_input() noexcept
{
    sal::Mouse_position const m_mouse_delta{m_input_manager.mouse_pos_delta()};
    static constexpr float sensitivity{0.1f};

    double const x_offset{m_mouse_delta.x * sensitivity};
    double const y_offset{m_mouse_delta.y * sensitivity};

    auto camera_view = m_registry.view<sal::Transform, sal::Camera>();
    for (auto [entity, transform, camera] : camera_view.each()) {
        m_camera_controller(x_offset, y_offset, m_delta_time, m_window, m_input_manager, camera,
                            transform);
    }
    if (m_input_manager.key(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(m_window.get(), true);
    }
    if (m_input_manager.key(GLFW_KEY_F1)) {
        sal::Log::info("FPS: {}", (1 / m_delta_time));
    }
}


void N_body_sim::create_nodes(std::size_t const n) noexcept
{
    std::uniform_int_distribution<std::int32_t> position(-200, 200);
    std::uniform_real_distribution<float> mass(1e5, 1e5);

    m_root = std::make_unique<Oct>(glm::vec3{std::numeric_limits<float>::min()},
                                   glm::vec3{std::numeric_limits<float>::max()});

    for (std::size_t i{0}; i < n; i++) {
        auto entity = m_registry.create();
        glm::vec3 p{position(m_rand_engine), position(m_rand_engine), position(m_rand_engine)};

        Node n{p, {}, {}, {}, mass(m_rand_engine)};


        m_registry.emplace<std::shared_ptr<Node>>(entity, std::make_shared<Node>(n));
        m_registry.emplace<sal::Model>(entity, m_models.at(0));
        m_registry.emplace<sal::Shader_program>(entity, m_shaders.at(2));
        sal::Transform t{n.position, glm::vec3{0.f}, glm::vec3{1.f}};
        m_registry.emplace<sal::Transform>(entity, t);
    }
}


void N_body_sim::update_nodes() noexcept
{
    std::chrono::high_resolution_clock::time_point sw_start{
        std::chrono::high_resolution_clock::now()};
    m_root.reset();
    m_root = std::make_unique<Oct>(glm::vec3{std::numeric_limits<float>::min()},
                                   glm::vec3{std::numeric_limits<float>::max()});
    auto node_view = m_registry.view<sal::Transform, std::shared_ptr<Node>>();
    for (auto [entity, transform, node] : node_view.each()) {
        m_root->insert(node);
    }

    for (auto [entity, transform, node] : node_view.each()) {
        node->force = glm::vec3{0.f};
        m_root->update_force(node);
        node->acceleration = node->force / node->mass;
        node->velocity += node->acceleration * m_sim_timescale;
        node->position += node->velocity * m_sim_timescale;

        transform.rotation += node->velocity;
        transform.position = node->position;
    }

    std::chrono::high_resolution_clock::time_point now{std::chrono::high_resolution_clock::now()};

    float time_diff =
        std::chrono::duration_cast<std::chrono::duration<float>>(now - sw_start).count();
    sal::Log::info("update_nodes_time: {}", time_diff);
}
