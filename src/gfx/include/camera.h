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

class Camera {
public:
    explicit Camera(glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                    float yaw = YAW,
                    float pitch = PITCH) noexcept;

    [[nodiscard]] glm::mat4 get_view_matrix(glm::vec3 const& world_position) const
    {
        return glm::lookAt(world_position, world_position + m_front, m_up);
    }

    void look_around(float const x_offset,
                     float const y_offset,
                     const bool constrain_pitch) noexcept;

    void zoom(float const y_offset) noexcept;

    [[nodiscard]] float zoom() const noexcept { return m_zoom; }

    [[nodiscard]] glm::vec3 const& front() const noexcept { return m_front; }
    [[nodiscard]] glm::vec3 const& right() const noexcept { return m_right; }

private:
    void update_camera_vectors() noexcept;

    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;

    // euler Angles
    float m_yaw;
    float m_pitch;

    float m_zoom;

    static constexpr float ZOOM{45.0f};
    static constexpr float YAW{0.0f};
    static constexpr float PITCH{0.0f};
};

} // namespace sal

#endif //SALMIAC_CAMERA_H
