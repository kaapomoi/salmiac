/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_TEXT_H
#define SALMIAC_TEXT_H

#include "font.h"
#include "mesh.h"
#include "vertex.h"

#include <memory>
#include <string>


namespace sal {


class Text {
public:
    static constexpr std::size_t max_content_length{256};

    Text(std::string const& content,
         Font& font,
         glm::vec2 const& pos,
         glm::vec2 const& scale,
         glm::vec4 const color = glm::vec4{1.f}) noexcept;

    void set_content(std::string const& content) noexcept;

    std::vector<Mesh> char_quads() noexcept;

    glm::vec4 const& color() noexcept;

    glm::vec2 const& size() noexcept;

private:
    void update_character_quads() noexcept;

    Font& m_font;

    std::string m_content;
    std::vector<Mesh> m_char_quads; // Not to be confused with Farquaads
    glm::vec4 m_color;
    glm::vec2 m_size;
    glm::vec2 m_scale;
    glm::vec2 m_pos;
};


} // namespace sal

#endif //SALMIAC_TEXT_H
