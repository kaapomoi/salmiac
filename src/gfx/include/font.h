/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_FONT_H
#define SALMIAC_FONT_H

#include <array>
#include <cstdint>

namespace sal {


struct Font {
    static constexpr std::size_t n_characters{96};

    struct Atlas {
        std::uint32_t id{0};
        std::int32_t width{0};
        std::int32_t height{0};
    };

    struct Character_info {
        float advance_x;
        float advance_y;
        float bitmap_w;
        float bitmap_h;
        float bitmap_left;
        float bitmap_top;
        float atlas_offset_x;
    };

    Atlas atlas;
    std::array<Character_info, n_characters> character_info;
};


} // namespace sal

#endif //SALMIAC_TEXT_H
