#include "conquest.h"

#include "texture_loader.h"


sal::Application::Exit_code Conquest::start() noexcept
{
    register_keys({GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_E,
                   GLFW_KEY_Q, GLFW_KEY_R, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_ESCAPE, GLFW_KEY_F1},
                  {GLFW_MOUSE_BUTTON_RIGHT});

    return setup(1920, 1080);
}


sal::Application::Exit_code Conquest::run() noexcept
{
    auto v_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
    auto f_str = sal::File_reader::read_file("../res/shaders/basic_lighting.fsh");

    m_shaders.push_back(sal::Shader_loader::from_sources(
        v_str, f_str, {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}}, {"material"}));


    auto color_only_frag = sal::File_reader::read_file("../res/shaders/color_lighting_frag.glsl");
    auto instanced_vert = sal::File_reader::read_file("../res/shaders/instanced_vert.glsl");
    m_shaders.push_back(sal::Shader_loader::from_sources(instanced_vert, color_only_frag,
                                                         {{"in_uv"},
                                                          {"in_normal"},
                                                          {"in_pos"},
                                                          {"in_color"},
                                                          {"in_instance_model_matrix", 4},
                                                          {"in_instance_color"}},
                                                         {"material", "frame"}));

    std::uint64_t const base_flags = aiProcess_Triangulate | aiProcess_GenNormals
                                     | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes;

    float const scale_factor{1.0f};

    sal::Texture cube_tex{sal::Texture_loader::from_file("../res/models/case/j-cover.png",
                                                         sal::Texture::Type::diffuse)};

    sal::Mesh cube{sal::Primitive_factory::cube(glm::vec3{scale_factor}, {cube_tex})};
    m_models.push_back(sal::Model{{cube}});

    auto text_vert = sal::File_reader::read_file("../res/shaders/basic_text_vert.glsl");
    auto text_frag = sal::File_reader::read_file("../res/shaders/basic_text_frag.glsl");
    m_shaders.push_back(sal::Shader_loader::from_sources(
        text_vert, text_frag, {{"in_uv"}, {"in_normal"}, {"in_pos"}, {"in_color"}}, {"atlas"}));
    m_fonts.emplace_back(m_font_loader.create("../res/fonts/calibri.ttf"));

    for (std::size_t g{0}; g < n_games; g++) {
        for (std::size_t y{0}; y < board_h; y++) {
            for (std::size_t x{0}; x < board_w; x++) {
                auto ent = m_registry.create();
                m_registry.emplace<sal::Instanced>(ent, m_models.front(),
                                                   glm::vec4{1.f, 1.f, 1.f, 0.5f});
                m_registry.emplace<sal::Shader_program>(ent, m_shaders.at(1));
                m_registry.emplace<Cell_position>(ent, x, y, g);
                sal::Transform t{
                    glm::vec3{x + board_w * 1.2f * (g % 10), y + board_h * 1.2f * (g / 10), 0},
                    glm::vec3{0.f}, glm::vec3{1.f}};
                m_registry.emplace<sal::Transform>(ent, t);
            }
        }
    }

    auto entity2 = m_registry.create();
    sal::Text text{"conquest3d", m_fonts.front(), glm::vec2{0}, glm::vec2{0.5f},
                   glm::vec4{1.f, 1.f, 1.f, 1.f}};
    m_registry.emplace<sal::Text>(entity2, text);
    m_registry.emplace<sal::Shader_program>(entity2, m_shaders.at(2));
    sal::Transform t{glm::vec3{20, -2, 0}, glm::vec3{0.f}, glm::vec3{0.1f}};
    m_registry.emplace<sal::Transform>(entity2, t);

    m_rand_engine.seed(time(NULL));

    entt::entity camera{m_registry.create()};
    m_registry.emplace<sal::Transform>(camera,
                                       glm::vec3{board_w / 2.f - 0.5f, board_h / 2.f - 0.5f, 50.f},
                                       glm::vec3{0.0f}, glm::vec3{1.0f});
    m_registry.emplace<sal::Camera>(camera, glm::vec3{0.f, 1.f, 0.f}, -90.f, 0.f);


    m_t_start = std::chrono::high_resolution_clock::now();
    m_t_prev_update = m_t_start;

    while (!m_suggest_close) {
        update();
    }

    m_orchestrator.stop();

    return Exit_code::ok;
}

void Conquest::cleanup() noexcept
{
    for (auto const& shader : m_shaders) {
        glDeleteProgram(shader.program_id);
    }
    glfwTerminate();
}


///
/// Private section:
///
void Conquest::run_user_tasks() noexcept
{
    handle_input();
    glClearColor(0.0f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    // Set culling mode to back faces
    glCullFace(GL_BACK);
    // Set winding order to counter-clockwise (default)
    glFrontFace(GL_CCW);

    if (m_should_restart_sim) {
        m_should_restart_sim = false;
        m_orchestrator.restart();
    }

    m_orchestrator.play_one_game_each();

    auto const cells = m_orchestrator.cells();

    /// TODO: Only update the colors that have changed.
    /// Right now every game gives its cells over, but that is bad and slow.
    /// Find a better way.
    auto cell_view = m_registry.view<sal::Transform, sal::Instanced, Cell_position>();
    for (auto [entity, transform, instance, cell_pos] : cell_view.each()) {
        instance.color =
            m_cell_colors.at(cells.at(cell_pos.game_id).at(cell_pos.y).at(cell_pos.x).color);
    }

    std::string camera_pos_text;
    auto camera_view = m_registry.view<sal::Transform, sal::Camera>();
    for (auto [entity, transform, camera] : camera_view.each()) {
        auto c_of_m = transform.position;
        std::string x{std::to_string(c_of_m.x)};
        std::string y{std::to_string(c_of_m.y)};
        std::string z{std::to_string(c_of_m.z)};
        x.resize(5);
        y.resize(5);
        z.resize(5);
        std::string a{x + ", " + y + ", " + z};
        camera_pos_text = a;
    }

    auto text_view = m_registry.view<sal::Transform, sal::Text>();
    for (auto [entity, transform, text] : text_view.each()) {
        text.set_content(camera_pos_text);
    }
}

void Conquest::set_user_uniforms_before_render() noexcept
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

void Conquest::set_render_model_uniforms(sal::Shader_program& shader) noexcept
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


void Conquest::handle_input() noexcept
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

