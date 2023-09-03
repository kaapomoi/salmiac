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

/// TODO: Maybe just use glfwGetKey() directly instead of these callbacks.
void Input_manager::update(std::function<bool(std::int32_t)>& key_callback,
                           std::function<bool(std::int32_t)>& mouse_callback) noexcept
{
    update_internal_state(key_callback, m_keys);
    update_internal_state(mouse_callback, m_buttons);
}

void Input_manager::update_internal_state(std::function<bool(std::int32_t)>& input_callback,
                                          std::unordered_map<std::int32_t, Key_state>& map) noexcept
{
    for (auto& key_state_pair : map) {
        key_state_pair.second.down_this_frame = false;

        /// Check whether it was pressed or released
        if (input_callback(key_state_pair.first)) {
            /// Only turn this on if last frame the key was off
            if (!key_state_pair.second.down) {
                key_state_pair.second.down_this_frame = true;
            }
            key_state_pair.second.down = true;
        }
        else {
            // Release
            key_state_pair.second.down = false;
        }
    }
}


} // namespace sal
