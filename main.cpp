/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"

#include <chrono>
#include <random>

static GLsizei constexpr WINDOW_WIDTH{1920};
static GLsizei constexpr WINDOW_HEIGHT{1080};

namespace {

struct Rotator {
    glm::vec3 amount;
};

}; // namespace

class Market : public sal::Application {
public:
    sal::Application::Exit_code start() noexcept
    {
        register_keys(
            {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_ESCAPE},
            {GLFW_MOUSE_BUTTON_RIGHT});

        return setup(WINDOW_WIDTH, WINDOW_HEIGHT);
    }


    sal::Application::Exit_code run() noexcept
    {
        auto v_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
        auto f_str = sal::File_reader::read_file("../res/shaders/bad_lighting_frag.glsl");

        m_shaders.push_back(
            sal::Shader_loader::from_sources(v_str, f_str, {"in_uv", "in_normal", "in_pos"}));

        auto v2_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
        auto f2_str = sal::File_reader::read_file("../res/shaders/liquid_frag.glsl");
        m_shaders.push_back(
            sal::Shader_loader::from_sources(v2_str, f2_str, {"in_uv", "in_normal", "in_pos"}));

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

        entt::entity palace = m_registry.create();
        m_registry.emplace<sal::Transform>(palace, glm::vec3{0.0f}, glm::vec3{0.0f},
                                           glm::vec3{1.0f});
        m_registry.emplace<sal::Model>(palace, m_models.back());
        m_registry.emplace<sal::Shader_program>(palace, m_shaders.back());
        static std::mt19937 rand_engine;
        auto dist{std::uniform_real_distribution<double>()};

        for (std::size_t i{0}; i < 8; i++) {
            for (std::size_t j{0}; j < 8; j++) {
                entt::entity kao = m_registry.create();
                m_registry.emplace<sal::Transform>(kao,
                                                   glm::vec3{dist(rand_engine) * 4,
                                                             static_cast<float>(i) + 0.5f,
                                                             static_cast<float>(j)},
                                                   glm::vec3{90.0f, 0.0f, 90.f}, glm::vec3{1.0f});
                m_registry.emplace<sal::Model>(kao, m_models.front());
                m_registry.emplace<sal::Shader_program>(kao, m_shaders.at((i + j) % 2));
                m_registry.emplace<Rotator>(
                    kao, glm::vec3{0.f, dist(rand_engine) * 10.f, dist(rand_engine) * 10.f});
            }
        }

        m_t_start = std::chrono::high_resolution_clock::now();
        m_t_prev_update = m_t_start;

        while (!m_suggest_close) {
            update();
        }

        return Exit_code::ok;
    }

    void cleanup() noexcept
    {
        for (auto const& shader : m_shaders) {
            glDeleteProgram(shader.program_id);
        }
        glfwTerminate();
    }

private:
    void run_user_tasks() noexcept final
    {
        handle_input();
        glClearColor(0.0f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        auto t_now = std::chrono::high_resolution_clock::now();
        float delta_time =
            std::chrono::duration_cast<std::chrono::duration<float>>(t_now - m_t_prev_update)
                .count();
        m_t_prev_update = t_now;

        auto rotator_view = m_registry.view<Rotator, sal::Transform>();
        for (auto [entity, rotator, transform] : rotator_view.each()) {
            transform.rotation += rotator.amount * delta_time;
        }
    }

    void set_user_uniforms(sal::Shader_program& shader) noexcept final
    {
        glm::mat4 const projection = glm::perspective(
            glm::radians(m_camera.zoom()),
            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f, 1000.0f);

        glm::mat4 const view = m_camera.get_view_matrix();

        shader.set_uniform<float>("material.shininess", 64.0f);
        shader.set_uniform("camera_pos", m_camera.position());
        shader.set_uniform("projection", projection);
        shader.set_uniform("view", view);
        /// TODO:
        //if (shader.has_property("")) {
        //}
        if (shader.program_id == m_shaders.back().program_id) {
            shader.set_uniform<std::int32_t>("frame", m_frame_counter);
        }
    }

    void handle_input()
    {
        double x;
        double y;
        glfwGetCursorPos(m_window.get(), &x, &y);

        static constexpr float sensitivity{0.1f};

        double const x_offset{(x - m_mouse_x) * sensitivity};
        double const y_offset{(y - m_mouse_y) * sensitivity};

        m_mouse_x = x;
        m_mouse_y = y;

        if (m_input_manager.button(GLFW_MOUSE_BUTTON_RIGHT)) {
            m_camera.look_around(static_cast<float>(x_offset), static_cast<float>(-y_offset), true);
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        static constexpr float camera_movement_speed{0.1f};

        if (m_input_manager.key(GLFW_KEY_W)) {
            m_camera.move(sal::Camera::FORWARD, camera_movement_speed);
        }
        if (m_input_manager.key(GLFW_KEY_S)) {
            m_camera.move(sal::Camera::BACKWARD, camera_movement_speed);
        }
        if (m_input_manager.key(GLFW_KEY_A)) {
            m_camera.move(sal::Camera::LEFT, camera_movement_speed);
        }
        if (m_input_manager.key(GLFW_KEY_D)) {
            m_camera.move(sal::Camera::RIGHT, camera_movement_speed);
        }

        if (m_input_manager.key(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(m_window.get(), true);
        }
    }

    sal::Camera m_camera{{-10.f, 4.f, 4.f}};
    double m_mouse_x{0};
    double m_mouse_y{0};
    std::chrono::high_resolution_clock::time_point m_t_start;
    std::chrono::high_resolution_clock::time_point m_t_prev_update;

    std::vector<sal::Shader_program> m_shaders;
    std::vector<sal::Model> m_models;
};


int main()
{
    Market app;
    app.start();

    app.run();

    app.cleanup();

    return 0;
}
