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

    void register_keys(std::initializer_list<std::int32_t> keys,
                       std::initializer_list<std::int32_t> mouse_buttons) noexcept;

    Window_ptr create_window(GLsizei const w, GLsizei const h) noexcept;


    entt::registry m_registry;
    Window_ptr m_window;
    Input_manager m_input_manager;
    bool m_suggest_close{false};

private:
    static void framebuffer_size_callback(GLFWwindow*, int width, int height);

    void run_engine_tasks() noexcept;

    bool init_glew() noexcept;

    bool key_callback(std::int32_t const key) noexcept;
    bool mouse_click_callback(std::int32_t const button) noexcept;
};

} // namespace sal

#endif
