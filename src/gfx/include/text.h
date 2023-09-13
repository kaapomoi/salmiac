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
    Text(std::string const& content,
         Font& font,
         glm::vec2 const& pos,
         glm::vec2 const& scale,
         glm::vec4 const color = glm::vec4{1.f}) noexcept;

    std::vector<Mesh> const& char_quads() noexcept;

    glm::vec4 const& color() noexcept;

    glm::vec2 const& size() noexcept;

private:
    Font& m_font;

    std::string m_content;
    std::vector<Mesh> m_char_quads; // Not to be confused with Farquaads
    glm::vec4 m_color;
    glm::vec2 m_size;
};


} // namespace sal

#endif //SALMIAC_TEXT_H
