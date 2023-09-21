/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef CONQUEST_H
#define CONQUEST_H

#include "application.h"
#include "artisan.h"
#include "camera_controller.h"
#include "game.h"
#include "orchestrator.h"
#include "primitives.h"

#include <chrono>
#include <random>

class Conquest : public sal::Application {
public:
    sal::Application::Exit_code start() noexcept;


    sal::Application::Exit_code run() noexcept;

    void cleanup() noexcept;

private:
    static constexpr std::size_t n_games{64};
    static constexpr std::size_t board_h{40};
    static constexpr std::size_t board_w{40};

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
    std::vector<glm::vec4> m_cell_colors{{0.8f, 0.2f, 0.2f, 1.f},  {0.1f, 0.8f, 0.15f, 1.f},
                                         {0.23f, 0.1f, 0.8f, 1.f}, {0.8f, 0.75f, 0.11f, 1.f},
                                         {1.f, 0.25f, 0.87f, 1.f}, {0.05f, 0.78f, .78f, 1.f}};
    Orchestrator<board_w, board_h, 6, 2> m_orchestrator{n_games};
};

#endif
