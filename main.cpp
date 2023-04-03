/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#include "file_reader.h"
#include "log.h"
#include "model_loader.h"
#include "shader_loader.h"

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

    auto v_str = sal::File_reader::read_file("../res/shaders/core.vs");
    auto f_str = sal::File_reader::read_file("../res/shaders/core.fs");

    auto shader = sal::Shader_loader::from_sources(v_str, f_str, {});

    std::uint64_t const base_flags = aiProcess_Triangulate | aiProcess_GenNormals
                                     | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
    std::string const model_file{"../res/models/avocado/Avocado.gltf"};

    auto avocado = sal::Model_loader::from_file(model_file, base_flags | aiProcess_FlipUVs
                                                                | aiProcess_GlobalScale);

    sal::Log::info("{}", shader.program_id);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        0.5f,  0.5f,  0.0f, // top right
        0.5f,  -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f,  0.0f  // top left
    };
    unsigned int indices[] = {
        // note that we start from 0!
        0, 1, 3, // first Triangle
        1, 2, 3  // second Triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    auto t_start = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window.get())) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shader.program_id);
        // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        auto t_now = std::chrono::high_resolution_clock::now();
        float time =
            std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        shader.set_uniform("color", glm::vec3{(sin(time * 4.0f) + 1.0f) / 2.0f,
                                              (cos(time * 4.0f) + 1.0f) / 2.0f,
                                              (tan(time * 4.0f) + 1.0f) / 2.0f});

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader.program_id);

    glfwTerminate();
    return 0;
}
