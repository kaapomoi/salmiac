/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "camera.h"

namespace sal {

Camera::Camera(glm::vec3 up, float yaw, float pitch) noexcept
    : m_front{glm::vec3{1.0f, 0.0f, 0.0f}}
    , m_zoom{ZOOM}
    , m_up{up}
    , m_world_up{up}
    , m_yaw{yaw}
    , m_pitch{pitch}
    , m_right{}
{
    update_camera_vectors();
}


void Camera::look_around(float const x_offset,
                         float const y_offset,
                         bool const constrain_pitch) noexcept
{
    static constexpr float min_pitch{-89.0f};
    static constexpr float max_pitch{89.0f};

    m_yaw += x_offset;
    m_pitch += y_offset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrain_pitch) {
        m_pitch = std::max(m_pitch, min_pitch);
        m_pitch = std::min(m_pitch, max_pitch);
    }

    // Update m_front, m_right and m_up Vectors using the updated Euler angles
    update_camera_vectors();
}

void Camera::zoom(float const y_offset) noexcept
{
    static constexpr float min_zoom{1.0f};
    static constexpr float max_zoom{100.0f};
    m_zoom -= y_offset;
    m_zoom = std::max(m_zoom, min_zoom);
    m_zoom = std::min(m_zoom, max_zoom);
}

void Camera::update_camera_vectors() noexcept
{
    glm::vec3 front{};
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    // Also re-calculate the right and up vector:
    // Normalize the vectors, because their length gets closer to 0
    m_right = glm::normalize(glm::cross(front, m_world_up));
    // The more you look up or down which results in slower movement.
    m_up = glm::normalize(glm::cross(m_right, front));
}

} // namespace sal
