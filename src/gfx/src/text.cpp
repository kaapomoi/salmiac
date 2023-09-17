/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "text.h"

#include "log.h"
#include "mesh_binder.h"

namespace sal {

std::vector<Mesh> Text::char_quads() noexcept
{
    return std::vector<Mesh>{m_char_quads.begin(), m_char_quads.begin() + m_content.length()};
}

glm::vec4 const& Text::color() noexcept
{
    return m_color;
}

glm::vec2 const& Text::size() noexcept
{
    return m_size;
}

void Text::set_content(std::string const& content) noexcept
{
    bool const dirty{m_content != content};
    m_content = content;
    if (dirty) {
        for (std::size_t i{0}; i < m_content.size(); i++) {
            //Mesh_binder::clear_buffer_data(m_char_quads.at(i));
        }
        update_character_quads();
    }
}

Text::Text(std::string const& content,
           Font& font,
           glm::vec2 const& pos,
           glm::vec2 const& scale,
           glm::vec4 const color) noexcept
    : m_content{content}, m_font{font}, m_pos{pos}, m_scale{scale}, m_color{color}
{
    m_char_quads.resize(max_content_length);
    update_character_quads();
}

void Text::update_character_quads() noexcept
{
    float x{m_pos.x};
    float y{m_pos.y};
    float const sx{m_scale.x};
    float const sy{m_scale.y};

    float max_h{0.f};

    /// Create text
    std::size_t index{0};
    for (auto const& character : m_content) {
        std::size_t const c{static_cast<size_t>(character - 32)};
        float const x2{x + m_font.character_info.at(c).bitmap_left * sx};
        float const y2{-y - m_font.character_info.at(c).bitmap_top * sy};

        float const w{m_font.character_info.at(c).bitmap_w * sx};
        float const h{m_font.character_info.at(c).bitmap_h * sy};

        x += m_font.character_info.at(c).advance_x * sx;
        y += m_font.character_info.at(c).advance_y * sy;

        Mesh& quad{m_char_quads.at(index++)};

        // Skip glyphs without pixels
        // TODO: Check if this causes the issue with spaces.

        static glm::vec3 const normal{0.f, 0.f, -1.f};

        Vertex v0{glm::vec2{m_font.character_info.at(c).atlas_offset_x, 0}, normal,
                  glm::vec3{x2, -y2, 0}, m_color};

        Vertex v1{glm::vec2{m_font.character_info.at(c).atlas_offset_x
                                + m_font.character_info.at(c).bitmap_w / m_font.atlas.width,
                            0},
                  normal, glm::vec3{x2 + w, -y2, 0}, m_color};

        Vertex v2{glm::vec2{m_font.character_info.at(c).atlas_offset_x,
                            m_font.character_info.at(c).bitmap_h / m_font.atlas.height},
                  normal, glm::vec3{x2, -y2 - h, 0}, m_color};

        Vertex v3{glm::vec2{m_font.character_info.at(c).atlas_offset_x
                                + m_font.character_info.at(c).bitmap_w / m_font.atlas.width,
                            m_font.character_info.at(c).bitmap_h / m_font.atlas.height},
                  normal, glm::vec3{x2 + w, -y2 - h, 0}, m_color};

        quad.vertices = {v0, v1, v2, v3};
        quad.indices = {0, 2, 1, 1, 2, 3};
        quad.textures = {Texture{m_font.atlas.id, m_font.atlas.width, m_font.atlas.height}};

        if (!quad.initialized) {
            Mesh_binder::setup(quad);
        }
        else {
            Mesh_binder::set_buffer_data(quad);
        }

        max_h = std::max(max_h, h);

        /// Last character, set final size
        if (index == m_content.size() - 1) {
            m_size = {x, max_h};
        }
    }
}


} // namespace sal
