/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#ifndef SALMIAC_CAMERA_H
#define SALMIAC_CAMERA_H

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"

#include <memory>

namespace sal {


// A camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:
    // constructor with vectors
    explicit Camera(glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                    float yaw = YAW,
                    float pitch = PITCH) noexcept;

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    [[nodiscard]] glm::mat4 get_view_matrix(glm::vec3 const& world_position) const
    {
        return glm::lookAt(world_position, world_position + m_front, m_up);
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void look_around(float const x_offset,
                     float const y_offset,
                     const bool constrain_pitch) noexcept;

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void zoom(float const y_offset) noexcept;

    [[nodiscard]] float zoom() const noexcept { return m_zoom; }

    [[nodiscard]] glm::vec3 const& front() const noexcept { return m_front; }
    [[nodiscard]] glm::vec3 const& right() const noexcept { return m_right; }

private:
    // calculates the m_front vector from the Camera's (updated) Euler Angles
    void update_camera_vectors() noexcept;

    // camera Attributes
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;

    // euler Angles
    float m_yaw;
    float m_pitch;

    // camera options
    float m_zoom;

    static constexpr float ZOOM{45.0f};
    static constexpr float YAW{0.0f};
    static constexpr float PITCH{0.0f};
};

} // namespace sal

#endif //SALMIAC_CAMERA_H
