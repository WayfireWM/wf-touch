#include "math.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <cmath>

#include <iostream>
#define _ << " " <<
#define debug(x) #x << " = " << (x)

uint32_t get_move_direction(const finger_t& finger)
{
    auto delta = finger.delta();
    uint32_t result = 0;
    if (delta.x < 0)
    {
        result |= MOVE_DIRECTION_LEFT;
    } else if (delta.x > 0)
    {
        result |= MOVE_DIRECTION_RIGHT;
    }

    if (delta.y < 0)
    {
        result |= MOVE_DIRECTION_UP;
    } else if (delta.y > 0)
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

double get_incorrect_drag_distance(const finger_t& finger, uint32_t direction)
{
    const auto normal = get_dir_nv(direction);
    const auto delta = finger.delta();
    /* grahm-schmidt */
    const auto residual = delta - normal * (glm::dot(delta, normal) / glm::dot(normal, normal));
    return glm::length(residual);
}

double get_pinch_scale(const gesture_state_t& state)
{
    auto center = state.get_center();
    double old_dist = 0;
    double new_dist = 0;

    for (const auto& f : state.fingers)
    {
        old_dist += glm::length(f.second.origin - center.origin);
        new_dist += glm::length(f.second.current - center.current);
    }

    old_dist /= state.fingers.size();
    new_dist /= state.fingers.size();
    return new_dist / old_dist;
}

double oriented_angle(point_t a, point_t b)
{
    return std::acos(glm::dot(a, b) / glm::length(a) / glm::length(b));
}

double get_rotation_angle(const gesture_state_t& state)
{
    auto center = state.get_center();

    double angle_sum = 0;
    for (const auto& f : state.fingers)
    {
        auto v1 = glm::normalize(f.second.origin - center.origin);
        auto v2 = glm::normalize(f.second.current - center.current);
        angle_sum += glm::orientedAngle(v1, v2);
    }

    angle_sum /= state.fingers.size();
    return angle_sum;
}
