/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef OCT_H
#define OCT_H

#include "node.h"

#include <array>
#include <memory>

class Oct {
public:
    Oct(glm::vec3 const front_top_left, glm::vec3 const back_bot_right) noexcept;

    void update_force(std::shared_ptr<Node> const& node) noexcept;

    void insert(std::shared_ptr<Node> const& node) noexcept;

private:
    bool is_external() const noexcept;

    bool in_boundary(glm::vec3 const& point) const noexcept;


    /// Variables
    glm::vec3 m_front_top_left{0.0f};
    glm::vec3 m_back_bottom_right{0.0f};
    float m_width{0.f};
    float m_height{0.f};
    float m_depth{0.f};
    double m_total_mass{0.f};
    glm::vec3 m_center_of_mass{0.f};

    std::shared_ptr<Node> m_node;

    /*
    Children of this tree:
         frontTopLeftTree;
         frontBotLeftTree;

         frontTopRightTree;
         frontBotRightTree;

         backTopLeftTree;
         backBotLeftTree;

         backTopRightTree;
         backBotRightTree;
     */
    std::array<std::shared_ptr<Oct>, 8> m_children;
};

#endif
