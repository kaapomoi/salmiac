/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_INPUT_MANAGER_H
#define SALMIAC_INPUT_MANAGER_H

#include "camera.h"

#include <cstdint>
#include <unordered_map>

namespace sal {

struct Key_state {
    bool down_this_frame{false};
    bool down{false};
};

class Input_manager {
public:
    void register_keys(std::initializer_list<std::int32_t> keys,
                       std::initializer_list<std::int32_t> mouse_buttons) noexcept;

    void tick(Window_ptr const& window) noexcept;

    bool key(std::int32_t key) noexcept;

    bool key_now(std::int32_t key) noexcept;

    bool button(std::int32_t button) noexcept;

    bool button_now(std::int32_t button) noexcept;

private:
    enum class Input_type : std::uint8_t { key, mouse_button };

    static void tick_map(Window_ptr const& window,
                         std::unordered_map<std::int32_t, Key_state>& map,
                         Input_type const input_type) noexcept;

    std::unordered_map<std::int32_t, Key_state> m_keys;
    std::unordered_map<std::int32_t, Key_state> m_buttons;
};

} // namespace sal

#endif //SALMIAC_INPUT_MANAGER_H
