/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "text.h"

#include "log.h"
#include "mesh_binder.h"

namespace sal {

std::vector<Mesh> const& Text::char_quads() noexcept
{
    return m_char_quads;
}

glm::vec4 const& Text::color() noexcept
{
    return m_color;
}

Text::Text(std::string const& content,
           Font& font,
           glm::vec2 const& pos,
           glm::vec2 const& size,
           glm::vec4 const color) noexcept
    : m_font{font}, m_content{content}, m_color{color}
{
    float x{pos.x};
    float y{pos.y};
    float const sx{size.x};
    float const sy{size.y};

    /// Create text
    for (auto const& character : content) {
        std::size_t c{static_cast<size_t>(character - 32)};
        float x2{x + m_font.character_info.at(c).bitmap_left * sx};
        float y2{-y - m_font.character_info.at(c).bitmap_top * sy};

        float w{m_font.character_info.at(c).bitmap_w * sx};
        float h{m_font.character_info.at(c).bitmap_h * sy};

        x += m_font.character_info.at(c).advance_x * sx;
        y += m_font.character_info.at(c).advance_y * sy;

        // Skip glyphs without pixels
        if (!w || !h) {
            continue;
        }

        static glm::vec3 normal{0.f, 0.f, -1.f};

        Mesh quad;

        Vertex v0{glm::vec2{m_font.character_info.at(c).atlas_offset_x, 0}, normal,
                  glm::vec3{x2, -y2, 0}};

        Vertex v1{glm::vec2{m_font.character_info.at(c).atlas_offset_x
                                + m_font.character_info.at(c).bitmap_w / m_font.atlas.width,
                            0},
                  normal, glm::vec3{x2 + w, -y2, 0}};

        Vertex v2{glm::vec2{m_font.character_info.at(c).atlas_offset_x,
                            m_font.character_info.at(c).bitmap_h / m_font.atlas.height},
                  normal, glm::vec3{x2, -y2 - h, 0}};

        Vertex v3{glm::vec2{m_font.character_info.at(c).atlas_offset_x
                                + m_font.character_info.at(c).bitmap_w / m_font.atlas.width,
                            m_font.character_info.at(c).bitmap_h / m_font.atlas.height},
                  normal, glm::vec3{x2 + w, -y2 - h, 0}};
        quad.vertices = {v0, v1, v2, v3};
        quad.indices = {0, 1, 2, 1, 2, 3};
        quad.textures = {Texture{m_font.atlas.id, m_font.atlas.width, m_font.atlas.height}};

        Mesh_binder::setup(quad);

        m_char_quads.push_back(quad);
    }
}


} // namespace sal
