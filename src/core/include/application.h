/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_APPLICATION_H
#define SALMIAC_APPLICATION_H

#include "camera.h"
#include "file_reader.h"
#include "input_manager.h"
#include "log.h"
#include "model_loader.h"
#include "shader_loader.h"
#include "transform.h"

#include "entt/entity/registry.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace sal {

/// TODO: Move these to a sensible file.
struct window_deleter {
    void operator()(GLFWwindow* w) { glfwDestroyWindow(w); }
};

typedef std::unique_ptr<GLFWwindow, window_deleter> Window_ptr;


class Application {
public:
    enum class Exit_code : std::size_t { ok = 0, glfw_init_fail = 1, glew_init_fail = 2 };
    Exit_code setup(std::size_t const w_w, std::size_t const w_h) noexcept;

protected:
    void update() noexcept;

    virtual void run_user_tasks() noexcept = 0;
    virtual void set_user_uniforms(Shader_program& shader) noexcept = 0;

    void register_keys(std::initializer_list<std::int32_t> keys,
                       std::initializer_list<std::int32_t> mouse_buttons) noexcept;

    Window_ptr create_window(std::size_t const w, std::size_t const h) noexcept;


    entt::registry m_registry;
    Window_ptr m_window;
    Input_manager m_input_manager;
    bool m_suggest_close{false};
    std::size_t m_frame_counter{0};
    float m_delta_time{0.0};

    std::chrono::high_resolution_clock::time_point m_t_start;
    std::chrono::high_resolution_clock::time_point m_t_prev_update;

    static std::size_t m_window_width;
    static std::size_t m_window_height;

private:
    void run_engine_tasks() noexcept;

    bool init_glew() noexcept;

    void render_models() noexcept;

    bool key_callback(std::int32_t const key) noexcept;
    bool mouse_click_callback(std::int32_t const button) noexcept;
};

} // namespace sal

#endif
