/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_INPUT_MANAGER_H
#define SALMIAC_INPUT_MANAGER_H

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace sal {

struct Key_state {
    bool down_this_frame{false};
    bool down{false};
};

struct Mouse_position {
    double x;
    double y;

    Mouse_position operator+(Mouse_position const& other) const
    {
        return {x + other.x, y + other.y};
    }
    Mouse_position operator-(Mouse_position const& other) const
    {
        return {x - other.x, y - other.y};
    }
};

class Input_manager {
public:
    void register_keys(std::initializer_list<std::int32_t> keys,
                       std::initializer_list<std::int32_t> mouse_buttons) noexcept;

    void update(std::function<bool(std::int32_t)>& key_callback,
                std::function<bool(std::int32_t)>& mouse_callback,
                std::function<Mouse_position()>& mouse_position_callback) noexcept;

    [[nodiscard]] bool key(std::int32_t const key) const noexcept;

    [[nodiscard]] bool key_now(std::int32_t const key) const noexcept;

    [[nodiscard]] bool button(std::int32_t const button) const noexcept;

    [[nodiscard]] bool button_now(std::int32_t const button) const noexcept;

    [[nodiscard]] Mouse_position mouse_pos() const noexcept;
    [[nodiscard]] Mouse_position mouse_pos_delta() const noexcept;

private:
    enum class Input_type : std::uint8_t { key, mouse_button };

    static void update_internal_state(std::function<bool(std::int32_t)>& input_callback,
                                      std::unordered_map<std::int32_t, Key_state>& map) noexcept;

    Mouse_position m_mouse_pos{0, 0};
    Mouse_position m_mouse_pos_prev_frame{0, 0};
    std::unordered_map<std::int32_t, Key_state> m_keys;
    std::unordered_map<std::int32_t, Key_state> m_buttons;
};

} // namespace sal

#endif //SALMIAC_INPUT_MANAGER_H
