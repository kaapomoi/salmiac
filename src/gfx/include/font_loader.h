/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_FONT_LOADER_H
#define SALMIAC_FONT_LOADER_H

#include "font.h"

#include <ft2build.h>

#include <string>
#include FT_FREETYPE_H

namespace sal {


class Font_loader {
public:
    Font_loader() noexcept;
    ~Font_loader() noexcept;

    Font create(std::string const& path) noexcept;

private:
    Font generate_texture_atlas(FT_Face& face,
                                FT_GlyphSlot& glyph,
                                std::uint32_t const w,
                                std::uint32_t const h) noexcept;

    FT_Library m_ft_instance;
};


} // namespace sal

#endif //SALMIAC_TEXT_H
