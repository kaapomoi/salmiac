/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "input_manager.h"

namespace sal {

void Input_manager::register_keys(std::initializer_list<std::int32_t> keys,
                                  std::initializer_list<std::int32_t> mouse_buttons) noexcept
{
    for (auto& key : keys) {
        m_keys.emplace(key, Key_state{false, false});
    }
    for (auto& button : mouse_buttons) {
        m_buttons.emplace(button, Key_state{false, false});
    }
}

void Input_manager::tick(Window_ptr const& window) noexcept
{
    tick_map(window, m_keys, Input_type::key);
    tick_map(window, m_buttons, Input_type::mouse_button);
}

bool Input_manager::key(std::int32_t key) noexcept
{
    return m_keys.at(key).down;
}

bool Input_manager::key_now(std::int32_t key) noexcept
{
    return m_keys.at(key).down_this_frame;
}

bool Input_manager::button(std::int32_t button) noexcept
{
    return m_buttons.at(button).down;
}

bool Input_manager::button_now(std::int32_t button) noexcept
{
    return m_buttons.at(button).down_this_frame;
}

void Input_manager::tick_map(Window_ptr const& window,
                             std::unordered_map<std::int32_t, Key_state>& map,
                             Input_type const input_type) noexcept
{
    for (auto& key_state_pair : map) {
        key_state_pair.second.down_this_frame = false;

        std::int32_t state{0};

        // Check key or button
        if (input_type == Input_type::key) {
            state = glfwGetKey(window.get(), key_state_pair.first);
        }
        else if (input_type == Input_type::mouse_button) {
            state = glfwGetMouseButton(window.get(), key_state_pair.first);
        }
        else {
            state = 0;
        }

        /// Check the state
        if (state == GLFW_PRESS) {
            /// Only turn this on if last frame the key was off
            if (!key_state_pair.second.down) {
                key_state_pair.second.down_this_frame = true;
            }
            key_state_pair.second.down = true;
        }
        else if (state == GLFW_RELEASE) {
            key_state_pair.second.down = false;
        }
    }
}

} // namespace sal