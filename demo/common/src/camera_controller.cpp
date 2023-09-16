/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "camera_controller.h"


void Camera_controller::operator()(double const x_offset,
                                   double const y_offset,
                                   float const amount,
                                   sal::Window_ptr const& window,
                                   sal::Input_manager const& input_manager,
                                   sal::Camera& camera_data,
                                   sal::Transform& transform) const noexcept
{

    if (input_manager.button(GLFW_MOUSE_BUTTON_RIGHT)) {
        camera_data.look_around(static_cast<float>(x_offset), static_cast<float>(-y_offset), true);
        glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else {
        glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    float camera_movement_speed{50.f};

    if (input_manager.key(GLFW_KEY_LEFT_SHIFT)) {
        camera_movement_speed = 500.f;
    }

    if (input_manager.key(GLFW_KEY_W)) {
        transform.position += camera_data.front() * amount * camera_movement_speed;
    }
    if (input_manager.key(GLFW_KEY_S)) {
        transform.position -= camera_data.front() * amount * camera_movement_speed;
    }
    if (input_manager.key(GLFW_KEY_A)) {
        transform.position -= camera_data.right() * amount * camera_movement_speed;
    }
    if (input_manager.key(GLFW_KEY_D)) {
        transform.position += camera_data.right() * amount * camera_movement_speed;
    }
}
