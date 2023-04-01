/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#include "file_reader.h"
#include "log.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <thread>

namespace {

struct window_deleter {
    void operator()(GLFWwindow* w) { glfwDestroyWindow(w); }
};

typedef std::unique_ptr<GLFWwindow, window_deleter> Window_ptr;

Window_ptr create_window(GLsizei const w, GLsizei const h) noexcept
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = Window_ptr(glfwCreateWindow(w, h, "salmiac sandbox", nullptr, nullptr));

    if (window == nullptr) {
        glfwTerminate();
        return nullptr;
    }

    return window;
}

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    sal::Log::info("Changing framebuffer size to {} {}", width, height);
    glViewport(0, 0, width, height);
}

/// Don't pass ownership of window, only get a non-mutable ref.
void process_input(Window_ptr const& window)
{
    if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window.get(), true);
    }
}

bool init_glew(Window_ptr const& window)
{
    glewExperimental = true; // Needed for core profile
    glfwMakeContextCurrent(window.get());
    if (glewInit() != GLEW_OK) {
        sal::Log::fatal("Failed to initialize GLEW, Reason: %s", glGetError());
        return false;
    }

    sal::Log::info("GLEW initialized!");
    return true;
}

} // namespace

int main()
{
    static GLsizei constexpr WINDOW_WIDTH{640};
    static GLsizei constexpr WINDOW_HEIGHT{480};
    sal::Log::init("sal_log.txt");

    if (!glfwInit()) {
        sal::Log::fatal("Failed to initialize glfw3");
        return -1;
    }
    else {
        sal::Log::info("glfw3 initialized!");
    }

    auto window = create_window(WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!init_glew(window)) {
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glfwSetFramebufferSizeCallback(window.get(), framebuffer_size_callback);

    auto v_str = sal::File_reader::read_file("../res/shaders/core.vert");
    auto f_str = sal::File_reader::read_file("../res/shaders/core.frag");

    sal::Log::info(v_str);
    sal::Log::info(f_str);

    while (!glfwWindowShouldClose(window.get())) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
