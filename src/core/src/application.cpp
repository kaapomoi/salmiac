/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"

#include <GLFW/glfw3.h>

namespace sal {

Application::Exit_code Application::setup(std::size_t const w_w, std::size_t const w_h) noexcept
{
    sal::Log::init("sal_log.txt");

    if (!glfwInit()) {
        sal::Log::fatal("Failed to initialize glfw3");
        return Exit_code::glfw_init_fail;
    }
    else {
        sal::Log::info("glfw3 initialized!");
    }

    m_window = create_window(w_w, w_h);

    glfwSetFramebufferSizeCallback(m_window.get(), framebuffer_size_callback);

    glViewport(0, 0, w_w, w_h);

    if (!init_glew()) {
        glfwTerminate();
        return Exit_code::glew_init_fail;
    }

    /// TODO: Make a renderer helper class?
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Application::update() noexcept
{
    /// Update inputs
    run_engine_tasks();

    /// User does something with the updated inputs
    run_user_tasks();

    /// Draw?


    m_suggest_close = glfwWindowShouldClose(m_window.get());
}

void Application::run_engine_tasks() noexcept
{
    static std::function<bool(std::int32_t)> k_cb = [this](std::int32_t const key) {
        return key_callback(key);
    };

    static std::function<bool(std::int32_t)> m_cb = [this](std::int32_t const button) {
        return mouse_click_callback(button);
    };

    m_input_manager.update(k_cb, m_cb);
}

void Application::register_keys(std::initializer_list<std::int32_t> keys,
                                std::initializer_list<std::int32_t> mouse_buttons) noexcept
{
    m_input_manager.register_keys(keys, mouse_buttons);
}

Window_ptr Application::create_window(GLsizei const w, GLsizei const h) noexcept
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = sal::Window_ptr(glfwCreateWindow(w, h, "salmiac sandbox", nullptr, nullptr));

    if (window == nullptr) {
        glfwTerminate();
        return nullptr;
    }

    return window;
}

void Application::framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    sal::Log::info("Changing framebuffer size to {} {}", width, height);
    glViewport(0, 0, width, height);
}

bool Application::init_glew() noexcept
{
    glewExperimental = true; // Needed for core profile
    glfwMakeContextCurrent(m_window.get());
    if (glewInit() != GLEW_OK) {
        sal::Log::fatal("Failed to initialize GLEW, Reason: %s", glGetError());
        return false;
    }

    fprintf(stdout, "OpenGL version: %s\n", glGetString(GL_VERSION));
    sal::Log::info("GLEW initialized!");
    return true;
}

bool Application::key_callback(std::int32_t const key) noexcept
{
    return static_cast<bool>(glfwGetKey(m_window.get(), key));
}

bool Application::mouse_click_callback(std::int32_t const button) noexcept
{
    return static_cast<bool>(glfwGetMouseButton(m_window.get(), button));
}

}; // namespace sal
