/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "camera.h"
#include "file_reader.h"
#include "input_manager.h"
#include "log.h"
#include "model_loader.h"
#include "shader_loader.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <thread>

namespace {

sal::Window_ptr create_window(GLsizei const w, GLsizei const h) noexcept
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

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    sal::Log::info("Changing framebuffer size to {} {}", width, height);
    glViewport(0, 0, width, height);
}

void handle_input(sal::Input_manager& input_manager,
                  sal::Window_ptr const& window,
                  sal::Camera& camera,
                  double& last_x,
                  double& last_y)
{
    double x{};
    double y{};
    glfwGetCursorPos(window.get(), &x, &y);

    static constexpr float sensitivity{0.001f};

    double const x_offset{(x - last_x) * sensitivity};
    double const y_offset{(y - last_y) * sensitivity};

    last_x = x;
    last_y = y;

    if (input_manager.button(GLFW_MOUSE_BUTTON_RIGHT)) {
        camera.look_around(static_cast<float>(x_offset), static_cast<float>(-y_offset), true);
        glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else {
        glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    static constexpr float camera_movement_speed{0.001f};

    if (input_manager.key(GLFW_KEY_W)) {
        camera.move(sal::Camera::FORWARD, camera_movement_speed);
    }
    if (input_manager.key(GLFW_KEY_S)) {
        camera.move(sal::Camera::BACKWARD, camera_movement_speed);
    }
    if (input_manager.key(GLFW_KEY_A)) {
        camera.move(sal::Camera::LEFT, camera_movement_speed);
    }
    if (input_manager.key(GLFW_KEY_D)) {
        camera.move(sal::Camera::RIGHT, camera_movement_speed);
    }

    if (input_manager.key(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window.get(), true);
    }
}

bool init_glew(sal::Window_ptr const& window)
{
    glewExperimental = true; // Needed for core profile
    glfwMakeContextCurrent(window.get());
    if (glewInit() != GLEW_OK) {
        sal::Log::fatal("Failed to initialize GLEW, Reason: %s", glGetError());
        return false;
    }

    fprintf(stdout, "OpenGL version: %s\n", glGetString(GL_VERSION));
    sal::Log::info("GLEW initialized!");
    return true;
}

} // namespace

int main()
{
    static GLsizei constexpr WINDOW_WIDTH{1280};
    static GLsizei constexpr WINDOW_HEIGHT{720};

    sal::Camera camera{{0.f, 0.f, 3.f}};
    sal::Input_manager input_manager;
    input_manager.register_keys(
        {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_ESCAPE},
        {GLFW_MOUSE_BUTTON_RIGHT});

    double mouse_x{0};
    double mouse_y{0};

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

    auto v_str = sal::File_reader::read_file("../res/shaders/basic_lighting.vsh");
    auto f_str = sal::File_reader::read_file("../res/shaders/basic_lighting.fsh");

    auto shader = sal::Shader_loader::from_sources(v_str, f_str, {"in_uv", "in_normal", "in_pos"});

    std::uint64_t const base_flags =
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_OptimizeGraph | aiProcess_FlipUVs;
    std::string const model_file{"../res/models/avocado/Avocado.gltf"};

    auto avocado = sal::Model_loader::from_file(model_file, base_flags);

    sal::Log::info("{}", shader.program_id);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto t_start = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window.get())) {
        input_manager.tick(window);

        handle_input(input_manager, window, camera, mouse_x, mouse_y);

        glClearColor(0.4f, 0.3f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto t_now = std::chrono::high_resolution_clock::now();
        float time =
            std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        glm::mat4 const projection = glm::perspective(
            glm::radians(1.f), static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
            0.1f, 1000.0f);

        glm::mat4 const view = camera.get_view_matrix();

        shader.use();

        shader.set_uniform("camera_pos", camera.position());

        shader.set_uniform("projection", projection);
        shader.set_uniform("view", view);

        shader.set_uniform<float>("material.shininess", 64.0f);

        glm::mat4 model = glm::mat4{1.0f};

        model = glm::scale(model, glm::vec3{1.0f, 1.0f, 1.0f});

        glm::vec3 model_rotation{0.0f};
        glm::vec3 model_position{0.0f};

        model = glm::rotate(model, glm::radians(model_rotation.x * 360.0f), {1.0f, 0.0f, 0.0f});
        model = glm::rotate(model, glm::radians(model_rotation.y * 360.0f), {0.0f, 1.0f, 0.0f});
        model = glm::rotate(model, glm::radians(model_rotation.z * 360.0f), {0.0f, 0.0f, 1.0f});
        model = glm::translate(model, model_position);

        shader.set_uniform("model", model);

        auto const err = glGetError();
        assert(err == GL_NO_ERROR);

        for (auto const& mesh : avocado.meshes) {
            for (std::int32_t i{0}; i < mesh.textures.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
                // retrieve texture number (the N in diffuse_textureN)
                const auto texture_type = mesh.textures.at(i).type;
                //if (texture_type == "diffuse") {
                //    number = std::to_string(diffuse_nr++);
                //} else if (texture_type == "specular") {
                //    number = std::to_string(specular_nr++);
                //}
                std::string const uniform_identifier{"material." + sal::Texture::str(texture_type)};

                // now set the sampler to the correct texture unit
                //shader.set_int(texture_type + number, i);
                shader.set_uniform<std::int32_t>(uniform_identifier, i);
                glBindTexture(GL_TEXTURE_2D, mesh.textures.at(i).id);
            }

            // draw mesh
            glBindVertexArray(mesh.vao);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);


            glActiveTexture(GL_TEXTURE0);
        }

        shader.un_use();

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glDeleteProgram(shader.program_id);

    glfwTerminate();
    return 0;
}
