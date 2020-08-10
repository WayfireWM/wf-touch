#pragma once

#include <glm/vec2.hpp>

namespace wf
{
namespace touch
{
using point_t = glm::dvec2;

struct finger_t
{
    point_t origin;
    point_t current;
};

/**
 * Movement direction.
 */
enum move_direction_t
{
    MOVE_DIRECTION_LEFT  = (1 << 0),
    MOVE_DIRECTION_RIGHT = (1 << 1),
    MOVE_DIRECTION_UP    = (1 << 2),
    MOVE_DIRECTION_DOWN  = (1 << 3),
};

}
}
