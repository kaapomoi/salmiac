/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef DEMO_CAMERA_CONTROLLER_H
#define DEMO_CAMERA_CONTROLLER_H

#include "application.h"

struct Camera_controller {
    void operator()(double const x_offset,
                    double const y_offset,
                    float const amount,
                    sal::Window_ptr const& window,
                    sal::Input_manager const& input_manager,
                    sal::Camera& camera_data,
                    sal::Transform& transform) const noexcept;
};

#endif //SALMIAC_CAMERA_CONTROLLER_H
