#define GLM_ENABLE_EXPERIMENTAL // for glm::orientedAngle

#include <wayfire/touch/touch.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <cmath>

#include <iostream>
#define _ << " " <<
#define debug(x) #x << " = " << (x)

static constexpr double DIRECTION_TAN_THRESHOLD = 1.0 / 3.0;

using namespace wf::touch;
uint32_t wf::touch::finger_t::get_direction() const
{
    double to_left = this->get_drag_distance(MOVE_DIRECTION_LEFT);
    double to_right = this->get_drag_distance(MOVE_DIRECTION_RIGHT);
    double to_up = this->get_drag_distance(MOVE_DIRECTION_UP);
    double to_down = this->get_drag_distance(MOVE_DIRECTION_DOWN);

    double horizontal = std::max(to_left, to_right);
    double vertical = std::max(to_up, to_down);

    uint32_t result = 0;
    if (to_left > 0 && to_left / vertical >= DIRECTION_TAN_THRESHOLD)
    {
        result |= MOVE_DIRECTION_LEFT;
    } else if (to_right > 0 && to_right / vertical >= DIRECTION_TAN_THRESHOLD)
    {
        result |= MOVE_DIRECTION_RIGHT;
    }

    if (to_up > 0 && to_up / horizontal >= DIRECTION_TAN_THRESHOLD)
    {
        result |= MOVE_DIRECTION_UP;
    } else if (to_down > 0 && to_down / horizontal >= DIRECTION_TAN_THRESHOLD)
    {
        result |= MOVE_DIRECTION_DOWN;
    }

    return result;
}

/** Get normal vector in direction */
static point_t get_dir_nv(uint32_t direction)
{
    assert((direction != 0) && ((direction & 0b1111) == direction));

    point_t dir = {0, 0};
    if (direction & MOVE_DIRECTION_LEFT)
    {
        dir.x = -1;
    }
    else if (direction & MOVE_DIRECTION_RIGHT)
    {
        dir.x = 1;
    }
    if (direction & MOVE_DIRECTION_UP)
    {
        dir.y = -1;
    }
    else if (direction & MOVE_DIRECTION_DOWN)
    {
        dir.y = 1;
    }

    return dir;
}

double wf::touch::finger_t::get_drag_distance(uint32_t direction) const
{
    const auto normal = get_dir_nv(direction);
    const auto delta = this->delta();

    /* grahm-schmidt */
    const double amount_alongside_dir = glm::dot(delta, normal) / glm::dot(normal, normal);
    if (amount_alongside_dir >= 0)
    {
        return glm::length(amount_alongside_dir * normal);
    }

    return 0;
}

double wf::touch::finger_t::get_incorrect_drag_distance(uint32_t direction) const
{
    const auto normal = get_dir_nv(direction);
    const auto delta = this->delta();

    /* grahm-schmidt */
    double amount_alongside_dir = glm::dot(delta, normal) / glm::dot(normal, normal);
    if (amount_alongside_dir < 0)
    {
        /* Drag in opposite direction */
        return glm::length(delta);
    }

    const auto residual = delta - normal * amount_alongside_dir;
    return glm::length(residual);
}

double wf::touch::gesture_state_t::get_pinch_scale() const
{
    auto center = get_center();
    double old_dist = 0;
    double new_dist = 0;

    for (const auto& f : fingers)
    {
        old_dist += glm::length(f.second.origin - center.origin);
        new_dist += glm::length(f.second.current - center.current);
    }

    old_dist /= fingers.size();
    new_dist /= fingers.size();
    return new_dist / old_dist;
}

double wf::touch::gesture_state_t::get_rotation_angle() const
{
    auto center = get_center();

    double angle_sum = 0;
    for (const auto& f : fingers)
    {
        auto v1 = glm::normalize(f.second.origin - center.origin);
        auto v2 = glm::normalize(f.second.current - center.current);
        angle_sum += glm::orientedAngle(v1, v2);
    }

    angle_sum /= fingers.size();
    return angle_sum;
}
