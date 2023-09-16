/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef N_BODY_SIM_H
#define N_BODY_SIM_H

#include "application.h"
#include "camera_controller.h"
#include "oct.h"
#include "primitives.h"
#include "text.h"

#include <chrono>
#include <random>


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
    std::vector<sal::Font> m_fonts;
    std::vector<sal::Text> m_texts;
    std::mt19937 m_rand_engine;
    float m_sim_timescale{10.f};
    bool m_should_restart_sim{false};

    std::unique_ptr<Oct> m_root{nullptr};
};

#endif
