#include "n_body_sim.h"

#include "texture_loader.h"


sal::Application::Exit_code N_body_sim::start() noexcept
{
    register_keys({GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_E,
                   GLFW_KEY_Q, GLFW_KEY_R, GLFW_KEY_ESCAPE, GLFW_KEY_F1},
                  {GLFW_MOUSE_BUTTON_RIGHT});

    return setup(1920, 1080);
}


sal::Application::Exit_code N_body_sim::run() noexcept
{
    auto v_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
    auto f_str = sal::File_reader::read_file("../res/shaders/bad_lighting_frag.glsl");

    m_shaders.push_back(sal::Shader_loader::from_sources(
        v_str, f_str, {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}}, {"material"}));

    auto basic_lighting_str = sal::File_reader::read_file("../res/shaders/basic_lighting.fsh");
    m_shaders.push_back(sal::Shader_loader::from_sources(
        v_str, basic_lighting_str, {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}},
        {"material"}));

    auto f2_str = sal::File_reader::read_file("../res/shaders/liquid_frag.glsl");
    m_shaders.push_back(sal::Shader_loader::from_sources(
        v_str, f2_str, {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}},
        {"material", "frame"}));

    auto instanced_vert = sal::File_reader::read_file("../res/shaders/instanced_vert.glsl");
    m_shaders.push_back(sal::Shader_loader::from_sources(instanced_vert, f2_str,
                                                         {{"in_uv"},
                                                          {"in_normal"},
                                                          {"in_pos"},
                                                          {"in_color"},
                                                          {"in_instance_model_matrix", 4},
                                                          {"in_instance_color"}},
                                                         {"material", "frame"}));

    std::uint64_t const base_flags = aiProcess_Triangulate | aiProcess_GenNormals
                                     | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes;

    std::string const model_file{"../res/models/cube/cube.obj"};
    float const scale_factor{0.5f};
    m_models.push_back(
        sal::Model_loader::from_file(model_file, scale_factor, base_flags | aiProcess_FlipUVs));

    sal::Texture cube_tex{sal::Texture_loader::from_file("../res/models/case/j-cover.png",
                                                         sal::Texture::Type::diffuse)};

    sal::Mesh cube{sal::Primitive_factory::cube(glm::vec3{scale_factor}, {cube_tex})};
    m_models.push_back(sal::Model{{cube}});

    auto text_vert = sal::File_reader::read_file("../res/shaders/basic_text_vert.glsl");
    auto text_frag = sal::File_reader::read_file("../res/shaders/basic_text_frag.glsl");
    m_shaders.push_back(sal::Shader_loader::from_sources(
        text_vert, text_frag, {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}},
        {"atlas", "color"}));
    m_fonts.emplace_back(m_font_loader.create("../res/fonts/calibri.ttf"));


    auto entity2 = m_registry.create();
    sal::Text text{"Hello, world", m_fonts.front(), glm::vec2{0}, glm::vec2{0.2f},
                   glm::vec4{0.8f, 0.8f, 0.8f, 0.5f}};
    m_registry.emplace<sal::Text>(entity2, text);
    m_registry.emplace<sal::Shader_program>(entity2, m_shaders.at(4));
    sal::Transform t{glm::vec3{0.f}, glm::vec3{0.f}, glm::vec3{1.f}};
    m_registry.emplace<sal::Transform>(entity2, t);

    m_rand_engine.seed(time(NULL));

    create_nodes(50000);

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

    update_nodes();

    auto text_view = m_registry.view<sal::Transform, sal::Text>();
    for (auto [entity, transform, text] : text_view.each()) {
        auto c_of_m = m_root->center_of_mass();
        std::string x{std::to_string(c_of_m.x)};
        std::string y{std::to_string(c_of_m.y)};
        std::string z{std::to_string(c_of_m.z)};
        x.resize(5);
        y.resize(5);
        z.resize(5);
        std::string a{x + ", " + y + ", " + z};
        text.set_content(a);
        transform.position = c_of_m;
        //transform.rotation += glm::vec3{1.f, 1.f, 0.f};
    }
}

void N_body_sim::set_user_uniforms_before_render() noexcept
{
    for (auto& shader : m_shaders) {
        shader.use();

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

        shader.un_use();
    }
}

void N_body_sim::set_render_model_uniforms(sal::Shader_program& shader) noexcept
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
    if (m_input_manager.key_now(GLFW_KEY_Q)) {
        m_sim_timescale /= 2;
        sal::Log::info("Timescale: {}", m_sim_timescale);
    }
    if (m_input_manager.key_now(GLFW_KEY_E)) {
        m_sim_timescale *= 2;
        sal::Log::info("Timescale: {}", m_sim_timescale);
    }
    if (m_input_manager.key_now(GLFW_KEY_R)) {
        m_should_restart_sim = true;
    }
}


void N_body_sim::create_nodes(std::size_t const n) noexcept
{
    std::uniform_int_distribution<std::int32_t> position(-200, 200);
    std::uniform_real_distribution<float> velo0(0.0001f, 0.0002f);
    std::uniform_real_distribution<float> mass(5e5, 5e5);

    m_root = std::make_unique<Oct>(glm::vec3{std::numeric_limits<float>::min()},
                                   glm::vec3{std::numeric_limits<float>::max()});

    float const radius{300.f};
    float const offset{75.f};

    std::uniform_real_distribution<float> pos0(-1.0f, 1.0f);
    for (std::size_t j{0}; j < 10; j++) {
        glm::vec3 const offset{pos0(m_rand_engine) * 200.f, pos0(m_rand_engine) * 200.f,
                               pos0(m_rand_engine) * 200.f};
        for (std::size_t i{0}; i < (n * 0.1); i++) {
            auto entity = m_registry.create();
            glm::vec3 const p{pos0(m_rand_engine), pos0(m_rand_engine), pos0(m_rand_engine)};

            glm::vec3 const p_norm{glm::normalize(p)};

            glm::vec3 const perpendicular{0.f, 1.f, 0.f};
            glm::vec3 const tangent{glm::cross(p_norm, perpendicular)};
            glm::vec3 const v0{tangent * velo0(m_rand_engine)};

            Node node{p_norm * radius * 0.3f + offset, {}, {}, v0, mass(m_rand_engine)};

            float const m{mass(m_rand_engine)};

            m_registry.emplace<std::shared_ptr<Node>>(entity, std::make_shared<Node>(node));
            m_registry.emplace<sal::Instanced>(entity, sal::Instanced{m_models.at(1)});
            m_registry.emplace<sal::Shader_program>(entity, m_shaders.at(3));
            sal::Transform t{node.position, glm::vec3{0.f}, glm::vec3{1.f}};
            m_registry.emplace<sal::Transform>(entity, t);
        }
    }
}


void N_body_sim::update_nodes() noexcept
{
    if (m_should_restart_sim) {
        m_should_restart_sim = false;

        auto node_view =
            m_registry
                .view<std::shared_ptr<Node>, sal::Transform, sal::Instanced, sal::Shader_program>();
        m_registry.destroy(node_view.begin(), node_view.end());
        create_nodes(50000);
    }

    std::chrono::high_resolution_clock::time_point sw_start{
        std::chrono::high_resolution_clock::now()};
    m_root.reset();
    m_root = std::make_unique<Oct>(glm::vec3{-20000}, glm::vec3{20000});
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

        transform.rotation = node->velocity * 10000.f * 360.f;
        transform.position = node->position;
    }

    std::chrono::high_resolution_clock::time_point now{std::chrono::high_resolution_clock::now()};

    float time_diff =
        std::chrono::duration_cast<std::chrono::duration<float>>(now - sw_start).count();
    sal::Log::info("update_nodes_time: {}", time_diff);
}
