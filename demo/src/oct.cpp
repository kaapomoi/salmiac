/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "oct.h"

#include "log.h"

#include <algorithm>

Oct::Oct(glm::vec3 const front_top_left, glm::vec3 const back_bot_right) noexcept
    : m_front_top_left(front_top_left), m_back_bottom_right(back_bot_right)
{
    m_width = back_bot_right.x - m_front_top_left.x;
    m_height = back_bot_right.y - m_front_top_left.y;
    m_depth = back_bot_right.z - m_front_top_left.z;
}


void Oct::update_force(std::shared_ptr<Node> const& node) noexcept
{
    if (!node) {
        return;
    }

    if ((!m_node) || (m_node == node)) {
        return;
    }

    static const double G = 6.67e-11;
    static const double eps = 100;
    static const float thresh = 0.5f;

    static const double smoothing = 0.0;


    if (is_external()) {
        glm::vec3 delta{node->position - m_node->position};
        double const dist{sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z)};
        double const F{(G * m_node->mass * node->mass) / (dist * dist + eps * eps)};
        node->force -= glm::vec3{F * delta.x / (dist + smoothing), F * delta.y / (dist + smoothing),
                                 F * delta.z / (dist + smoothing)};
    }
    else if (m_width
                 / sqrt((node->position.x - m_center_of_mass.x)
                            * (node->position.x - m_center_of_mass.x)
                        + (node->position.y - m_center_of_mass.y)
                              * (node->position.y - m_center_of_mass.y)
                        + (node->position.z - m_center_of_mass.z)
                              * (node->position.z - m_center_of_mass.z))
             < thresh) {
        glm::vec3 delta{node->position - m_center_of_mass};
        double const dist{sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z)};
        double const F{(G * m_node->mass * node->mass) / (dist * dist + eps * eps)};
        node->force -= glm::vec3{F * delta.x / dist, F * delta.y / dist, F * delta.z / dist};
    }
    else {
        for (auto& child : m_children) {
            if (child) {
                child->update_force(node);
            }
        }
    }
}

void Oct::insert(std::shared_ptr<Node> const& node) noexcept
{
    if (!node) {
        return;
    }

    if (!in_boundary(node->position)) {
        return;
    }

    double const total_mass{m_total_mass + node->mass};
    glm::vec3 const new_center_mass{
        static_cast<float>((node->position.x * node->mass + m_center_of_mass.x * m_total_mass)
                           / total_mass),
        static_cast<float>((node->position.y * node->mass + m_center_of_mass.y * m_total_mass)
                           / total_mass),
        static_cast<float>((node->position.z * node->mass + m_center_of_mass.z * m_total_mass)
                           / total_mass)};
    m_center_of_mass = new_center_mass;

    m_total_mass += node->mass;

    if (!m_node) {
        m_node = node;
        return;
    }

    /// This Oct already has a node, we need to subdivide.
    if ((m_front_top_left.z + m_back_bottom_right.z) / 2 >= node->position.z) {

        if ((m_front_top_left.x + m_back_bottom_right.x) / 2 >= node->position.x) {
            // Indicates topLeftTree
            if ((m_front_top_left.y + m_back_bottom_right.y) / 2 >= node->position.y) {
                if (!m_children.at(0)) {
                    m_children.at(0) = std::make_shared<Oct>(
                        Oct(glm::vec3(m_front_top_left.x, m_front_top_left.y, m_front_top_left.z),
                            glm::vec3((m_front_top_left.x + m_back_bottom_right.x) / 2,
                                      (m_front_top_left.y + m_back_bottom_right.y) / 2,
                                      (m_front_top_left.z + m_back_bottom_right.z) / 2)));
                }

                m_children.at(0)->insert(node);
            }
            // Indicates botLeftTree
            else {
                if (!m_children.at(1)) {
                    m_children.at(1) = std::make_shared<Oct>(
                        glm::vec3(m_front_top_left.x,
                                  (m_front_top_left.y + m_back_bottom_right.y) / 2,
                                  m_front_top_left.z),
                        glm::vec3((m_front_top_left.x + m_back_bottom_right.x) / 2,
                                  m_back_bottom_right.y,
                                  (m_front_top_left.z + m_back_bottom_right.z) / 2));
                }
                m_children.at(1)->insert(node);
            }
        }
        else {
            // Indicates topRightTree
            if ((m_front_top_left.y + m_back_bottom_right.y) / 2 >= node->position.y) {
                if (!m_children.at(2)) {
                    m_children.at(2) = std::make_shared<Oct>(
                        glm::vec3((m_front_top_left.x + m_back_bottom_right.x) / 2,
                                  m_front_top_left.y, m_front_top_left.z),
                        glm::vec3(m_back_bottom_right.x,
                                  (m_front_top_left.y + m_back_bottom_right.y) / 2,
                                  (m_front_top_left.z + m_back_bottom_right.z) / 2));
                }
                m_children.at(2)->insert(node);
            }

            // Indicates botRightTree
            else {
                if (!m_children.at(3)) {
                    m_children.at(3) = std::make_shared<Oct>(
                        glm::vec3((m_front_top_left.x + m_back_bottom_right.x) / 2,
                                  (m_front_top_left.y + m_back_bottom_right.y) / 2,
                                  m_front_top_left.z),
                        glm::vec3(m_back_bottom_right.x, m_back_bottom_right.y,
                                  (m_front_top_left.z + m_back_bottom_right.z) / 2));
                }
                m_children.at(3)->insert(node);
            }
        }
    }
    else {
        if ((m_front_top_left.x + m_back_bottom_right.x) / 2 >= node->position.x) {
            // Indicates topLeftTree
            if ((m_front_top_left.y + m_back_bottom_right.y) / 2 >= node->position.y) {
                if (!m_children.at(4)) {
                    m_children.at(4) = std::make_shared<Oct>(
                        glm::vec3(m_front_top_left.x, m_front_top_left.y,
                                  (m_front_top_left.z + m_back_bottom_right.z) / 2),
                        glm::vec3((m_front_top_left.x + m_back_bottom_right.x) / 2,
                                  (m_front_top_left.y + m_back_bottom_right.y) / 2,
                                  m_back_bottom_right.z));
                }

                m_children.at(4)->insert(node);
            }

            // Indicates botLeftTree
            else {
                if (!m_children.at(5)) {
                    m_children.at(5) = std::make_shared<Oct>(
                        glm::vec3(m_front_top_left.x,
                                  (m_front_top_left.y + m_back_bottom_right.y) / 2,
                                  (m_front_top_left.z + m_back_bottom_right.z) / 2),
                        glm::vec3((m_front_top_left.x + m_back_bottom_right.x) / 2,
                                  m_back_bottom_right.y, m_back_bottom_right.z));
                }
                m_children.at(5)->insert(node);
            }
        }
        else {
            // Indicates topRightTree
            if ((m_front_top_left.y + m_back_bottom_right.y) / 2 >= node->position.y) {
                if (!m_children.at(6)) {
                    m_children.at(6) = std::make_shared<Oct>(
                        glm::vec3((m_front_top_left.x + m_back_bottom_right.x) / 2,
                                  m_front_top_left.y,
                                  (m_front_top_left.z + m_back_bottom_right.z) / 2),
                        glm::vec3(m_back_bottom_right.x,
                                  (m_front_top_left.y + m_back_bottom_right.y) / 2,
                                  m_back_bottom_right.z));
                }
                m_children.at(6)->insert(node);
            }

            // Indicates botRightTree
            else {
                if (!m_children.at(7)) {
                    m_children.at(7) = std::make_shared<Oct>(
                        glm::vec3((m_front_top_left.x + m_back_bottom_right.x) / 2,
                                  (m_front_top_left.y + m_back_bottom_right.y) / 2,
                                  (m_front_top_left.z + m_back_bottom_right.z) / 2),
                        glm::vec3(m_back_bottom_right.x, m_back_bottom_right.y,
                                  m_back_bottom_right.z));
                }
                m_children.at(7)->insert(node);
            }
        }
    }
}


bool Oct::is_external() const noexcept
{
    return std::none_of(m_children.begin(), m_children.end(),
                        [](auto const& child) -> bool { return child != nullptr; });
}

bool Oct::in_boundary(glm::vec3 const& point) const noexcept
{
    return (point.x >= m_front_top_left.x) && (point.x <= m_back_bottom_right.x)
           && (point.x >= m_front_top_left.y) && (point.x <= m_back_bottom_right.y)
           && (point.x >= m_front_top_left.z) && (point.x <= m_back_bottom_right.z);
}
