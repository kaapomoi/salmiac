/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef N_BODY_SIM_H
#define N_BODY_SIM_H

#include "application.h"
#include "oct.h"

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
                    sal::Transform& transform) const noexcept
    {
        if (input_manager.button(GLFW_MOUSE_BUTTON_RIGHT)) {
            camera_data.look_around(static_cast<float>(x_offset), static_cast<float>(-y_offset),
                                    true);
            glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        float camera_movement_speed{50.f};

        if (input_manager.key(GLFW_KEY_LEFT_SHIFT)) {
            camera_movement_speed = 500.f;
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

class N_body_sim : public sal::Application {
public:
    sal::Application::Exit_code start() noexcept;


    sal::Application::Exit_code run() noexcept;

    void cleanup() noexcept;

private:
    void run_user_tasks() noexcept final;

    void set_render_model_uniforms(sal::Shader_program& shader) noexcept final;
    void set_user_uniforms_before_render() noexcept final;

    void handle_input() noexcept;

    void create_nodes(std::size_t const n) noexcept;
    void update_nodes() noexcept;

    Camera_controller m_camera_controller{};
    std::vector<sal::Shader_program> m_shaders;
    std::vector<sal::Model> m_models;
    std::mt19937 m_rand_engine;
    float m_sim_timescale{1000.f};

    std::unique_ptr<Oct> m_root{nullptr};
};

#endif
