/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"

#include <chrono>
#include <random>

static GLsizei constexpr WINDOW_WIDTH{1920};
static GLsizei constexpr WINDOW_HEIGHT{1080};


struct Rotator {
    glm::vec3 amount;
};

struct Mover {
    glm::vec3 amount;
};

struct Sine_mover {
    void operator()(Mover const& mover, sal::Transform& transform, float const time) noexcept
    {
        transform.position += sin(time) * mover.amount;
    }
};

struct Camera_controller {
    void operator()(double const x_offset,
                    double const y_offset,
                    float const amount,
                    sal::Window_ptr const& window,
                    sal::Input_manager const& input_manager,
                    sal::Camera& camera_data,
                    sal::Transform& transform) noexcept
    {
        if (input_manager.button(GLFW_MOUSE_BUTTON_RIGHT)) {
            camera_data.look_around(static_cast<float>(x_offset), static_cast<float>(-y_offset),
                                    true);
            glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        float camera_movement_speed{5.f};

        if (input_manager.key(GLFW_KEY_LEFT_SHIFT)) {
            camera_movement_speed = 50.f;
        }

        if (input_manager.key(GLFW_KEY_W)) {
            transform.position += camera_data.front() * amount * camera_movement_speed;
        }
        if (input_manager.key(GLFW_KEY_S)) {
            transform.position -= camera_data.front() * amount * camera_movement_speed;
        }
        if (input_manager.key(GLFW_KEY_A)) {
            transform.position -= camera_data.right() * amount * camera_movement_speed;
        }
        if (input_manager.key(GLFW_KEY_D)) {
            transform.position += camera_data.right() * amount * camera_movement_speed;
        }
    }
};


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

        entt::entity palace = m_registry.create();
        m_registry.emplace<sal::Transform>(palace, glm::vec3{0.0f}, glm::vec3{0.0f},
                                           glm::vec3{1.0f});
        m_registry.emplace<sal::Model>(palace, m_models.back());
        m_registry.emplace<sal::Shader_program>(palace, m_shaders.at(1));
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
                m_registry.emplace<sal::Shader_program>(kao, m_shaders.at((i + j) % 3));
                m_registry.emplace<Rotator>(
                    kao, glm::vec3{0.f, dist(rand_engine) * 10.f, dist(rand_engine) * 10.f});
                m_registry.emplace<Mover>(
                    kao, glm::vec3{0.f, dist(rand_engine) * 0.1f, dist(rand_engine) * 0.01f});
            }
        }

        entt::entity camera{m_registry.create()};
        m_registry.emplace<sal::Transform>(camera, glm::vec3{0.0f}, glm::vec3{0.0f},
                                           glm::vec3{1.0f});
        m_registry.emplace<sal::Camera>(camera);


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
    }

    void set_user_uniforms(sal::Shader_program& shader) noexcept final
    {
        auto camera_view = m_registry.view<sal::Transform, sal::Camera>();
        for (auto [entity, transform, camera] : camera_view.each()) {

            glm::mat4 const projection = glm::perspective(glm::radians(camera.zoom()),
                                                          static_cast<float>(m_window_width)
                                                              / static_cast<float>(m_window_height),
                                                          0.1f, 1000.0f);

            glm::mat4 const view = camera.get_view_matrix(transform.position);

            shader.set_uniform("camera_pos", transform.position);
            shader.set_uniform("view", view);
            shader.set_uniform("projection", projection);
        }

        if (shader.has_uniform("frame")) {
            shader.set_uniform<std::int32_t>("frame", m_frame_counter);
        }

        shader.set_uniform<float>("material.shininess", 64.0f);
    }

    void handle_input()
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
    }

    Camera_controller m_camera_controller{};
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
