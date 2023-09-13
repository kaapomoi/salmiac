/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "font_loader.h"

#include "log.h"

#include "GL/glew.h"

#include <algorithm>

namespace sal {

Font_loader::Font_loader() noexcept
{
    if (FT_Init_FreeType(&m_ft_instance)) {
        Log::error("Could not init FreeType Library");
        return;
    }
}

Font_loader::~Font_loader() noexcept
{
    FT_Done_FreeType(m_ft_instance);
}

Font Font_loader::create(std::string const& path) noexcept
{

    FT_Face face;
    if (FT_New_Face(m_ft_instance, path.c_str(), 0, &face)) {
        Log::error("Failed to load font from path: {}", path);
        return {};
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    FT_GlyphSlot g{face->glyph};
    std::uint32_t w{0};
    std::uint32_t h{0};

    /// Load ASCII characters only
    for (std::size_t i{32}; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            Log::warn("Loading character {} from path: {} failed", i, path);
            continue;
        }

        w += g->bitmap.width + 1;
        h = std::max(h, g->bitmap.rows);
    }

    return generate_texture_atlas(face, g, w, h);
}


Font Font_loader::generate_texture_atlas(FT_Face& face,
                                         FT_GlyphSlot& glyph,
                                         std::uint32_t const w,
                                         std::uint32_t const h) noexcept
{
    Font result;
    result.atlas.width = w;
    result.atlas.height = h;

    // Create an empty texture of size w x h:
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &result.atlas.id);
    glBindTexture(GL_TEXTURE_2D, result.atlas.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    /* We require 1 byte alignment when uploading texture data */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Clamping to edges is important to prevent artifacts when scaling */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Linear filtering usually looks best for text */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::int32_t x_offset = 0;

    for (std::size_t i{32}; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            continue;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, 0, glyph->bitmap.width, glyph->bitmap.rows,
                        GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);


        std::size_t const char_info_index{i - 32};

        result.character_info.at(char_info_index).advance_x = glyph->advance.x >> 6;
        result.character_info.at(char_info_index).advance_y = glyph->advance.y >> 6;

        result.character_info.at(char_info_index).bitmap_w = glyph->bitmap.width;
        result.character_info.at(char_info_index).bitmap_h = glyph->bitmap.rows;

        result.character_info.at(char_info_index).bitmap_left = glyph->bitmap_left;
        result.character_info.at(char_info_index).bitmap_top = glyph->bitmap_top;

        result.character_info.at(char_info_index).atlas_offset_x = static_cast<float>(x_offset) / w;

        x_offset += glyph->bitmap.width + 1;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);

    return result;
}

} // namespace sal
