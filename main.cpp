/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"
#include "camera.h"
#include "file_reader.h"
#include "input_manager.h"
#include "log.h"
#include "model_loader.h"
#include "shader_loader.h"
#include "shader_program.h"

#include <chrono>
#include <map>
#include <thread>

namespace {


} // namespace


static GLsizei constexpr WINDOW_WIDTH{1280};
static GLsizei constexpr WINDOW_HEIGHT{720};


class Market : public sal::Application {
public:
    sal::Application::Exit_code start() noexcept
    {
        register_keys(
            {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_ESCAPE},
            {GLFW_MOUSE_BUTTON_RIGHT});

        return setup(WINDOW_WIDTH, WINDOW_HEIGHT);
    }


    sal::Application::Exit_code run() noexcept
    {
        auto v_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
        auto f_str = sal::File_reader::read_file("../res/shaders/basic_lighting.fsh");

        m_shaders.push_back(
            sal::Shader_loader::from_sources(v_str, f_str, {"in_uv", "in_normal", "in_pos"}));

        auto v2_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
        auto f2_str = sal::File_reader::read_file("../res/shaders/basic_lighting.fsh");
        m_shaders.push_back(
            sal::Shader_loader::from_sources(v2_str, f2_str, {"in_uv", "in_normal", "in_pos"}));

        std::uint64_t const base_flags = aiProcess_Triangulate | aiProcess_GenNormals
                                         | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes;

        std::string const model_file{"../res/models/case/j-case.obj"};
        float const scale_factor{1.0f};
        m_models.push_back(
            sal::Model_loader::from_file(model_file, scale_factor, base_flags | aiProcess_FlipUVs));

        std::string const avo_file{"../res/models/Sponza/Sponza.gltf"};
        float const scale_factor2{0.01f};
        m_models.push_back(
            sal::Model_loader::from_file(avo_file, scale_factor2, base_flags | aiProcess_FlipUVs));

        m_shader_model_pairs.push_back({m_shaders.front(), m_models.front()});
        m_shader_model_pairs.push_back({m_shaders.back(), m_models.back()});

        m_t_start = std::chrono::high_resolution_clock::now();

        while (!m_suggest_close) {
            update();
        }

        return Exit_code::ok;
    }

    void cleanup() noexcept
    {
        for (auto const& shader : m_shaders) {
            glDeleteProgram(shader.program_id);
        }
        glfwTerminate();
    }

private:
    void run_user_tasks() noexcept final
    {
        handle_input();
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glm::mat4 const projection = glm::perspective(
            glm::radians(m_camera.zoom()),
            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f, 1000.0f);

        glm::mat4 const view = m_camera.get_view_matrix();

        auto t_now = std::chrono::high_resolution_clock::now();
        float time =
            std::chrono::duration_cast<std::chrono::duration<float>>(t_now - m_t_start).count();


        for (auto& shader_model_pair : m_shader_model_pairs) {
            auto& shader = shader_model_pair.first;
            auto& model = shader_model_pair.second;

            sal::Log::info("Shader id {}", shader.program_id);
            sal::Log::info("Meshes size {}", model.meshes.size());

            glm::vec3 model_rotation{0.0f + time * 0.01f};
            glm::vec3 model_position{0.0f};

            shader.use();

            glm::mat4 model_mat{1.0f};

            model_mat = glm::scale(model_mat, glm::vec3{1.0f, 1.0f, 1.0f});


            model_mat =
                glm::rotate(model_mat, glm::radians(model_rotation.x * 360.0f), {1.0f, 0.0f, 0.0f});
            model_mat =
                glm::rotate(model_mat, glm::radians(model_rotation.y * 360.0f), {0.0f, 1.0f, 0.0f});
            model_mat =
                glm::rotate(model_mat, glm::radians(model_rotation.z * 360.0f), {0.0f, 0.0f, 1.0f});
            model_mat = glm::translate(model_mat, model_position);

            shader.set_uniform<float>("material.shininess", 64.0f);
            shader.set_uniform("camera_pos", m_camera.position());
            shader.set_uniform("projection", projection);
            shader.set_uniform("view", view);
            shader.set_uniform("model", model_mat);

            for (auto const& mesh : model.meshes) {
                for (std::size_t i{0}; i < mesh.textures.size(); i++) {
                    glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
                    // retrieve texture number (the N in diffuse_textureN)
                    const auto texture_type = mesh.textures.at(i).type;
                    std::string const uniform_identifier{"material."
                                                         + sal::Texture::str(texture_type)};

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

    void handle_input()
    {
        double x;
        double y;
        glfwGetCursorPos(m_window.get(), &x, &y);

        static constexpr float sensitivity{0.1f};

        double const x_offset{(x - m_mouse_x) * sensitivity};
        double const y_offset{(y - m_mouse_y) * sensitivity};

        m_mouse_x = x;
        m_mouse_y = y;

        if (m_input_manager.button(GLFW_MOUSE_BUTTON_RIGHT)) {
            m_camera.look_around(static_cast<float>(x_offset), static_cast<float>(-y_offset), true);
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        static constexpr float camera_movement_speed{0.01f};

        if (m_input_manager.key(GLFW_KEY_W)) {
            m_camera.move(sal::Camera::FORWARD, camera_movement_speed);
        }
        if (m_input_manager.key(GLFW_KEY_S)) {
            m_camera.move(sal::Camera::BACKWARD, camera_movement_speed);
        }
        if (m_input_manager.key(GLFW_KEY_A)) {
            m_camera.move(sal::Camera::LEFT, camera_movement_speed);
        }
        if (m_input_manager.key(GLFW_KEY_D)) {
            m_camera.move(sal::Camera::RIGHT, camera_movement_speed);
        }

        if (m_input_manager.key(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(m_window.get(), true);
        }
    }

    sal::Camera m_camera{{0.f, 0.f, 3.f}};
    double m_mouse_x{0};
    double m_mouse_y{0};
    std::chrono::high_resolution_clock::time_point m_t_start;

    std::vector<sal::Shader_program> m_shaders;
    std::vector<sal::Model> m_models;

    std::vector<std::pair<sal::Shader_program&, sal::Model&>> m_shader_model_pairs;
};


int main()
{
    Market app;
    app.start();

    app.run();

    app.cleanup();

    return 0;
}
