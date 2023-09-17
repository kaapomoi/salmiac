/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef CONQUEST_H
#define CONQUEST_H

#include "application.h"
#include "camera_controller.h"
#include "game.h"
#include "primitives.h"

#include <chrono>
#include <random>

class Conquest : public sal::Application {
public:
    sal::Application::Exit_code start() noexcept;


    sal::Application::Exit_code run() noexcept;

    void cleanup() noexcept;

private:
    void run_user_tasks() noexcept final;

    void set_render_model_uniforms(sal::Shader_program& shader) noexcept final;
    void set_user_uniforms_before_render() noexcept final;

    void handle_input() noexcept;

    Camera_controller m_camera_controller{};
    std::vector<sal::Shader_program> m_shaders;
    std::vector<sal::Model> m_models;
    std::vector<sal::Font> m_fonts;
    std::vector<sal::Text> m_texts;
    std::mt19937 m_rand_engine;
    float m_sim_timescale{10.f};
    bool m_should_restart_sim{false};
    std::vector<Game> m_games;
    std::vector<glm::vec4> m_cell_colors{{1.f, 0.f, 0.f, 1.f}, {0.f, 1.f, 0.f, 1.f},
                                         {0.f, 0.f, 1.f, 1.f}, {1.f, 1.f, 0.f, 1.f},
                                         {1.f, 0.f, 1.f, 1.f}, {0.f, 1.f, 1.f, 1.f}};
};

#endif
