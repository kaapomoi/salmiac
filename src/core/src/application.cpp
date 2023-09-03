/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"

#include <GLFW/glfw3.h>

namespace sal {

std::size_t Application::m_window_width{0};
std::size_t Application::m_window_height{0};

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

    glfwSetFramebufferSizeCallback(m_window.get(), [](GLFWwindow*, int width, int height) {
        sal::Log::info("Changing framebuffer size to {} {}", width, height);
        glViewport(0, 0, width, height);
        m_window_width = width;
        m_window_height = height;
    });

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
    auto t_now = std::chrono::high_resolution_clock::now();
    m_delta_time =
        std::chrono::duration_cast<std::chrono::duration<float>>(t_now - m_t_prev_update).count();
    m_t_prev_update = t_now;

    /// Update inputs
    run_engine_tasks();

    /// User does something with the updated inputs
    run_user_tasks();

    /// Draw?
    render_models();


    m_suggest_close = glfwWindowShouldClose(m_window.get());
    m_frame_counter++;
}

void Application::render_models() noexcept
{
    auto render_view = m_registry.view<Transform, Model, Shader_program>();
    for (auto [entity, transform, model, shader] : render_view.each()) {

        glm::vec3 model_position{transform.position};
        glm::vec3 model_rotation{transform.rotation};
        glm::vec3 model_scale{transform.scale};

        shader.use();

        glm::mat4 model_mat{1.0f};

        model_mat = glm::translate(model_mat, model_position);

        model_mat = glm::rotate(model_mat, glm::radians(model_rotation.x), {1.0f, 0.0f, 0.0f});
        model_mat = glm::rotate(model_mat, glm::radians(model_rotation.y), {0.0f, 1.0f, 0.0f});
        model_mat = glm::rotate(model_mat, glm::radians(model_rotation.z), {0.0f, 0.0f, 1.0f});
        model_mat = glm::scale(model_mat, model_scale);

        set_user_uniforms(shader);

        shader.set_uniform("model", model_mat);

        for (auto const& mesh : model.meshes) {
            for (std::size_t i{0}; i < mesh.textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
                // retrieve texture number (the N in diffuse_textureN)
                const auto texture_type = mesh.textures.at(i).type;
                std::string const uniform_identifier{"material." + sal::Texture::str(texture_type)};

                // now set the sampler to the correct texture unit
                shader.set_uniform<std::int32_t>(uniform_identifier, i);
                glBindTexture(GL_TEXTURE_2D, mesh.textures.at(i).id);
            }

            // draw mesh
            glBindVertexArray(mesh.vao);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);


            glActiveTexture(GL_TEXTURE0);
        }

        auto const err = glGetError();
        assert(err == GL_NO_ERROR);


        shader.un_use();
    }

    glfwSwapBuffers(m_window.get());
    glfwPollEvents();
}

void Application::run_engine_tasks() noexcept
{
    static std::function<bool(std::int32_t)> k_cb = [this](std::int32_t const key) {
        return key_callback(key);
    };

    static std::function<bool(std::int32_t)> m_cb = [this](std::int32_t const button) {
        return mouse_click_callback(button);
    };

    static std::function<Mouse_position()> mouse_pos_cb = [this]() -> Mouse_position {
        double x;
        double y;
        glfwGetCursorPos(m_window.get(), &x, &y);
        return {x, y};
    };

    m_input_manager.update(k_cb, m_cb, mouse_pos_cb);
}

void Application::register_keys(std::initializer_list<std::int32_t> keys,
                                std::initializer_list<std::int32_t> mouse_buttons) noexcept
{
    m_input_manager.register_keys(keys, mouse_buttons);
}

Window_ptr Application::create_window(std::size_t const w, std::size_t const h) noexcept
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = sal::Window_ptr(glfwCreateWindow(static_cast<GLsizei>(w), static_cast<GLsizei>(h),
                                                   "salmiac sandbox", nullptr, nullptr));
    m_window_width = w;
    m_window_height = h;

    if (window == nullptr) {
        glfwTerminate();
        return nullptr;
    }


    return window;
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
