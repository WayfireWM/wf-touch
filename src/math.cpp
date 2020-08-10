#include "math.hpp"
#include <glm/glm.hpp>

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
